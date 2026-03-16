KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/lighttpd

KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION := 1.4.82
KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE := lighttpd-$(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/lighttpd-$(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)
KEFIR_EXTERNAL_TEST_LIGHTTPD_URL := https://download.lighttpd.net/lighttpd/releases-1.4.x/$(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE)

KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE_SHA256=4f07f2d61ee8d136d105d9a62f139a46ad8216fe9e346476ee5340f87bcabd79

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/$(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE):
	@mkdir -p $(shell dirname $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_LIGHTTPD_URL)..."
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/$(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_ARCHIVE)"
	@find $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR) -type f -name "*.h" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@find $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR) -type f -name "*.c" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring LightHTTPD $(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(shell $(REALPATH) $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(shell $(REALPATH) $(HEADERS_DIR)/kefir/runtime)" \
		CC="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CC_FOR_BUILD="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/lighttpd/include" \
		sh autogen.sh
	@sed -i \
		-e '0,/^[ ]*lt_prog_compiler_wl=$$/s//lt_prog_compiler_wl=-Wl,/' \
		-e '0,/^[ ]*lt_prog_compiler_pic=$$/s//lt_prog_compiler_pic=-fPIC/' \
		-e '0,/^[ ]*lt_prog_compiler_static=$$/s//lt_prog_compiler_static=-static/' \
		"$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/configure"

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/configure
	@echo "Configuring LightHTTPD $(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(shell $(REALPATH) $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(shell $(REALPATH) $(HEADERS_DIR)/kefir/runtime)" \
		CC="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CC_FOR_BUILD="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/lighttpd/include" \
		./configure
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/lighttpd: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/Makefile
	@echo "Building LightHTTPD $(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(shell $(REALPATH) $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(shell $(REALPATH) $(HEADERS_DIR)/kefir/runtime)" \
		CC="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CC_FOR_BUILD="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/lighttpd/include" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)/lighttpd
	@echo "Testing LightHTTPD $(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CC_FOR_BUILD="$(shell $(REALPATH) $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/lighttpd/include" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/lighttpd.test.done: $(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/tests.log
	@"$(SOURCE_DIR)/tests/external/lighttpd/validate.sh" "$(KEFIR_EXTERNAL_TEST_LIGHTTPD_DIR)/tests.log"
	@touch "$@"
	@echo "Successfully tested LightHTTPD $(KEFIR_EXTERNAL_TEST_LIGHTTPD_VERSION)"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/lighttpd.test.done
