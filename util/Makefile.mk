$(HEXDUMP_EXE): $(ROOT)/util/hexdump.c
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) -o $@ $^
