
KEFIR_EXTERNAL_TEST_CSP_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/csp

KEFIR_EXTERNAL_TEST_CSP_VERSION := 577888382eff018cd7746becab8dcc00a3e2760a
KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_FILENAME := $(KEFIR_EXTERNAL_TEST_CSP_VERSION).zip
KEFIR_EXTERNAL_TEST_CSP_ARCHIVE := $(KEFIR_EXTERNAL_TEST_CSP_DIR)/$(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_CSP_DIR)/csp-$(KEFIR_EXTERNAL_TEST_CSP_VERSION)
KEFIR_EXTERNAL_TEST_CSP_URL := https://github.com/josugoar/csp/archive/$(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_SHA256 := 6ec30a82b669677c610dfb16e7cf78e064cf886dbde3841847a8e85d8a45f7f1

$(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_CSP_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_CSP_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_CSP_DIR)" && unzip -o "$(KEFIR_EXTERNAL_TEST_CSP_ARCHIVE_FILENAME)"
	@echo "Patching csp $(KEFIR_EXTERNAL_TEST_CSP_VERSION)"
	@cd "$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/csp/csp.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/libcsp.so: $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building $@..."
	@mkdir -p "$(shell dirname $@)"
	@cd "$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		"$(realpath $(KEFIR_EXE))" -std=c23 -shared -fPIC -O1 -g -I ./include ./src/*.c -o build/libcsp.so

$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr/%: $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/libcsp.so
	@echo "Building $@..."
	@mkdir -p "$(shell dirname $@)"
	@cd "$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		"$(realpath $(KEFIR_EXE))" -std=c23 -fPIC -O1 -g -I ./include ./test/unique_ptr/$(shell basename $@).c -o build/unique_ptr/$(shell basename $@) -Lbuild -lcsp

$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr-test/%: $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr/%
	@echo "Testing $<..."
	@mkdir -p "$(shell dirname $@)"
	@LD_LIBRARY_PATH="$(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build" $<
	@touch "$@"
		
$(KEFIR_EXTERNAL_TESTS_DIR)/csp.test.done: $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr-test/cmp \
                                           $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr-test/swap
	@touch "$@"
	@echo "Successfully tested csp $(KEFIR_EXTERNAL_TEST_CSP_VERSION)"

TEST_BINARIES += $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr/cmp
TEST_BINARIES += $(KEFIR_EXTERNAL_TEST_CSP_SOURCE_DIR)/build/unique_ptr/swap
EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/csp.test.done
