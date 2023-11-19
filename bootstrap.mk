ROOT=
SOURCE=
HEADERS=
BOOTSTRAP=
BOOTSTRAP_CC=
BOOTSTRAP_EXTRA_CFLAGS=
PLATFORM=
KEFIR_HOST_ENV_CONFIG_HEADER=
GENERATED_HELP_DIR=
KEFIR_AS=$(AS)
KEFIR_LD=ld

BOOTSTRAP_CFLAGS=-I $(HEADERS)
ifeq ($(PLATFORM),freebsd)
BOOTSTRAP_CFLAGS += --target hostcpu-freebsd -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1
else ifeq ($(PLATFORM),openbsd)
BOOTSTRAP_CFLAGS += --target hostcpu-openbsd -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include $(HEADERS)/bootstrap_include/openbsd.h
else ifeq ($(PLATFORM),netbsd)
BOOTSTRAP_CFLAGS += --target hostcpu-netbsd -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -D__ELF__=1 -include $(HEADERS)/bootstrap_include/netbsd.h
else
BOOTSTRAP_CFLAGS += --target hostcpu-linux
endif

ifeq ($(USE_SHARED),yes)
BOOTSTRAP_CFLAGS += -fPIC
endif

BOOTSTRAP_CFLAGS += $(BOOTSTRAP_EXTRA_CFLAGS)

KEFIR_BUILD_SOURCE_ID := $(shell $(SOURCE)/../scripts/get-source-id.sh)
KEFIR_BUILD_CFLAGS := $(BOOTSTRAP_CFLAGS)
BOOTSTRAP_CFLAGS += '-DKEFIR_BUILD_CFLAGS="$(KEFIR_BUILD_CFLAGS)"'
BOOTSTRAP_CFLAGS += '-DKEFIR_BUILD_SOURCE_ID="$(KEFIR_BUILD_SOURCE_ID)"'

ifneq (,$(wildcard $(KEFIR_HOST_ENV_CONFIG_HEADER)))
BOOTSTRAP_CFLAGS += -include $(realpath $(KEFIR_HOST_ENV_CONFIG_HEADER))
endif

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
	$(SOURCE)/codegen/amd64/*.c \
	$(SOURCE)/codegen/amd64/code/*.c \
	$(SOURCE)/codegen/asmcmp/*.c \
	$(SOURCE)/codegen/asmcmp/pipeline/*.c \
	$(SOURCE)/codegen/naive-system-v-amd64/*.c \
	$(SOURCE)/codegen/naive-system-v-amd64/builtins/*.c \
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
	$(SOURCE)/target/abi/amd64/*.c \
	$(SOURCE)/target/abi/amd64/system-v/*.c \
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
	@KEFIR_AS=$(KEFIR_AS) KEFIR_LD=$(KEFIR_LD) $(BOOTSTRAP_CC) $(BOOTSTRAP_CFLAGS) -o $@ $^

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
	@KEFIR_AS=$(KEFIR_AS) $(BOOTSTRAP_CC) $(BOOTSTRAP_CFLAGS) $$(cat $(subst .s,.deps,$@)) -S -o $@ $<

BIN_HEADERS_SRCDIR=$(SOURCE)
BIN_HEADERS_DESTDIR=$(BOOTSTRAP)
include source/binary_headers.mk

$(BOOTSTRAP)/libkefir.so: $(KEFIR_LIB_ASM_FILES)
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(BOOTSTRAP_CC) $(BOOTSTRAP_CFLAGS) -shared $^ -o $@

ifeq ($(USE_SHARED),yes)
$(BOOTSTRAP)/kefir: $(KEFIR_DRIVER_ASM_FILES) $(BOOTSTRAP)/libkefir.so
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(BOOTSTRAP_CC) -pie $(BOOTSTRAP_CFLAGS) $(KEFIR_DRIVER_ASM_FILES) -o $@ -L$(BOOTSTRAP) -lkefir
else
$(BOOTSTRAP)/kefir: $(KEFIR_ASM_FILES)
	@echo "Linking $@"
	@KEFIR_LD=$(KEFIR_LD) $(BOOTSTRAP_CC) $(BOOTSTRAP_CFLAGS) $^ -o $@
endif

bootstrap: $(BOOTSTRAP)/kefir

.COMPILE_DEPS: $(KEFIR_COMPILE_DEPS)
.ASM_FILES: $(KEFIR_ASM_FILES)

.PHONY: bootstrap .COMPILE_DEPS .ASM_FILES