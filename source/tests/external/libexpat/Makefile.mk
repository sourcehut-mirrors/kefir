
KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/libexpat

KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION := 2.7.1
KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION_UNDERSCORE := $(subst .,_,$(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION))
KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_FILENAME := R_$(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION_UNDERSCORE).tar.gz
KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE := $(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)/$(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)/libexpat-R_$(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION_UNDERSCORE)
KEFIR_EXTERNAL_TEST_LIBEXPAT_URL := https://github.com/libexpat/libexpat/archive/refs/tags/$(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_SHA256 := 85372797ff0673a8fc4a6be16466bb5a0ca28c0dcf3c6f7ac1686b4a3ba2aabb

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_LIBEXPAT_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_ARCHIVE_FILENAME)"
	@echo "Pathcing libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION)..."
	@find $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR) -type f -name "*.h" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@find $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR) -type f -name "*.c" -exec sed -i 's/__attribute__/__attribute/g' {} \;
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/configure: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Buildconf libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION)..."
	@mkdir -p "$(shell dirname $@)"
	@cd "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./buildconf.sh

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/Makefile: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/configure $(KEFIR_EXE)
	@echo "Configure libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION)..."
	@mkdir -p "$(shell dirname $@)"
	@cd "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./configure

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/lib/libexpat.la: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/Makefile
	@echo "Building libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat/lib/libexpat.la
	@echo "Testing libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_SOURCE_DIR)/expat" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/libexpat.test.done: $(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)/tests.log
	@"$(SOURCE_DIR)/tests/external/libexpat/validate.sh" "$(KEFIR_EXTERNAL_TEST_LIBEXPAT_DIR)/tests.log"
	@touch "$@"
	@echo "libexpat $(KEFIR_EXTERNAL_TEST_LIBEXPAT_VERSION) test successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/libexpat.test.done
