
KEFIR_EXTERNAL_TEST_OPENSSH_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/openssh

KEFIR_EXTERNAL_TEST_OPENSSH_VERSION := 9.9p1
KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME := openssh-$(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE := $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/$(KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_OPENSSH_DIR)/openssh-$(KEFIR_EXTERNAL_TEST_OPENSSH_VERSION)
KEFIR_EXTERNAL_TEST_OPENSSH_URL := https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-9.9p1.tar.gz

KEFIR_EXTERNAL_TEST_OPENSSH_ARCHIVE_SHA256 := b343fbcdbff87f15b1986e6e15d6d4fc9a7d36066be6b7fb507087ba8f966c02

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
# These tests fail when CWD is long for some reason
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/channel-timeout.sh" < "$(SOURCE_DIR)/tests/external/openssh/channel-timeout.patch"
	@patch -l "$(KEFIR_EXTERNAL_TEST_OPENSSH_SOURCE_DIR)/regress/penalty-expire.sh" < "$(SOURCE_DIR)/tests/external/openssh/penalty-expire.patch"
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
		$(MAKE) $(patsubst %.test.log,%,$(notdir $@)) | tee "../$(notdir $@).tmp"
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
