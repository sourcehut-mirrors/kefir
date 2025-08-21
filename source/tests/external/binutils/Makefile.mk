
KEFIR_EXTERNAL_TEST_BINUTILS_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/binutils

KEFIR_EXTERNAL_TEST_BINUTILS_VERSION := 2.45
KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_FILENAME := binutils-$(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE := $(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)/$(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)/binutils-$(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)
KEFIR_EXTERNAL_TEST_BINUTILS_URL := https://sourceware.org/pub/binutils/releases/binutils-$(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_SHA256 := 8a3eb4b10e7053312790f21ee1a38f7e2bbd6f4096abb590d3429e5119592d96

$(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_BINUTILS_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_BINUTILS_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_BINUTILS_ARCHIVE_FILENAME)"
	@echo "Patching binutils $(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/binutils/binutils-$(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION).patch"
	@rm "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/libctf/testsuite/libctf-lookup/unnamed-field-info.c"
	@rm "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/libctf/testsuite/libctf-lookup/unnamed-field-info.lk"
	@find "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)" -name "*.c" -exec sed -i "s/__attribute__/__attribute/g" {} \;
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring binutils $(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g" \
		KEFIR_AS="$(AS)" \
		KEFIR_LD="$(LD)" \
		./configure --enable-gprofng=no

$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/ld/ld-new: $(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/Makefile
	@echo "Building binutils $(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g" \
		KEFIR_AS="$(AS)" \
		KEFIR_LD="$(LD)" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)/ld/ld-new
	@echo "Testing binutils $(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BINUTILS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g" \
		KEFIR_AS="$(AS)" \
		KEFIR_LD="$(LD)" \
		bash --noprofile --norc -c 'set -o pipefail; $(MAKE) check CC_FOR_TARGET="$(CC)" CC_FOR_BUILD="$(CC)" -j1 2>&1 | tee "$(shell realpath $@.tmp)"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/binutils.test.done: $(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)/tests.log
	@"$(SOURCE_DIR)/tests/external/binutils/validate.sh" "$(KEFIR_EXTERNAL_TEST_BINUTILS_DIR)/tests.log"
	@touch "$@"
	@echo "Successfully tested binutils $(KEFIR_EXTERNAL_TEST_BINUTILS_VERSION)"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/binutils.test.done
