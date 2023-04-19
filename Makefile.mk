CC=gcc
AS=as
AR=ar
USE_SHARED=yes

OPT=-O0
DBG=-g3 -ggdb -DKFT_NOFORK
EXTRAFLAGS=
CFLAGS=-std=c11 -Wall -Wextra -pedantic -Wno-overlength-strings -Wstrict-prototypes -Wformat=2 -Wno-format-nonliteral -Wundef -Wunreachable-code -Wstrict-aliasing=2  -fno-common
INCLUDES=-Iheaders
SANFLAGS=
PLATFORM=

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
GENERATED_DIR=$(BIN_DIR)/generated
SOURCE_DIR=$(ROOT)/source
HEADERS_DIR=$(ROOT)/headers
SCRIPTS_DIR=$(ROOT)/scripts
BOOTSTRAP_DIR=$(ROOT)/bootstrap

LIBKEFIR_SO=$(LIB_DIR)/libkefir.so
LIBKEFIR_SO_VERSION=0.0
LIBKEFIR_A=$(LIB_DIR)/libkefir.a

LIBKEFIRRT_A=$(LIB_DIR)/libkefirrt.a
LIBKEFIR_X86_64_LIBC_A=$(LIB_DIR)/libkefir_x86_64_libc.a

DEPENDENCIES :=
ASM_FILES :=
OBJECT_FILES :=
BINARIES :=
TEST_BINARIES :=
TEST_RESULTS :=
TESTS :=

$(BIN_DIR)/%.d: $(SOURCE_DIR)/%.c
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(CC) $(INCLUDES) -MM -MT '$(@:.d=.o)' $< > $@

$(BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(BIN_DIR)/%.d
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR)/%.s.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(AS) -o $@ $<
