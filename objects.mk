$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CFLAGS) -o $@

# Header file recursive dependency list
hypervisor_h = $(SRC_DIR)/hypervisor.h
csr_h = $(SRC_DIR)/csr.h $(SRC_DIR)/csrs.cc
exchandlers_h = $(SRC_DIR)/exchandlers.h
base_types_h = $(SRC_DIR)/base_types.h $(SRC_DIR)/registers.cc $(csr_h)
vmem_h = $(SRC_DIR)/vmem.h $(base_types_h)
pagealloc_h = $(SRC_DIR)/pagealloc.h $(vmem_h)
sync_h = $(SRC_DIR)/sync.h
uart_h = $(SRC_DIR)/uart.h
print_h = $(SRC_DIR)/print.h
panic_h = $(SRC_DIR)/panic.h
contexts_h = $(SRC_DIR)/contexts.h $(base_types_h) $(vmem_h)
instructions_h = $(SRC_DIR)/instructions.h
virtcsr_h = $(SRC_DIR)/virtcsr.h $(csr_h)
virtmmdev_h = $(SRC_DIR)/virtmmdev.h $(vmem_h)
guestprivilege_h = $(SRC_DIR)/guestprivilege.h
stacktrace_h = $(SRC_DIR)/stacktrace.h

$(BUILD_DIR)/hypervisor.o: $(SRC_DIR)/hypervisor.c $(hypervisor_h) $(csr_h) $(exchandlers_h) $(print_h) $(panic_h) $(contexts_h)

$(BUILD_DIR)/exchandlers.o: $(SRC_DIR)/exchandlers.c $(exchandlers_h) $(hypervisor_h) $(csr_h) $(print_h) $(panic_h) $(vmem_h) $(contexts_h) $(instructions_h) $(virtcsr_h)

$(BUILD_DIR)/vmem.o: $(SRC_DIR)/vmem.c $(vmem_h) $(print_h) $(panic_h) $(pagealloc_h) $(contexts_h) $(instructions_h) $(virtmmdev_h)

$(BUILD_DIR)/pagealloc.o: $(SRC_DIR)/pagealloc.c $(pagealloc_h) $(sync_h)

$(BUILD_DIR)/print.o: $(SRC_DIR)/print.c $(print_h) $(uart_h)

$(BUILD_DIR)/panic.o: $(SRC_DIR)/panic.c $(panic_h) $(hypervisor_h) $(print_h) $(contexts_h) $(stacktrace_h) $(SRC_DIR)/registers.cc

$(BUILD_DIR)/contexts.o: $(SRC_DIR)/contexts.c $(contexts_h)

$(BUILD_DIR)/virtcsr.o: $(SRC_DIR)/virtcsr.c $(virtcsr_h) $(print_h) $(panic_h) $(contexts_h) $(instructions_h) $(SRC_DIR)/csrs.cc

$(BUILD_DIR)/virtmmdev.o: $(SRC_DIR)/virtmmdev.c $(virtmmdev_h) $(print_h) $(uart_h)

$(BUILD_DIR)/guestprivilege.o: $(SRC_DIR)/guestprivilege.c $(guestprivilege_h) $(contexts_h) $(print_h) $(panic_h)

$(BUILD_DIR)/stacktrace.o: $(SRC_DIR)/stacktrace.c $(stacktrace_h) $(contexts_h)
