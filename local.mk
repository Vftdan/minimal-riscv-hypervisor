# Please, define a QEMULOAD variable with loader devices providing the kernel with dependencies (e. g. userspace programs)
# Add GUEST_MEMORY_OFFSET (defined in src/vmem.h) to the addresses
# Example:
# QEMULOAD=-device loader,file=example/kernel.bin,addr=0x85000000
