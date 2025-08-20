
KEFIR_EXTERNAL_TEST_OPENSSH_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/openssh

KEFIR_EXTERNAL_TEST_OPENSSH_VERSION := 10.0p1
KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME := openssh-$(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE := $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/openssh-$(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION)
KEFIR_EXTERNAL_TEST_OPENSSH_URL := https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_SHA256 := 021a2e709a0edf4250b1256bd5a9e500411a90dddabea830ed59cef90eb9d85c

$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_OPENSSH_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_OPENSSH_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME)"
	@echo "Patching OpenSSH $(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION) test suite..."
# These tests fail when CWD is long: "path ... too long for Unix domain socket"
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/channel-timeout.sh" < "$(SOURCE_DIR)/tests/external/openssh/channel-timeout.patch"
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/penalty-expire.sh" < "$(SOURCE_DIR)/tests/external/openssh/penalty-expire.patch"
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/forward-control.sh" < "$(SOURCE_DIR)/tests/external/openssh/forward-control.patch"
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/connection-timeout.sh" < "$(SOURCE_DIR)/tests/external/openssh/connection-timeout.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring openssh $(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-fPIC -O1 -g" \
		LC_ALL=C.UTF-8 \
		./configure

$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/ssh: $(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/Makefile
	@echo "Building openssh $(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/%.test.log: $(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/ssh
	@echo "Testing openssh $(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION) $(notdir $@)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		LC_ALL=C.UTF-8 \
		bash -c 'set -o pipefail; $(MAKE) $(patsubst %.test.log,%,$(notdir $@)) | tee "../$(notdir $@).tmp"'
	@mv "$@.tmp" "$@"

.NOTPARALLEL: $(KEFIR_EXTERNAL_TESTS_DIR)/openssh.test.done
$(KEFIR_EXTERNAL_TESTS_DIR)/openssh.test.done: $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/file-tests.test.log \
											   $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/t-exec.test.log \
											   $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/interop-tests.test.log \
											   $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/extra-tests.test.log \
											   $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/unit.test.log
	@$(SOURCE_DIR)/tests/external/openssh/validate.sh $^
	@touch "$@"
	@echo "OpenSSH $(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/openssh.test.done
