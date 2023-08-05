CC=cc
LD=$(CC)
AS=as
AR=ar
EMCC=emcc
GZIP=gzip
USE_SHARED=yes

OPT=-O0
DBG=-g3 -ggdb -DKFT_NOFORK
EXTRAFLAGS=
CFLAGS=-std=c11 -Wall -Wextra -pedantic -Wno-overlength-strings -Wstrict-prototypes -Wformat=2 -Wno-format-nonliteral -Wundef -Wunreachable-code  -fno-common
LDFLAGS=
INCLUDES=-Iheaders
SANFLAGS=
PLATFORM=

ifeq ($(REALPATH),)
REALPATH=realpath
endif

MDOC_CONV=groff -mandoc
ifeq ($(PLATFORM),freebsd)
MDOC_CONV=mandoc -mdoc
endif
ifeq ($(PLATFORM),openbsd)
MDOC_CONV=mandoc -mdoc
endif
ifeq ($(PLATFORM),netbsd)
MDOC_CONV=mandoc -mdoc
endif

ifeq ($(USE_SHARED),yes)
CFLAGS+=-fPIC
endif
CFLAGS+=$(OPT) $(DBG) $(EXTRAFLAGS)

DETECT_HOST_ENV=yes
DETECT_HOST_ENV_CC:=$(CC)
DETECT_HOST_ENV_CFLAGS:=$(CFLAGS)

ifeq ($(SANITIZE),undefined)
SANFLAGS=-fsanitize=undefined -fno-sanitize-recover=all
CFLAGS+=$(SANFLAGS)
endif

ROOT=.
KEFIR_BIN_DIR=$(ROOT)/bin
LIB_DIR=$(KEFIR_BIN_DIR)/libs
SOURCE_DIR=$(ROOT)/source
HEADERS_DIR=$(ROOT)/headers
SCRIPTS_DIR=$(ROOT)/scripts
BOOTSTRAP_DIR=$(ROOT)/bootstrap
DOCS_DIR=$(ROOT)/docs
DOCS_MAN_DIR=$(DOCS_DIR)/man
GENERATED_HELP_DIR=$(KEFIR_BIN_DIR)/help

KEFIR_HOST_ENV_CONFIG_HEADER=$(KEFIR_BIN_DIR)/config.h
LIBKEFIR_SO=$(LIB_DIR)/libkefir.so
LIBKEFIR_SO_VERSION=0.0
LIBKEFIR_A=$(LIB_DIR)/libkefir.a
LIBKEFIRRT_A=$(LIB_DIR)/libkefirrt.a
KEFIR_EXE=$(KEFIR_BIN_DIR)/kefir
KEFIR_CC1_EXE=$(KEFIR_BIN_DIR)/kefir-cc1
KEFIR_JS=$(KEFIR_BIN_DIR)/kefir.js
HEXDUMP_EXE=$(KEFIR_BIN_DIR)/hexdump

COMPILE_DEPS :=
DEPENDENCIES :=
TEST_ARTIFACTS :=
ASM_FILES :=
OBJECT_FILES :=
BINARIES :=
TEST_BINARIES :=
TEST_RESULTS :=
TESTS :=
BOOTSTRAP :=
WEB :=
WEBAPP :=
MAN_PAGES :=

$(KEFIR_HOST_ENV_CONFIG_HEADER):
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
ifeq ($(DETECT_HOST_ENV),yes)
	@CC='$(DETECT_HOST_ENV_CC)' CFLAGS='$(DETECT_HOST_ENV_CFLAGS)' $(SCRIPTS_DIR)/detect-host-env.sh --header > "$@" && true || echo -n > "$@"
else
	@echo -n > "$@"
endif

$(KEFIR_BIN_DIR)/%.deps:
	@mkdir -p $(shell dirname "$@")
	@touch $@

$(KEFIR_BIN_DIR)/%.d: $(SOURCE_DIR)/%.c $(KEFIR_BIN_DIR)/%.deps $(KEFIR_HOST_ENV_CONFIG_HEADER)
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(CC) $(INCLUDES) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" $$(cat $(subst .d,.deps,$@)) -MM -MT '$(@:.d=.o)' $< > $@

$(KEFIR_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(KEFIR_BIN_DIR)/%.d $(KEFIR_BIN_DIR)/%.deps $(KEFIR_HOST_ENV_CONFIG_HEADER)
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" $$(cat $(subst .o,.deps,$@)) -c $< -o $@

$(KEFIR_BIN_DIR)/%.s.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(AS) -o $@ $<

$(KEFIR_BIN_DIR)/%.binary.h: $(HEXDUMP_EXE)
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(HEXDUMP_EXE) $(BINARY_HEADER_CONTENT) > $@
