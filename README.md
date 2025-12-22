# Simple RISC-V hypervisor

This repository contains a simple RISC-V machine-mode hypervisor. It expects the guest kernel and other RAM data (e. g. RAM disk) to be mapped with an offset of 0x5000000 bytes (i. e. starting from the address 0x85000000), hypervisor code and data is located below this address. This offset can be changed in `src/vmem.h`.

## Features & limitations

 - Only uses machine-mode and user-mode privilege levels of the underlying host machine
 - Single guest, single vcpu (guest-specific and vcpu-specific data is stored separately, but guest switching is not yet supported)
 - Basic emulation of some memory-mapped devices (clint, uart, plic); use the `xv6-support` branch for better interrupt support
 - Allows guests to use machine, user, and (in the `xv6-support` branch) supervisor modes
 - Lazy-initialized shadow page table
 - Simple memory page allocator (currently only used to store shadow page tables)
 - Currently only tested inside qemu, might work incorrectly on real hardware (e. g. uart device is not being configured)

## Branches
 - `main` basic support for machine-mode-only kernels
 - `xv6-support` support for supervisor mode and better interrupt handling
 - `xv6-demo` pre-configured setup with a compatible version of the XV6 kernel

## Building & running

Building requires GNU make, gcc-riscv64-unknown-elf.
Running requires `qemu-system-riscv64`.

Run `make` to compile the hypervisor.
To run the hypervisor inside qemu, configure guest kernel loading inside `local.mk` or use the configuration from the `xv6-demo` branch, and run `make run`.

## Files

 - `src/exchandlers.c`: exception handling based on the `mcause` value
 - `src/extinterrupts.c`: external interrupt handling and their forwarding to the guest
 - `src/guestprivilege.c`: guest exception and privilege level switching emulation
 - `src/hypenterleave.S`: hypervisor call stack initialization and register saving/restoration
 - `src/hypervisor.c`: hypervisor initialization; common exception handling logic
 - `src/lto_required.c`: standard library functions that are assumed to be available by the C compiler and linker
 - `src/pagealloc.c`: memory page allocator
 - `src/panic.c`: error-caused hypervisor termination
 - `src/print.c`: hypervisor user message output
 - `src/stacktrace.c`: additional information used for hypervisor debugging
 - `src/timer.c`: core-local timer setup and timer interrupt emulation
 - `src/virtcsr.c`: emulation of guest CSRs
 - `src/virtmmdev.c`: emulation of memory-mapped guest device registers
 - `src/vmem.c`: page fault handling & page table modification
