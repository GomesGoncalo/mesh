mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

export TOP = ${mkfile_dir}

KERNEL_VERSION = $(shell cat ${TOP}/build/linux-x86-basic/.config | grep "Kernel Configuration" | awk '{print $$3}')

.PHONY: all_conf configure linux busybox busybox_package filesystem package mesh all menuconfig-linux menuconfig-busybox

all: linux busybox mesh package 

all_conf: configure all

configure:
	@mkdir -p ${TOP}/build/busybox-x86
	@$(MAKE) -C ${TOP}/busybox O=${TOP}/build/busybox-x86 defconfig
	@$(MAKE) -C ${TOP}/linux O=${TOP}/build/linux-x86-basic x86_64_defconfig
	@$(MAKE) -C ${TOP}/linux O=${TOP}/build/linux-x86-basic kvmconfig

	sed -i '/CONFIG_STATIC/d' ${TOP}/build/busybox-x86/.config
	echo "CONFIG_STATIC=y" >> ${TOP}/build/busybox-x86/.config

linux:
	@$(MAKE) -C ${TOP}/linux O=${TOP}/build/linux-x86-basic

menuconfig-linux:
	@$(MAKE) -C ${TOP}/linux O=${TOP}/build/linux-x86-basic menuconfig

menuconfig-busybox:
	@$(MAKE) -C ${TOP}/busybox O=${TOP}/build/busybox-x86 menuconfig 

mesh:
	@mkdir -pv ${TOP}/build/initramfs/busybox-x86/lib/modules
	@$(MAKE) -C ${TOP}/build/linux-x86-basic M=${TOP}/mesh INSTALL_MOD_STRIP=1 
	@$(MAKE) -C ${TOP}/build/linux-x86-basic M=${TOP}/mesh INSTALL_MOD_PATH=${TOP}/build/initramfs/busybox-x86/ INSTALL_MOD_STRIP=1 modules_install

	ls -Llh ${TOP}/build/initramfs/busybox-x86/lib/modules/${KERNEL_VERSION}/extra/mesh.ko

busybox:
	@$(MAKE) -C ${TOP}/build/busybox-x86
	@$(MAKE) -C ${TOP}/build/busybox-x86 install

busybox_package:
	@mkdir -p ${TOP}/build/initramfs/busybox-x86
	@mkdir -p ${TOP}/build/initramfs/busybox-x86/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}
	@cp -a ${TOP}/build/busybox-x86/_install/* ${TOP}/build/initramfs/busybox-x86/

filesystem:
	@cp -av ${TOP}/filesystem/* ${TOP}/build/initramfs/busybox-x86/
	@chmod +x ${TOP}/build/initramfs/busybox-x86/init

package: busybox_package filesystem
	@cd ${TOP}/build/initramfs/busybox-x86/ && find . -print0 | cpio --null -ov --format=newc | bzip2 -9 > ${TOP}/build/initramfs-busybox-x86.img
	
	@ls -Llh ${TOP}/build/initramfs-busybox-x86.img
	@ls -Llh ${TOP}/build/linux-x86-basic/arch/x86_64/boot/bzImage

clean:
	@$(MAKE) -C ${TOP}/build/linux-x86-basic mrproper
