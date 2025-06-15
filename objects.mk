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

$(BUILD_DIR)/hypervisor.o: $(SRC_DIR)/hypervisor.c $(hypervisor_h) $(csr_h) $(exchandlers_h) $(print_h) $(panic_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/exchandlers.o: $(SRC_DIR)/exchandlers.c $(exchandlers_h) $(hypervisor_h) $(csr_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/vmem.o: $(SRC_DIR)/vmem.c $(vmem_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/pagealloc.o: $(SRC_DIR)/pagealloc.c $(pagealloc_h) $(sync_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/print.o: $(SRC_DIR)/print.c $(print_h) $(uart_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/panic.o: $(SRC_DIR)/panic.c $(panic_h) $(hypervisor_h) $(print_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@
