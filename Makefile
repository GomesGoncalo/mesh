mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

export TOP = ${mkfile_dir}

ARCH ?= x86_64
IMG_TYPE = bzImage
BUILD_DIR = ${TOP}build
KERNEL_DIR = ${TOP}linux
BUSYBOX_DIR = ${TOP}busybox
MESH_DIR = ${TOP}mesh
FILESYSTEM_DIR = ${TOP}filesystem

KERNEL_VERSION = $(shell cat ${BUILD_DIR}/linux/include/config/kernel.release)

.PHONY: configure linux busybox busybox_package filesystem package mesh all menuconfig-linux menuconfig-busybox clean clean_config

all: linux busybox mesh package 

configure:
	@mkdir -p ${BUILD_DIR}/busybox
	@$(MAKE) -C ${BUSYBOX_DIR} O=${BUILD_DIR}/busybox defconfig
	@$(MAKE) -C ${KERNEL_DIR} O=${BUILD_DIR}/linux ${ARCH}_defconfig
	@$(MAKE) -C ${KERNEL_DIR} O=${BUILD_DIR}/linux kvmconfig

	sed -i '/CONFIG_STATIC/d' ${BUILD_DIR}/busybox/.config
	echo "CONFIG_STATIC=y" >> ${BUILD_DIR}/busybox/.config

linux:
	@$(MAKE) -C ${KERNEL_DIR} O=${BUILD_DIR}/linux

menuconfig-linux:
	@$(MAKE) -C ${KERNEL_DIR} O=${TOP}/build/linux menuconfig

menuconfig-busybox:
	@$(MAKE) -C ${BUSYBOX_DIR} O=${TOP}/build/busybox menuconfig 

mesh:
	@mkdir -pv ${BUILD_DIR}/initramfs/busybox/lib/modules
	@$(MAKE) -C ${BUILD_DIR}/linux M=${MESH_DIR} INSTALL_MOD_STRIP=1 
	@$(MAKE) -C ${BUILD_DIR}/linux M=${MESH_DIR} INSTALL_MOD_PATH=${BUILD_DIR}/initramfs/busybox/ INSTALL_MOD_STRIP=1 modules_install

	@ls -Llh ${BUILD_DIR}/initramfs/busybox/lib/modules/${KERNEL_VERSION}/extra/mesh.ko

busybox:
	@$(MAKE) -C ${BUILD_DIR}/busybox
	@$(MAKE) -C ${BUILD_DIR}/busybox install

busybox_package:
	@mkdir -p ${BUILD_DIR}/initramfs/busybox
	@mkdir -p ${BUILD_DIR}/initramfs/busybox/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}
	@cp -a ${BUILD_DIR}/busybox/_install/* ${BUILD_DIR}/initramfs/busybox/

filesystem:
	@cp -av ${FILESYSTEM_DIR}/* ${BUILD_DIR}/initramfs/busybox/
	@chmod +x ${BUILD_DIR}/initramfs/busybox/init

package: busybox_package filesystem
	@cd ${BUILD_DIR}/initramfs/busybox/ && find . -print0 | cpio --null -ov --format=newc | bzip2 -9 > ${BUILD_DIR}/initramfs-busybox.img
	
	@ls -Llh ${BUILD_DIR}/initramfs-busybox.img
	@ls -Llh ${BUILD_DIR}/linux/arch/${ARCH}/boot/${IMG_TYPE}

clean:
	@$(MAKE) -C ${KERNEL_DIR} O=${BUILD_DIR}/linux clean
	@$(MAKE) -C ${BUILD_DIR}/busybox clean
	@$(MAKE) -C ${BUILD_DIR}/linux M=${MESH_DIR} INSTALL_MOD_STRIP=1 clean
	@rm ${BUILD_DIR}/initramfs-busybox.img || true

clean_config: clean
	@$(MAKE) -C ${KERNEL_DIR} O=${BUILD_DIR}/linux mrproper
	@$(MAKE) -C ${BUILD_DIR}/busybox mrproper

run:
	@qemu-system-x86_64 -kernel ${BUILD_DIR}/linux/arch/${ARCH}/boot/${IMG_TYPE} -append "console=ttyS0" -initrd ${BUILD_DIR}/initramfs-busybox.img --enable-kvm --nographic