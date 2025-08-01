KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/gcc-misc-tests

KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION := 15.1.0

KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME := gcc-$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_GCC_URL := https://ftp.mpi-inf.mpg.de/mirrors/gnu/mirror/gcc.gnu.org/pub/gcc/releases/gcc-$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION)/gcc-$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_GCC_ARCHIVE := $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)/$(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)/gcc-$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION)

KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_SHA256 := e2b09ec21660f01fecffb715e0120265216943f038d0e48a9868713e54f06cea

$(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE):
	@mkdir -p "$(dir $@)"
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME)..."
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GCC_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE)
	@echo "Unpacking $(basename $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE))..."
	@rm -rf "$(KEFIR_EXTERNAL_TEST_GCC_DIR)"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)" && \
		rm -f "$(basename $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME))" && \
		xz -d < "$(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME)" > "$(basename $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME))"
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)" && tar xvf "$(basename $(KEFIR_EXTERNAL_TEST_GCC_ARCHIVE_FILENAME))"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_SOURCE_DIR)/.extracted $(KEFIR_EXE) $(SOURCE_DIR)/tests/external/gcc-misc-tests/tests.lst
	@echo "Running parts of GCC $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION) test suite..."
	@cd "$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/gcc-torture/torture.h -O1 -g -fPIC -std=c23" \
		bash -c 'set -o pipefail; "$(realpath $(SOURCE_DIR))/tests/external/gcc-misc-tests/run.sh" "gcc-$(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION)" "$(realpath $(SOURCE_DIR))/tests/external/gcc-misc-tests/tests.lst" | tee "$(shell realpath $@.tmp)"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/gcc-misc-tests.test.done: $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_DIR)/tests.log
	@echo "Succesfully run parts of GCC $(KEFIR_EXTERNAL_TEST_GCC_MISC_TESTS_VERSION) test suite..."
	@touch "$@"

KEFIR_GCC_MISC_TESTS_TEST = $(KEFIR_EXTERNAL_TESTS_DIR)/gcc-misc-tests.test.done
EXTERNAL_TESTS_BASE_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/gcc-misc-tests.test.done
