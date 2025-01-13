# Build profile
PROFILE=release
USE_SHARED=yes
USE_SANITIZER=no
USE_VALGRIND=no
USE_GCOV=no
PLATFORM := $(shell uname | tr '[:upper:]' '[:lower:]')

# Tools
CC=cc
CCLD=$(CC)
LD=ld
AS=as
AR=ar
STRIP=strip
EMCC=emcc
GZIP=gzip
REALPATH=realpath

# Platform-dependent tools
MDOC_CONV=groff -mandoc
ifeq ($(PLATFORM),freebsd)
MDOC_CONV=mandoc -mdoc
REALPATH=grealpath
endif
ifeq ($(PLATFORM),openbsd)
MDOC_CONV=mandoc -mdoc
REALPATH=grealpath
endif
ifeq ($(PLATFORM),netbsd)
MDOC_CONV=mandoc -mdoc
REALPATH=grealpath
endif

# Build flags
CFLAGS=-std=c11 -fno-common
PROFILE_CFLAGS=
EXTRA_CFLAGS=
LDFLAGS=
SANITIZER_FLAGS=
ifeq ($(USE_SHARED),yes)
CFLAGS+=-fPIC
endif
WARNING_CFLAGS=-Wall -Wextra -pedantic -Wno-overlength-strings -Wstrict-prototypes -Wformat=2 -Wno-format-nonliteral -Wundef -Wunreachable-code 
ifeq ($(PROFILE),debug)
PROFILE_CFLAGS=-O0 -g3 -ggdb $(WARNING_CFLAGS)
endif
ifeq ($(PROFILE),reldebug)
PROFILE_CFLAGS=-O3 -g3 -ggdb $(WARNING_CFLAGS)
endif
ifeq ($(PROFILE),devrelease)
PROFILE_CFLAGS=-O3 -DNDEBUG
endif
ifeq ($(PROFILE),release)
PROFILE_CFLAGS=-O3 -DNDEBUG -DKEFIR_BUILD_RELEASE
endif
ifeq ($(USE_SANITIZER),yes)
SANITIZER_FLAGS=-fsanitize=undefined -fno-sanitize-recover=all
endif
ifeq ($(USE_GCOV),yes)
CFLAGS+=-fprofile-arcs -ftest-coverage
LDFLAGS+=-lgcov --coverage
endif
CFLAGS+=$(PROFILE_CFLAGS) $(SANITIZER_FLAGS) $(EXTRA_CFLAGS)

# Host environment detection
DETECT_HOST_ENV=yes
DETECT_HOST_ENV_CC:=$(CC)
DETECT_HOST_ENV_CFLAGS:=$(CFLAGS)

# Directories
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

# Build artifacts
KEFIR_HOST_ENV_CONFIG_HEADER=$(KEFIR_BIN_DIR)/config.h
LIBKEFIR_SO=$(LIB_DIR)/libkefir.so
LIBKEFIR_SO_VERSION=0.0
LIBKEFIR_A=$(LIB_DIR)/libkefir.a
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
EXTERNAL_TESTS_BASE_SUITE :=
EXTERNAL_TESTS_FAST_SUITE :=
EXTERNAL_TESTS_SLOW_SUITE :=
EXTERNAL_TESTS_EXTRA_SUITE :=
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
	@$(CC) -I$(HEADERS_DIR) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" $$(cat $(subst .d,.deps,$@)) -MM -MT '$(@:.d=.o)' $< > $@

$(KEFIR_BIN_DIR)/%.o: $(SOURCE_DIR)/%.c $(KEFIR_BIN_DIR)/%.d $(KEFIR_BIN_DIR)/%.deps $(KEFIR_HOST_ENV_CONFIG_HEADER)
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(CC) $(CFLAGS) -I$(HEADERS_DIR) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" $$(cat $(subst .o,.deps,$@)) -c $< -o $@

$(KEFIR_BIN_DIR)/%.s.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(shell dirname "$@")
	@echo "Building $@"
	@$(AS) -o $@ $<

$(KEFIR_BIN_DIR)/%.binary.h: $(HEXDUMP_EXE)
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(HEXDUMP_EXE) $(BINARY_HEADER_CONTENT) > $@
