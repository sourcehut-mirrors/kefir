KEFIR_EXTERNAL_TEST_GIT_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/git

KEFIR_EXTERNAL_TEST_GIT_VERSION := 2.51.0
KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_FILENAME := git-$(KEFIR_EXTERNAL_TEST_GIT_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_GIT_ARCHIVE := $(KEFIR_EXTERNAL_TEST_GIT_DIR)/$(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_GIT_DIR)/git-$(KEFIR_EXTERNAL_TEST_GIT_VERSION)
KEFIR_EXTERNAL_TEST_GIT_URL := https://www.kernel.org/pub/software/scm/git/git-$(KEFIR_EXTERNAL_TEST_GIT_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_SHA256 := 3d531799d2cf2cac8e294ec6e3229e07bfca60dc6c783fe69e7712738bef7283

$(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_GIT_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GIT_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GIT_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_GIT_ARCHIVE_FILENAME)"
	@echo "Patching git $(KEFIR_EXTERNAL_TEST_GIT_VERSION)..."
# "path ... too long for Unix domain socket" on deeply nested path in build environment
	@cd "$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/git/git.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/config.log: $(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring git $(KEFIR_EXTERNAL_TEST_GIT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -fPIC -pie" \
		./configure

$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/git: $(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/config.log
	@echo "Building git $(KEFIR_EXTERNAL_TEST_GIT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		$(MAKE) all

$(KEFIR_EXTERNAL_TEST_GIT_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)/git
	@echo "Testing Git $(KEFIR_EXTERNAL_TEST_GIT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_GIT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		bash -c 'set -o pipefail; $(MAKE) test 2>&1 | tee "$(shell realpath $@.tmp)"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/git.test.done: $(KEFIR_EXTERNAL_TEST_GIT_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/git/validate.sh "$(KEFIR_EXTERNAL_TEST_GIT_DIR)/tests.log"
	@touch "$@"
	@echo "Successfully tested Git $(KEFIR_EXTERNAL_TEST_GIT_VERSION)"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/git.test.done
