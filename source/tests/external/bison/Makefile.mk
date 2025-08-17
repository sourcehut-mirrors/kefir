
KEFIR_EXTERNAL_TEST_BISON_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/bison

KEFIR_EXTERNAL_TEST_BISON_VERSION := 3.8.2
KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_FILENAME := bison-$(KEFIR_EXTERNAL_TEST_BISON_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_BISON_ARCHIVE := $(KEFIR_EXTERNAL_TEST_BISON_DIR)/$(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_BISON_DIR)/bison-$(KEFIR_EXTERNAL_TEST_BISON_VERSION)
KEFIR_EXTERNAL_TEST_BISON_URL := https://www.nic.funet.fi/pub/gnu/ftp.gnu.org/pub/gnu/bison/$(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_SHA256 := 9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2

$(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_BISON_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_BISON_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BISON_DIR)" && tar xvf "$(KEFIR_EXTERNAL_TEST_BISON_ARCHIVE_FILENAME)"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/.extracted
	@echo "Configuring GNU Bison $(KEFIR_EXTERNAL_TEST_BISON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/bison/include" \
		LC_ALL=C.UTF-8 \
		./configure

$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/src/bison: $(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/Makefile
	@echo "Building GNU Bison $(KEFIR_EXTERNAL_TEST_BISON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/bison/include" \
		LC_ALL=C.UTF-8 \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_BISON_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)/src/bison
	@echo "Testing GNU Bison $(KEFIR_EXTERNAL_TEST_BISON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_BISON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-isystem $(realpath $(SOURCE_DIR))/tests/external/bison/include" \
		LC_ALL=C.UTF-8 \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/bison.test.done: $(KEFIR_EXTERNAL_TEST_BISON_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/bison/validate.sh "$(KEFIR_EXTERNAL_TEST_BISON_DIR)/tests.log"
	@touch "$@"
	@echo "GNU Bison $(KEFIR_EXTERNAL_TEST_BISON_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/bison.test.done
