#! /usr/bin/env -S gdb-multiarch -x
set architecture riscv:rv64
file build/hypervisor
b halt_thread
target remote localhost:1234
c
