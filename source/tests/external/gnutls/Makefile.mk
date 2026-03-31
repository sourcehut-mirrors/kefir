
KEFIR_EXTERNAL_TEST_GNUTLS_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/gnutls

KEFIR_EXTERNAL_TEST_GNUTLS_VERSION := 3.8.12
KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_FILENAME := gnutls-$(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE := $(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)/$(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)/gnutls-$(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION)
KEFIR_EXTERNAL_TEST_GNUTLS_URL := https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/$(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_SHA256 := a7b341421bfd459acf7a374ca4af3b9e06608dcd7bd792b2bf470bea012b8e51

$(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_GNUTLS_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GNUTLS_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)" && tar xvf "$(KEFIR_EXTERNAL_TEST_GNUTLS_ARCHIVE_FILENAME)"
	@sed -i \
		-e '0,/^[ ]*lt_prog_compiler_wl=$$/s//lt_prog_compiler_wl=-Wl,/' \
		-e '0,/^[ ]*lt_prog_compiler_pic=$$/s//lt_prog_compiler_pic=-fPIC/' \
		-e '0,/^[ ]*lt_prog_compiler_static=$$/s//lt_prog_compiler_static=-static/' \
		"$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/configure"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring gnutls $(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./configure

$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/src/.libs/gnutls-cli: $(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/Makefile
	@echo "Building gnutls $(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)/src/.libs/gnutls-cli
	@echo "Testing gnutls $(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GNUTLS_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/gnutls.test.done: $(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/gnutls/validate.sh "$(KEFIR_EXTERNAL_TEST_GNUTLS_DIR)/tests.log"
	@touch "$@"
	@echo "GnuTLS $(KEFIR_EXTERNAL_TEST_GNUTLS_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/gnutls.test.done
