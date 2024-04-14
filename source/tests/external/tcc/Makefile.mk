
KEFIR_EXTERNAL_TEST_TCC_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/tcc

KEFIR_EXTERNAL_TEST_TCC_VERSION := 0.9.27
KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME := tcc-$(KEFIR_EXTERNAL_TEST_TCC_VERSION).tar.bz
KEFIR_EXTERNAL_TEST_TCC_ARCHIVE := $(KEFIR_EXTERNAL_TEST_TCC_DIR)/$(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_TCC_DIR)/tcc-$(KEFIR_EXTERNAL_TEST_TCC_VERSION)
KEFIR_EXTERNAL_TEST_TCC_URL := https://download.savannah.gnu.org/releases/tinycc/tcc-$(KEFIR_EXTERNAL_TEST_TCC_VERSION).tar.bz2

KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_SHA256 := de23af78fca90ce32dff2dd45b3432b2334740bb9bb7b05bf60fdbfc396ceb9c

$(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_TCC_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_TCC_URL)"
	@$(SOURCE_DIR)/tests/external/util/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_TCC_DIR)" && \
		bunzip2 -d < "$(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME)" > "$(basename $(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME))"
	@cd "$(KEFIR_EXTERNAL_TEST_TCC_DIR)" && tar xvf "$(basename $(KEFIR_EXTERNAL_TEST_TCC_ARCHIVE_FILENAME))"
	@echo "Applying $(SOURCE_DIR)/tests/external/tcc/bcheck.patch"
	@patch "$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/lib/bcheck.c" < "$(SOURCE_DIR)/tests/external/tcc/bcheck.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/config.h: $(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/.extracted $(KEFIR_EXE) $(LIBKEFIRRT_A)
	@echo "Configuring tcc $(KEFIR_EXTERNAL_TEST_TCC_VERSION)..."
	@rm -f "$@"
	@cd "$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTCCC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		KEFIR_RTLIB="$(realpath $(LIBKEFIRRT_A))" \
		./configure --cc="$(realpath $(KEFIR_EXE))" --extra-cflags="-O1 -fPIC -pie"

$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/tcc: $(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/config.h
	@echo "Building tcc $(KEFIR_EXTERNAL_TEST_TCC_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTCCC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		KEFIR_RTLIB="$(realpath $(LIBKEFIRRT_A))" \
		$(MAKE) all

$(KEFIR_EXTERNAL_TEST_TCC_DIR)/test.log: $(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/tcc
	@echo "Validating tcc $(KEFIR_EXTERNAL_TEST_TCC_VERSION)..."
	@"$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)/tcc" \
		-B "$(KEFIR_EXTERNAL_TEST_TCC_SOURCE_DIR)" \
		-run "$(SOURCE_DIR)/tests/external/tcc/test.c" "test case" | tee "$@.tmp"
	@diff "$@.tmp" "$(SOURCE_DIR)/tests/external/tcc/test.log.expected"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/tcc.test.done: $(KEFIR_EXTERNAL_TEST_TCC_DIR)/test.log
	@touch "$@"
	@echo "Successfully validated tcc $(KEFIR_EXTERNAL_TEST_TCC_VERSION)"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/tcc.test.done
