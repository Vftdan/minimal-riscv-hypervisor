$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CFLAGS) -o $@

# Header file recursive dependency list
hypervisor_h = $(SRC_DIR)/hypervisor.h

$(BUILD_DIR)/hypervisor.o: $(SRC_DIR)/hypervisor.c $(hypervisor_h)
	mkdir -p $(shell dirname "$@")
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH) -o $@
