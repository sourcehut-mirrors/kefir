EMCC=emcc
EMCC_CFGLAGS=$(CFLAGS)

WEB_BIN_DIR=$(BIN_DIR)/web
EMSCRIPTEN_BIN_DIR=$(WEB_BIN_DIR)/emscripten
KEFIR_JS=$(WEB_BIN_DIR)/kefir.js

KEFIR_EMCC_SOURCE = $(KEFIR_LIB_SOURCE)
KEFIR_EMCC_SOURCE += $(wildcard $(SOURCE_DIR)/emscripten/*.c)
KEFIR_EMCC_SOURCE += $(filter-out $(SOURCE_DIR)/driver/main.c,$(wildcard $(SOURCE_DIR)/driver/*.c))
KEFIR_EMCC_DEPENDENCY_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_EMCC_OBJECT_FILES := $(KEFIR_EMCC_SOURCE:$(SOURCE_DIR)/%.c=$(EMSCRIPTEN_BIN_DIR)/%.o)

$(EMSCRIPTEN_BIN_DIR)/runtime/%.h: $(SOURCE_DIR)/runtime/%.s $(SOURCE_DIR)/runtime/common_amd64.s
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h: $(SOURCE_DIR)/compiler/predefined_defs.h
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/driver/help.h: $(SOURCE_DIR)/driver/help.txt
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@cat $^ | xxd -i > $@

$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: $(EMSCRIPTEN_BIN_DIR)/runtime/opt_amd64_sysv.h
$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: $(EMSCRIPTEN_BIN_DIR)/runtime/amd64_sysv.h
$(EMSCRIPTEN_BIN_DIR)/compiler/profile.o: CFLAGS+=-I$(ROOT) -DKEFIR_OPT_AMD64_SYSV_RUNTIME_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/runtime/opt_amd64_sysv.h -DKEFIR_AMD64_SYSV_RUNTIME_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/runtime/amd64_sysv.h

$(EMSCRIPTEN_BIN_DIR)/compiler/compiler.o: $(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h
$(EMSCRIPTEN_BIN_DIR)/compiler/compiler.o: CFLAGS+=-I$(ROOT) -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/compiler/predefined_defs.h

$(EMSCRIPTEN_BIN_DIR)/emscripten/main.o: $(EMSCRIPTEN_BIN_DIR)/driver/help.h
$(EMSCRIPTEN_BIN_DIR)/emscripten/main.o: CFLAGS+=-I$(ROOT) -DKEFIR_DRIVER_HELP_INCLUDE=$(EMSCRIPTEN_BIN_DIR)/driver/help.h

$(EMSCRIPTEN_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(EMCC) $(EMCC_CFGLAGS) $(INCLUDES) -c $< -o $@

$(KEFIR_JS): $(KEFIR_EMCC_OBJECT_FILES)
	@echo "Linking $@"
	@$(EMCC) $(EMCC_CFGLAGS) $^ -o $@ \
		-sMODULARIZE -s'EXPORT_NAME="createKefirModule"' \
		-sEXPORTED_FUNCTIONS=UTF8ToString,stringToUTF8Array,lengthBytesUTF8,_free,_malloc,_kefir_run_with_args \
		-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,setValue,FS

WEB += $(KEFIR_EMCC_DEPENDENCY_FILES)
WEB += $(KEFIR_EMCC_OBJECT_FILES)
WEB += $(KEFIR_JS)
