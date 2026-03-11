
KEFIR_EXTERNAL_TEST_XZ_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/xz

KEFIR_EXTERNAL_TEST_XZ_VERSION := 5.8.2
KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_FILENAME := xz-$(KEFIR_EXTERNAL_TEST_XZ_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_XZ_ARCHIVE := $(KEFIR_EXTERNAL_TEST_XZ_DIR)/$(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_XZ_DIR)/xz-$(KEFIR_EXTERNAL_TEST_XZ_VERSION)
KEFIR_EXTERNAL_TEST_XZ_URL := https://github.com/tukaani-project/xz/releases/download/v$(KEFIR_EXTERNAL_TEST_XZ_VERSION)/$(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_SHA256 := ce09c50a5962786b83e5da389c90dd2c15ecd0980a258dd01f70f9e7ce58a8f1

$(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_XZ_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_XZ_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_XZ_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_XZ_ARCHIVE_FILENAME)"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build/Makefile: $(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configure xz $(KEFIR_EXTERNAL_TEST_XZ_VERSION)..."
	@mkdir -p "$(shell dirname $@)"
	@cd "$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		cmake -DCMAKE_C_COMPILER="$(realpath $(KEFIR_EXE))" ..

$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build/xz: $(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build/Makefile
	@echo "Building xz $(KEFIR_EXTERNAL_TEST_XZ_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_XZ_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build/xz
	@echo "Testing xz $(KEFIR_EXTERNAL_TEST_XZ_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_XZ_SOURCE_DIR)/build" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) test 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/xz.test.done: $(KEFIR_EXTERNAL_TEST_XZ_DIR)/tests.log
	@"$(SOURCE_DIR)/tests/external/xz/validate.sh" "$(KEFIR_EXTERNAL_TEST_XZ_DIR)/tests.log"
	@touch "$@"
	@echo "xz $(KEFIR_EXTERNAL_TEST_XZ_VERSION) test successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/xz.test.done
