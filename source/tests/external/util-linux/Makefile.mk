
KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/util-linux

KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION := 2.41.3
KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_FILENAME := v$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE := $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)/$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)/util-linux-$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION)
KEFIR_EXTERNAL_TEST_UTIL_LINUX_URL := https://github.com/util-linux/util-linux/archive/refs/tags/$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_SHA256 := 25dc2fd70c6b6bec1c0c97cb11636edd2d5b2645df2324eef4820db3677bd412

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)" && tar xvf "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_ARCHIVE_FILENAME)"
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/util-linux/util-linux.patch"
	@find $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR) -type f -name "*.h" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@find $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR) -type f -name "*.c" -exec sed -i 's/__attribute__/__attribute/g' {} \;
# Fails in the container
	@rm -rf "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/tests/ts/misc/setarch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring util-linux $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -fPIC -isystem $(realpath $(SOURCE_DIR))/tests/external/util-linux/include" \
		sh autogen.sh
	@sed -i \
		-e '0,/^[ ]*lt_prog_compiler_wl=$$/s//lt_prog_compiler_wl=-Wl,/' \
		-e '0,/^[ ]*lt_prog_compiler_pic=$$/s//lt_prog_compiler_pic=-fPIC/' \
		-e '0,/^[ ]*lt_prog_compiler_static=$$/s//lt_prog_compiler_static=-static/' \
		"$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/configure"

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/configure
	@echo "Configuring util-linux $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -fPIC -isystem $(realpath $(SOURCE_DIR))/tests/external/util-linux/include" \
		./configure

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/partx: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/Makefile
	@echo "Building util-linux $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -fPIC -isystem $(realpath $(SOURCE_DIR))/tests/external/util-linux/include" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)/partx
	@echo "Testing util-linux $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -fPIC -isystem $(realpath $(SOURCE_DIR))/tests/external/util-linux/include" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/util-linux.test.done: $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/util-linux/validate.sh "$(KEFIR_EXTERNAL_TEST_UTIL_LINUX_DIR)/tests.log"
	@touch "$@"
	@echo "util-linux $(KEFIR_EXTERNAL_TEST_UTIL_LINUX_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/util-linux.test.done
