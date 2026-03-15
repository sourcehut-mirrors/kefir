
KEFIR_EXTERNAL_TEST_LIBRESSL_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/libressl

KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION := 4.2.1
KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_FILENAME := libressl-$(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE := $(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)/$(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)/libressl-$(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION)
KEFIR_EXTERNAL_TEST_LIBRESSL_URL := https://github.com/libressl/portable/releases/download/v$(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION)/$(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_SHA256 := 6d5c2f58583588ea791f4c8645004071d00dfa554a5bf788a006ca1eb5abd70b

$(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_LIBRESSL_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBRESSL_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_LIBRESSL_ARCHIVE_FILENAME)"
	@cd "$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/libressl/libressl.patch"
	@find $(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR) -type f -name "*.h" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@find $(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR) -type f -name "*.c" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@sed -i \
		-e '0,/^[ ]*lt_prog_compiler_wl=$$/s//lt_prog_compiler_wl=-Wl,/' \
		-e '0,/^[ ]*lt_prog_compiler_pic=$$/s//lt_prog_compiler_pic=-fPIC/' \
		-e '0,/^[ ]*lt_prog_compiler_static=$$/s//lt_prog_compiler_static=-static/' \
		"$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/configure"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring libressl $(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./configure

$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/ssl/.libs/libssl.so: $(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/Makefile
	@echo "Building libressl $(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)/ssl/.libs/libssl.so
	@echo "Testing libressl $(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBRESSL_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/libressl.test.done: $(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/libressl/validate.sh "$(KEFIR_EXTERNAL_TEST_LIBRESSL_DIR)/tests.log"
	@touch "$@"
	@echo "LibreSSL $(KEFIR_EXTERNAL_TEST_LIBRESSL_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/libressl.test.done
