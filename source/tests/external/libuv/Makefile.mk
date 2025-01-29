
KEFIR_EXTERNAL_TEST_LIBUV_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/libuv

KEFIR_EXTERNAL_TEST_LIBUV_VERSION := 1.49.2
KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_FILENAME := libuv-v$(KEFIR_EXTERNAL_TEST_LIBUV_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE := $(KEFIR_EXTERNAL_TEST_LIBUV_DIR)/$(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_LIBUV_DIR)/libuv-v$(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)
KEFIR_EXTERNAL_TEST_LIBUV_URL := https://dist.libuv.org/dist/v$(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)/libuv-v$(KEFIR_EXTERNAL_TEST_LIBUV_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_SHA256 := 8c10706bd2cf129045c42b94799a92df9aaa75d05f07e99cf083507239bae5a8

$(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_LIBUV_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBUV_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBUV_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_LIBUV_ARCHIVE_FILENAME)"
	@echo "Applying $(SOURCE_DIR)/tests/external/libuv/libuv-1.49.2.patch..."
	@cd $(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR) && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/libuv/libuv-1.49.2.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring libuv $(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		sh autogen.sh

$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/configure
	@echo "Configuring libuv $(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/libuv/include -g -fPIC -O1" \
		./configure

$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/.libs/libuv.so: $(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/Makefile
	@echo "Building libuv $(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_LIBUV_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)/.libs/libuv.so
	@echo "Testing libuv $(KEFIR_EXTERNAL_TEST_LIBUV_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBUV_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 -j1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/libuv.test.done: $(KEFIR_EXTERNAL_TEST_LIBUV_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/libuv/validate.sh "$(KEFIR_EXTERNAL_TEST_LIBUV_DIR)/tests.log"
	@touch "$@"
	@echo "libuv $(KEFIR_EXTERNAL_TEST_LIBUV_VERSION) test suite successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/libuv.test.done
