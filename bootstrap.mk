SOURCE=
HEADERS=
BOOTSTRAP=
KEFIRCC=
PLATFORM=
AS=as
LD=ld

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

KEFIR_BUILD_SOURCE_ID := $(shell $(SOURCE)/../scripts/get-source-id.sh)
KEFIR_FLAGS += '-DKEFIR_BUILD_SOURCE_ID="$(KEFIR_BUILD_SOURCE_ID)"'

KEFIR_SOURCE := $(wildcard \
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
	$(SOURCE)/codegen/system-v-amd64/*.c \
	$(SOURCE)/codegen/system-v-amd64/builtins/*.c \
	$(SOURCE)/compiler/*.c \
	$(SOURCE)/ir/*.c \
	$(SOURCE)/lexer/*.c \
	$(SOURCE)/lexer/tokens/*.c \
	$(SOURCE)/lexer/tokens/string_literal/*.c \
	$(SOURCE)/parser/*.c \
	$(SOURCE)/parser/rules/*.c \
	$(SOURCE)/platform/*.c \
	$(SOURCE)/preprocessor/*.c \
	$(SOURCE)/target/abi/*.c \
	$(SOURCE)/target/abi/system-v-amd64/*.c \
	$(SOURCE)/target/asm/*.c \
	$(SOURCE)/util/*.c \
	$(SOURCE)/driver/*.c)

KEFIR_ASM_FILES := $(KEFIR_SOURCE:$(SOURCE)/%.c=$(BOOTSTRAP)/%.s)
KEFIR_ASM_FILES += $(SOURCE)/runtime/amd64_sysv.s
KEFIR_ASM_FILES += $(SOURCE)/driver/help.s
KEFIR_ASM_FILES += $(SOURCE)/codegen/system-v-amd64/sysv-amd64-runtime-code.s
KEFIR_ASM_FILES += $(SOURCE)/compiler/predefined_macro_defs.s

$(BOOTSTRAP)/%.s: $(SOURCE)/%.c
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Compile $^"
	@KEFIR_AS=$(AS) $(KEFIRCC) $(KEFIR_FLAGS) -S -o $@ $<

$(BOOTSTRAP)/%.s.o: $(SOURCE)/%.s
	@echo "Assemble $^"
	@KEFIR_AS=$(AS) $(KEFIRCC) $(KEFIR_FLAGS) -c -o $@ $<

$(BOOTSTRAP)/runtime.o: $(SOURCE)/runtime/amd64_sysv.s
	@mkdir -p $(shell dirname "$@")
	@echo "Assemble $^"
	@KEFIR_AS=$(AS) $(KEFIRCC) $(KEFIR_FLAGS) -c -o $@ $<

$(BOOTSTRAP)/driver/help.s.o: $(SOURCE)/driver/help.txt

$(BOOTSTRAP)/codegen/system-v-amd64/amd64-sysv-runtime-code.s.o: $(SOURCE)/runtime/amd64_sysv.s

$(BOOTSTRAP)/kefir: $(KEFIR_ASM_FILES)
	@echo "Linking $@"
	@KEFIR_LD=$(LD) $(KEFIRCC) $(KEFIR_FLAGS) $^ -o $@

bootstrap: $(BOOTSTRAP)/kefir

.ASM_FILES: $(KEFIR_ASM_FILES)

.PHONY: bootstrap .ASM_FILES .OBJECT_FILES