KEFIR_EMCC_BIN_DIR=$(BIN_DIR)/web

KEFIR_EMCC_SOURCE = $(KEFIR_LIB_SOURCE)
KEFIR_EMCC_SOURCE += $(wildcard $(SOURCE_DIR)/web/*.c)
KEFIR_EMCC_SOURCE += $(filter-out $(SOURCE_DIR)/driver/main.c,$(wildcard $(SOURCE_DIR)/driver/*.c))

KEFIR_EMCC_DEPENDENCY_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_EMCC_OBJECT_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(KEFIR_EMCC_BIN_DIR)/%.o)

$(KEFIR_EMCC_BIN_DIR)/%.bin.h:
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $(BIN_HEADER_CONTENT) | xxd -i > $@

$(KEFIR_EMCC_BIN_DIR)/runtime/opt_amd64_sysv.bin.h: $(SOURCE_DIR)/runtime/opt_amd64_sysv.s $(SOURCE_DIR)/runtime/common_amd64.inc.s
$(KEFIR_EMCC_BIN_DIR)/runtime/opt_amd64_sysv.bin.h: BIN_HEADER_CONTENT=$(SOURCE_DIR)/runtime/opt_amd64_sysv.s $(SOURCE_DIR)/runtime/common_amd64.inc.s
$(KEFIR_EMCC_BIN_DIR)/runtime/amd64_sysv.bin.h: $(SOURCE_DIR)/runtime/amd64_sysv.s $(SOURCE_DIR)/runtime/common_amd64.inc.s
$(KEFIR_EMCC_BIN_DIR)/runtime/amd64_sysv.bin.h: BIN_HEADER_CONTENT=$(SOURCE_DIR)/runtime/amd64_sysv.s $(SOURCE_DIR)/runtime/common_amd64.inc.s
$(KEFIR_EMCC_BIN_DIR)/compiler/predefined_defs.bin.h: $(SOURCE_DIR)/compiler/predefined_defs.h
$(KEFIR_EMCC_BIN_DIR)/compiler/predefined_defs.bin.h: BIN_HEADER_CONTENT=$(SOURCE_DIR)/compiler/predefined_defs.h
$(KEFIR_EMCC_BIN_DIR)/driver/help.bin.h: $(SOURCE_DIR)/driver/help.txt
$(KEFIR_EMCC_BIN_DIR)/driver/help.bin.h: BIN_HEADER_CONTENT=$(SOURCE_DIR)/driver/help.txt

$(KEFIR_EMCC_BIN_DIR)/compiler/profile.o: $(KEFIR_EMCC_BIN_DIR)/runtime/opt_amd64_sysv.bin.h
$(KEFIR_EMCC_BIN_DIR)/compiler/profile.o: $(KEFIR_EMCC_BIN_DIR)/runtime/amd64_sysv.bin.h
$(KEFIR_EMCC_BIN_DIR)/compiler/profile.o: CFLAGS+=-I$(ROOT) -DKEFIR_OPT_AMD64_SYSV_RUNTIME_INCLUDE=$(KEFIR_EMCC_BIN_DIR)/runtime/opt_amd64_sysv.bin.h -DKEFIR_AMD64_SYSV_RUNTIME_INCLUDE=$(KEFIR_EMCC_BIN_DIR)/runtime/amd64_sysv.bin.h
$(KEFIR_EMCC_BIN_DIR)/compiler/compiler.o: $(KEFIR_EMCC_BIN_DIR)/compiler/predefined_defs.bin.h
$(KEFIR_EMCC_BIN_DIR)/compiler/compiler.o: CFLAGS+=-I$(ROOT) -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=$(KEFIR_EMCC_BIN_DIR)/compiler/predefined_defs.bin.h
$(KEFIR_EMCC_BIN_DIR)/web/main.o: $(KEFIR_EMCC_BIN_DIR)/driver/help.bin.h
$(KEFIR_EMCC_BIN_DIR)/web/main.o: CFLAGS+=-I$(ROOT) -DKEFIR_DRIVER_HELP_INCLUDE=$(KEFIR_EMCC_BIN_DIR)/driver/help.bin.h

$(KEFIR_EMCC_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(EMCC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KEFIR_JS): $(KEFIR_EMCC_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(EMCC) $(CFLAGS) $^ -o $@ \
		-sMODULARIZE -s'EXPORT_NAME="createKefirModule"' \
		-sEXPORTED_FUNCTIONS=UTF8ToString,stringToUTF8Array,lengthBytesUTF8,_free,_malloc,_kefir_run_with_args \
		-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,setValue,FS,ENV

WEB += $(KEFIR_EMCC_DEPENDENCY_FILES) \
       $(KEFIR_EMCC_OBJECT_FILES) \
       $(KEFIR_JS)

include source/web/app/Makefile.mk
