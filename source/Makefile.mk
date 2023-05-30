KEFIR_LIB_SOURCE := $(wildcard \
	$(SOURCE_DIR)/ast/*.c \
	$(SOURCE_DIR)/ast/analyzer/*.c \
	$(SOURCE_DIR)/ast/analyzer/nodes/*.c \
	$(SOURCE_DIR)/ast/constant_expression/*.c \
	$(SOURCE_DIR)/ast/nodes/*.c \
	$(SOURCE_DIR)/ast-translator/*.c \
	$(SOURCE_DIR)/ast-translator/nodes/*.c \
	$(SOURCE_DIR)/ast-translator/scope/*.c \
	$(SOURCE_DIR)/ast/type/*.c \
	$(SOURCE_DIR)/core/*.c \
	$(SOURCE_DIR)/codegen/*.c \
	$(SOURCE_DIR)/codegen/opt-common/*.c \
	$(SOURCE_DIR)/codegen/opt-system-v-amd64/*.c \
	$(SOURCE_DIR)/codegen/system-v-amd64/*.c \
	$(SOURCE_DIR)/codegen/system-v-amd64/builtins/*.c \
	$(SOURCE_DIR)/compiler/*.c \
	$(SOURCE_DIR)/ir/*.c \
	$(SOURCE_DIR)/lexer/*.c \
	$(SOURCE_DIR)/lexer/tokens/*.c \
	$(SOURCE_DIR)/lexer/tokens/string_literal/*.c \
	$(SOURCE_DIR)/optimizer/*.c \
	$(SOURCE_DIR)/optimizer/analysis/*.c \
	$(SOURCE_DIR)/parser/*.c \
	$(SOURCE_DIR)/parser/rules/*.c \
	$(SOURCE_DIR)/platform/*.c \
	$(SOURCE_DIR)/preprocessor/*.c \
	$(SOURCE_DIR)/target/abi/*.c \
	$(SOURCE_DIR)/target/abi/system-v-amd64/*.c \
	$(SOURCE_DIR)/target/asm/amd64/*.c \
	$(SOURCE_DIR)/util/*.c)

KEFIR_LIB_DEPENDENCIES := $(KEFIR_LIB_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_LIB_OBJECT_FILES := $(KEFIR_LIB_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.o)
KEFIR_LIB_OBJECT_FILES += $(BIN_DIR)/codegen/system-v-amd64/sysv-amd64-runtime-code.s.o
KEFIR_LIB_OBJECT_FILES += $(BIN_DIR)/compiler/predefined_macro_defs.s.o

KEFIR_BUILD_SOURCE_ID := $(shell $(ROOT)/scripts/get-source-id.sh)
CFLAGS += '-DKEFIR_BUILD_SOURCE_ID="$(KEFIR_BUILD_SOURCE_ID)"'

$(BIN_DIR)/codegen/system-v-amd64/amd64-sysv-runtime-code.s.o: $(SOURCE_DIR)/runtime/amd64_sysv.s
$(BIN_DIR)/compiler/predefined_macro_defs.s.o: $(SOURCE_DIR)/compiler/predefined_macro_defs.h

ifeq ($(USE_SHARED),yes)
BINARIES += $(LIBKEFIR_SO)
LIBKEFIR_DEP=$(LIBKEFIR_SO)
else
LIBKEFIR_DEP=$(LIBKEFIR_A)
endif

$(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION): $(KEFIR_LIB_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(CC) -shared -o $@ $(KEFIR_LIB_OBJECT_FILES)

$(LIBKEFIR_SO): $(LIBKEFIR_SO).$(LIBKEFIR_SO_VERSION)
	@echo "Symlinking $@"
	@ln -sf $(shell basename $<) $@

$(LIBKEFIR_A): $(KEFIR_LIB_OBJECT_FILES)
	@mkdir -p $(shell dirname "$@")
	@echo "Archiving $@"
	@$(AR) cr $@ $^
	@ranlib $@

$(LIBKEFIRRT_A): $(BIN_DIR)/runtime/amd64_sysv.s.o $(BIN_DIR)/runtime/amd64_setjmp.s.o
	@mkdir -p $(shell dirname "$@")
	@echo "Archiving $@"
	@$(AR) cr $@ $^
	@ranlib $@

DEPENDENCIES += $(KEFIR_LIB_DEPENDENCIES)
OBJECT_FILES += $(KEFIR_LIB_OBJECT_FILES)
BINARIES += $(LIBKEFIR_A)
BINARIES += $(LIBKEFIRRT_A)
