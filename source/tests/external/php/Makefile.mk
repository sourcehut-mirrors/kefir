
KEFIR_EXTERNAL_TEST_PHP_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/php

KEFIR_EXTERNAL_TEST_PHP_VERSION := 8.4.2
KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_FILENAME := php-$(KEFIR_EXTERNAL_TEST_PHP_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_PHP_ARCHIVE := $(KEFIR_EXTERNAL_TEST_PHP_DIR)/$(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_PHP_DIR)/php-$(KEFIR_EXTERNAL_TEST_PHP_VERSION)
KEFIR_EXTERNAL_TEST_PHP_URL := https://www.php.net/distributions/php-$(KEFIR_EXTERNAL_TEST_PHP_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_SHA256 := 5d3cf82a7f4cafdcfc4f3d98f3e3ee81077ae57c709a5613cbff5834d78a7747

$(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_PHP_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_PHP_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PHP_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_PHP_ARCHIVE_FILENAME)"
	@echo "Applying $(SOURCE_DIR)/tests/external/php/php-8.4.2.patch..."
	@patch -l "$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/ext/pcre/pcre2lib/sljit/sljitNativeX86_common.c" < "$(SOURCE_DIR)/tests/external/php/php-8.4.2.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring php $(KEFIR_EXTERNAL_TEST_PHP_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g" \
		LC_ALL=C.UTF-8 \
		./configure --disable-opcache

$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/sapi/cli/php: $(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/Makefile
	@echo "Building php $(KEFIR_EXTERNAL_TEST_PHP_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_PHP_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)/sapi/cli/php
	@echo "Testing php $(KEFIR_EXTERNAL_TEST_PHP_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_PHP_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		SKIP_IO_CAPTURE_TESTS=1 \
		bash -c "ulimit -s unlimited && $(MAKE) test NO_INTERACTION=1 2>&1" | tee "$(shell realpath "$@.tmp")"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/php.test.done: $(KEFIR_EXTERNAL_TEST_PHP_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/php/validate.sh "$(KEFIR_EXTERNAL_TEST_PHP_DIR)/tests.log"
	@touch "$@"
	@echo "Php $(KEFIR_EXTERNAL_TEST_PHP_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/php.test.done
