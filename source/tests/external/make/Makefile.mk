
KEFIR_EXTERNAL_TEST_MAKE_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/make

KEFIR_EXTERNAL_TEST_MAKE_VERSION := 4.4.1
KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_FILENAME := make-$(KEFIR_EXTERNAL_TEST_MAKE_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE := $(KEFIR_EXTERNAL_TEST_MAKE_DIR)/make-$(KEFIR_EXTERNAL_TEST_MAKE_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_MAKE_DIR)/make-$(KEFIR_EXTERNAL_TEST_MAKE_VERSION)
KEFIR_EXTERNAL_TEST_MAKE_URL := https://ftp.gnu.org/gnu/make/$(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_SHA256 := dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3

$(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_MAKE_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_MAKE_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_MAKE_DIR)/kefir-wrapper: $(KEFIR_EXE)
	@mkdir -p $(dir $@)
	@echo 'export LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH"' > "$@.tmp"
	@echo 'export KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime"' >> "$@.tmp"
	@echo 'exec $(realpath $(KEFIR_EXE)) $$@' >> "$@.tmp"
	@chmod +x "$@.tmp"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MAKE_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_MAKE_ARCHIVE_FILENAME)"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/.extracted $(KEFIR_EXTERNAL_TEST_MAKE_DIR)/kefir-wrapper
	@echo "Configuring GNU Make $(KEFIR_EXTERNAL_TEST_MAKE_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)" && \
		CC="$(realpath $(KEFIR_EXTERNAL_TEST_MAKE_DIR))/kefir-wrapper" \
		./configure

$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/make: $(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/Makefile
	@echo "Building GNU Make $(KEFIR_EXTERNAL_TEST_MAKE_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)" && \
		$(MAKE) all

$(KEFIR_EXTERNAL_TEST_MAKE_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/make
	@echo "Testing GNU Make $(KEFIR_EXTERNAL_TEST_MAKE_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/tests" && \
		./run_make_tests -make_path "$(realpath $(KEFIR_EXTERNAL_TEST_MAKE_SOURCE_DIR)/make)" | tee "$(shell realpath $@.tmp)"
	@$(SOURCE_DIR)/tests/external/make/validate.sh "$@.tmp"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/make.test.done: $(KEFIR_EXTERNAL_TEST_MAKE_DIR)/tests.log
	@touch "$@"
	@echo "GNU Make $(KEFIR_EXTERNAL_TEST_MAKE_VERSION) validation successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/make.test.done
