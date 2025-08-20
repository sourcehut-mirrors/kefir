
KEFIR_EXTERNAL_TEST_MUON_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/muon

KEFIR_EXTERNAL_TEST_MUON_VERSION := 0.5.0
KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_FILENAME := muon-v$(KEFIR_EXTERNAL_TEST_MUON_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_MUON_ARCHIVE := $(KEFIR_EXTERNAL_TEST_MUON_DIR)/$(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_MUON_DIR)/muon-v$(KEFIR_EXTERNAL_TEST_MUON_VERSION)
KEFIR_EXTERNAL_TEST_MUON_URL := https://muon.build/releases/v$(KEFIR_EXTERNAL_TEST_MUON_VERSION)/$(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_SHA256 := 24aa4d29ed272893f6e6d355b1ec4ef20647438454e88161bdb9defd7c6faf77

$(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_MUON_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_MUON_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MUON_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_MUON_ARCHIVE_FILENAME)"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build/muon-bootstrap: $(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Bootstrapping muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./bootstrap.sh build

$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build2/build.ninja: $(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build/muon-bootstrap
	@echo "Setting muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION) up..."
	@cd "$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		build/muon-bootstrap setup build2

$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build2/muon: $(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build2/build.ninja
	@echo "Building muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION) up..."
	@cd "$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		build/muon-bootstrap -C build2 samu

$(KEFIR_EXTERNAL_TEST_MUON_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)/build2/muon
	@echo "Testing muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_MUON_SOURCE_DIR)" && \
		bash -c 'set -o pipefail; build2/muon -C build2 test 2>&1 | tee "$(shell realpath $@.tmp)"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/muon.test.done: $(KEFIR_EXTERNAL_TEST_MUON_DIR)/tests.log
	@echo "Validating muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION)..."
	@$(SOURCE_DIR)/tests/external/muon/validate.sh "$(KEFIR_EXTERNAL_TEST_MUON_DIR)/tests.log"
	@touch "$@"
	@echo "muon $(KEFIR_EXTERNAL_TEST_MUON_VERSION) test successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/muon.test.done
