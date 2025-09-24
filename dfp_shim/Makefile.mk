KEFIR_DFP_SHIM_BIN_DIR := $(KEFIR_BIN_DIR)/dfp_shim
KEFIR_DFP_SHIM_SOURCE_DIR := $(ROOT)/dfp_shim/source
KEFIR_DFP_SHIM_HEADERS_DIR := $(ROOT)/dfp_shim/headers
KEFIR_DFP_SHIM_SOURCE := $(wildcard $(KEFIR_DFP_SHIM_SOURCE_DIR)/*.c)
KEFIR_DFP_SHIM_OBJECT_FILES := $(KEFIR_DFP_SHIM_SOURCE:$(KEFIR_DFP_SHIM_SOURCE_DIR)/%.c=$(KEFIR_DFP_SHIM_BIN_DIR)/%.o)
KEFIR_DFP_SHIM_STATIC_LIB := $(KEFIR_BIN_DIR)/libkefir_dfp_shim.a

$(KEFIR_DFP_SHIM_BIN_DIR)/%.o: $(KEFIR_DFP_SHIM_SOURCE_DIR)/%.c
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) -I$(HEADERS_DIR) -I$(KEFIR_DFP_SHIM_HEADERS_DIR) -c $< -o $@
	@echo "$(shell pwd)" > "$@.cmd"
	@echo "$<" >> "$@.cmd"
	@echo "$@" >> "$@.cmd"
	@echo "$(CC) $(CFLAGS) -I$(HEADERS_DIR) -I$(KEFIR_DFP_SHIM_HEADERS_DIR) -c $< -o $@" >> "$@.cmd"

$(KEFIR_BIN_DIR)/libkefir_dfp_shim.reloc.o: $(KEFIR_DFP_SHIM_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Partial linking $@"
	@$(CC) -r $^ -lgcc -o $@

$(KEFIR_DFP_SHIM_STATIC_LIB): $(KEFIR_BIN_DIR)/libkefir_dfp_shim.reloc.o
	@mkdir -p $(shell dirname "$@")
	@echo "Archiving $@"
	@$(AR) cr $@ $^

OBJECT_FILES += $(KEFIR_DFP_SHIM_OBJECT_FILES)
