obj-m := mesh.o

mesh-y := main.o

mesh-y += utils/proc.o

mesh-y += interface/hard.o
mesh-y += interface/soft.o
mesh-y += interface/netlink.o

ccflags-y := -fdiagnostics-show-location=once -fdiagnostics-generate-patch 
ccflags-y += -Werror -Wfatal-errors
ccflags-y += -g0 -feliminate-unused-debug-symbols -O3
ccflags-y += -fstack-protector-all -fstack-protector-strong
ccflags-y += -MP -I$(src)
ccflags-y += -fstack-usage

ldflags-y := 
