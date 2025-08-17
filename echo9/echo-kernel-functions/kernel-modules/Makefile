# Echo.Kern DTESN Kernel Modules Makefile
# Part of Echo9 development area

# Kernel module object files
obj-m += dtesn_core.o
obj-m += dtesn_membrane.o
obj-m += dtesn_esn.o
obj-m += dtesn_bseries.o
obj-m += dtesn_scheduler.o
obj-m += dtesn_memory.o

# Source file mappings
dtesn_core-objs := psystem.o profiler.o
dtesn_membrane-objs := psystem.o psystem_syscalls.o  
dtesn_esn-objs := esn.o esn_hardware.o esn_sparse.o
dtesn_bseries-objs := bseries.o bseries_vector.o
dtesn_scheduler-objs := scheduler.o sched_policy.o
dtesn_memory-objs := memory.o

# Build targets
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

# Testing targets
test: modules
	@echo "Testing DTESN kernel modules..."
	@echo "Note: Requires root privileges for module loading"

# Integration with main project
.PHONY: all modules modules_install clean test