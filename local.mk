# Please, define a QEMULOAD variable with loader devices providing the kernel with dependencies (e. g. userspace programs)
# Add GUEST_MEMORY_OFFSET (defined in src/vmem.h) to the addresses
# Example:
# QEMULOAD=-device loader,file=example/kernel.bin,addr=0x85000000

QEMULOAD=\
	 -device loader,file=xv6/kernel/kernel.bin,addr=0x85000000 \
	 -device loader,file=xv6/fs.img,addr=0xc5000000
run: xv6/kernel/kernel.bin xv6/fs.img
xv6/kernel/kernel.bin: xv6/kernel/kernel
	$(OBJCOPY) -O binary $< $@
xv6/kernel/kernel: xv6/Makefile
	cd xv6 && $(MAKE) kernel/kernel
xv6/fs.img: xv6/Makefile
	cd xv6 && $(MAKE) fs.img
xv6/Makefile:
	git submodule update --init --recursive
