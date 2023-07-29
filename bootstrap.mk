ROOT=
SOURCE=
HEADERS=
BOOTSTRAP=
KEFIRCC=
KEFIR_EXTRAFLAGS=
PLATFORM=
KEFIR_AS=$(AS)
KEFIR_LD=ld

KEFIR_FLAGS=-I $(HEADERS)
ifeq ($(PLATFORM),freebsd)
KEFIR_FLAGS += --target x86_64-freebsd-system -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1
else ifeq ($(PLATFORM),openbsd)
KEFIR_FLAGS += --target x86_64-openbsd-system -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include $(HEADERS)/bootstrap_include/openbsd.h
else ifeq ($(PLATFORM),netbsd)
KEFIR_FLAGS += --target x86_64-netbsd-system -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -D__ELF__=1 -include $(HEADERS)/bootstrap_include/netbsd.h
else
KEFIR_FLAGS += --target x86_64-linux-gnu
endif

ifeq ($(USE_SHARED),yes)
KEFIR_FLAGS += -fPIC
endif

KEFIR_FLAGS += $(KEFIR_EXTRAFLAGS)

KEFIR_BUILD_SOURCE_ID := $(shell $(SOURCE)/../scripts/get-source-id.sh)
KEFIR_FLAGS += '-DKEFIR_BUILD_SOURCE_ID="$(KEFIR_BUILD_SOURCE_ID)"'

KEFIR_LIB_SOURCE := $(wildcard \
	$(SOURCE)/ast/*.c \
	$(SOURCE)/ast/analyzer/*.c \
	$(SOURCE)/ast/analyzer/nodes/*.c \
	$(SOURCE)/ast/constant_expression/*.c \
	$(SOURCE)/ast/nodes/*.c \
	$(SOURCE)/ast-translator/*.c \
	$(SOURCE)/ast-translator/nodes/*.c \
	$(SOURCE)/ast-translator/scope/*.c \
	$(SOURCE)/ast/type/*.c \
	$(SOURCE)/core/*.c \
	$(SOURCE)/codegen/*.c \
	$(SOURCE)/codegen/opt-system-v-amd64/*.c \
	$(SOURCE)/codegen/opt-system-v-amd64/code/*.c \
	$(SOURCE)/codegen/opt-system-v-amd64/code/inline_assembly/*.c \
	$(SOURCE)/codegen/system-v-amd64/*.c \
	$(SOURCE)/codegen/system-v-amd64/builtins/*.c \
	$(SOURCE)/compiler/*.c \
	$(SOURCE)/ir/*.c \
	$(SOURCE)/lexer/*.c \
	$(SOURCE)/lexer/tokens/*.c \
	$(SOURCE)/lexer/tokens/string_literal/*.c \
	$(SOURCE)/optimizer/*.c \
	$(SOURCE)/optimizer/analysis/*.c \
	$(SOURCE)/optimizer/pipeline/*.c \
	$(SOURCE)/parser/*.c \
	$(SOURCE)/parser/rules/*.c \
	$(SOURCE)/platform/*.c \
	$(SOURCE)/preprocessor/*.c \
	$(SOURCE)/target/abi/*.c \
	$(SOURCE)/target/abi/system-v-amd64/*.c \
	$(SOURCE)/target/asm/amd64/*.c \
	$(SOURCE)/util/*.c)
KEFIR_LIB_ASM_FILES := $(KEFIR_LIB_SOURCE:$(SOURCE)/%.c=$(BOOTSTRAP)/%.s)

KEFIR_DRIVER_SOURCE := $(wildcard $(SOURCE)/driver/*.c)
KEFIR_DRIVER_ASM_FILES := $(KEFIR_DRIVER_SOURCE:$(SOURCE)/%.c=$(BOOTSTRAP)/%.s)

KEFIR_SOURCE := $(KEFIR_LIB_SOURCE)
KEFIR_SOURCE += $(KEFIR_DRIVER_SOURCE)
KEFIR_COMPILE_DEPS := $(KEFIR_SOURCE:$(SOURCE)/%.c=$(BOOTSTRAP)/%.deps)
KEFIR_ASM_FILES := $(KEFIR_LIB_ASM_FILES)
KEFIR_ASM_FILES += $(KEFIR_DRIVER_ASM_FILES)

$(BOOTSTRAP)/hexdump: $(ROOT)/util/hexdump.c
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(KEFIRCC) $(KEFIR_FLAGS) -o $@ $^

$(BOOTSTRAP)/%.binary.h: $(BOOTSTRAP)/hexdump
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(BOOTSTRAP)/hexdump $(BINARY_HEADER_CONTENT) > $@

$(BOOTSTRAP)/%.deps:
	@mkdir -p $(shell dirname "$@")
	@touch $@

$(BOOTSTRAP)/%.s: $(SOURCE)/%.c $(BOOTSTRAP)/%.deps
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Compile $<"
	@KEFIR_AS=$(KEFIR_AS) $(KEFIRCC) $(KEFIR_FLAGS) $$(cat $(subst .s,.deps,$@)) -S -o $@ $<

BIN_HEADERS_SRCDIR=$(SOURCE)
BIN_HEADERS_DESTDIR=$(BOOTSTRAP)
include source/binary_headers.mk

$(BOOTSTRAP)/libkefir.so: $(KEFIR_LIB_ASM_FILES)
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(KEFIRCC) $(KEFIR_FLAGS) -shared $^ -o $@

ifeq ($(USE_SHARED),yes)
$(BOOTSTRAP)/kefir: $(KEFIR_DRIVER_ASM_FILES) $(BOOTSTRAP)/libkefir.so
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(KEFIRCC) -pie $(KEFIR_FLAGS) $(KEFIR_DRIVER_ASM_FILES) -o $@ -L$(BOOTSTRAP) -lkefir
else
$(BOOTSTRAP)/kefir: $(KEFIR_ASM_FILES)
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(KEFIRCC) $(KEFIR_FLAGS) $^ -o $@
endif

bootstrap: $(BOOTSTRAP)/kefir

.COMPILE_DEPS: $(KEFIR_COMPILE_DEPS)
.ASM_FILES: $(KEFIR_ASM_FILES)

.PHONY: bootstrap .COMPILE_DEPS .ASM_FILES