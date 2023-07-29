KEFIR_EMCC_BIN_DIR=$(BIN_DIR)/web

KEFIR_EMCC_SOURCE = $(KEFIR_LIB_SOURCE)
KEFIR_EMCC_SOURCE += $(wildcard $(SOURCE_DIR)/web/*.c)
KEFIR_EMCC_SOURCE += $(filter-out $(SOURCE_DIR)/driver/main.c,$(wildcard $(SOURCE_DIR)/driver/*.c))

KEFIR_EMCC_COMPILE_DEPS := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(KEFIR_EMCC_BIN_DIR)/%.deps)
KEFIR_EMCC_DEPENDENCY_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_EMCC_OBJECT_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(KEFIR_EMCC_BIN_DIR)/%.o)

$(KEFIR_EMCC_BIN_DIR)/%.deps:
	@mkdir -p $(shell dirname "$@")
	@touch $@

BIN_HEADERS_SRCDIR=$(SOURCE_DIR)
BIN_HEADERS_DESTDIR=$(KEFIR_EMCC_BIN_DIR)
include source/binary_headers.mk

$(KEFIR_EMCC_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d $(KEFIR_EMCC_BIN_DIR)/%.deps
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(EMCC) $(CFLAGS) $(INCLUDES) $$(cat $(subst .o,.deps,$@)) -c $< -o $@

$(KEFIR_JS): $(KEFIR_EMCC_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(EMCC) $(CFLAGS) $^ -o $@ \
		-sMODULARIZE -s'EXPORT_NAME="createKefirModule"' \
		-sEXPORTED_FUNCTIONS=UTF8ToString,stringToUTF8Array,lengthBytesUTF8,_free,_malloc,_kefir_run_with_args \
		-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,setValue,FS,ENV

WEB += $(KEFIR_EMCC_COMPILE_DEPS) \
       $(KEFIR_EMCC_DEPENDENCY_FILES) \
       $(KEFIR_EMCC_OBJECT_FILES) \
       $(KEFIR_JS)

include source/web/app/Makefile.mk
