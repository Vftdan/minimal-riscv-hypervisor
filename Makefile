CC=riscv64-unknown-elf-gcc
CFLAGS=-g -Wall -Wextra -nodefaultlibs -nostdlib -fno-lto -fno-builtin -ffreestanding -mno-relax -mcmodel=medany -I.
LDFLAGS=-g -ffreestanding -fno-common -nostdlib -mno-relax -mcmodel=medany
OBJCOPY=riscv64-unknown-elf-objcopy

SRC_DIR=src
BUILD_DIR=build

QEMU=qemu-system-riscv64
QEMUFLAGS=-nographic -machine virt -smp 1 -bios none -kernel

HYPMODS=hypenterleave lto_required hypervisor exchandlers vmem pagealloc print

HYPOBJS:=$(foreach mod,$(HYPMODS),$(BUILD_DIR)/$(mod).o)

all: $(BUILD_DIR)/hypervisor.bin

$(BUILD_DIR)/hypervisor: $(SRC_DIR)/hypervisor.ld $(HYPOBJS)
	$(CC) $(LDFLAGS) -Wl,-T $^ -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%
	$(OBJCOPY) -O binary $< $@

include objects.mk

run: $(BUILD_DIR)/hypervisor.bin
	$(QEMU) $(QEMUFLAGS) ./$<

debug: $(BUILD_DIR)/hypervisor.bin $(BUILD_DIR)/hypervisor
	$(QEMU) $(QEMUFLAGS) ./$< -S -s

clean:
	-@rm $(HYPOBJS) $(BUILD_DIR)/hypervisor $(BUILD_DIR)/*.bin

.PHONY: all run debug
