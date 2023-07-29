CC=gcc
LD=$(CC)
AS=as
AR=ar
EMCC=emcc
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

ifeq ($(USE_SHARED),yes)
CFLAGS+=-fPIC
endif
CFLAGS+=$(OPT) $(DBG) $(EXTRAFLAGS)

ifeq ($(SANITIZE),undefined)
SANFLAGS=-fsanitize=undefined -fno-sanitize-recover=all
CFLAGS+=$(SANFLAGS)
endif

ROOT=.
BIN_DIR=$(ROOT)/bin
LIB_DIR=$(BIN_DIR)/libs
SOURCE_DIR=$(ROOT)/source
HEADERS_DIR=$(ROOT)/headers
SCRIPTS_DIR=$(ROOT)/scripts
BOOTSTRAP_DIR=$(ROOT)/bootstrap

LIBKEFIR_SO=$(LIB_DIR)/libkefir.so
LIBKEFIR_SO_VERSION=0.0
LIBKEFIR_A=$(LIB_DIR)/libkefir.a
LIBKEFIRRT_A=$(LIB_DIR)/libkefirrt.a
KEFIR_EXE=$(BIN_DIR)/kefir
KEFIR_CC1_EXE=$(BIN_DIR)/kefir-cc1
KEFIR_JS=$(BIN_DIR)/kefir.js
HEXDUMP_EXE=$(BIN_DIR)/hexdump

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

$(BIN_DIR)/%.deps:
	@mkdir -p $(shell dirname "$@")
	@touch $@

$(BIN_DIR)/%.d: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.deps
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(CC) $(INCLUDES) $$(cat $(subst .d,.deps,$@)) -MM -MT '$(@:.d=.o)' $< > $@

$(BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d $(BIN_DIR)/%.deps
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $$(cat $(subst .o,.deps,$@)) -c $< -o $@

$(BIN_DIR)/%.s.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(AS) -o $@ $<

$(BIN_DIR)/%.binary.h: $(HEXDUMP_EXE)
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(HEXDUMP_EXE) $(BINARY_HEADER_CONTENT) > $@
