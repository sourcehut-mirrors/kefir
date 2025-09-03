KEFIR_EXTERNAL_TEST_ZIG_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/zig

KEFIR_EXTERNAL_TEST_ZIG_VERSION := 35f013db11ff90ab9c75b028dc58b50d4f06ee42
KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_FILENAME := $(KEFIR_EXTERNAL_TEST_ZIG_VERSION).zip
KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE := $(KEFIR_EXTERNAL_TEST_ZIG_DIR)/$(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_ZIG_DIR)/zig-$(KEFIR_EXTERNAL_TEST_ZIG_VERSION)
KEFIR_EXTERNAL_TEST_ZIG_URL := https://github.com/ziglang/zig/archive/$(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_SHA256 := efc06b31c9e6228562e52affbca90eb304026474085adbf63b51a5634218580a

$(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_ZIG_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_ZIG_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_FILENAME)..."
	@rm -rf "$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)"
	@cd "$(KEFIR_EXTERNAL_TEST_ZIG_DIR)" && unzip -o "$(KEFIR_EXTERNAL_TEST_ZIG_ARCHIVE_FILENAME)"
	@echo "Patching zig $(KEFIR_EXTERNAL_TEST_ZIG_VERSION) bootstrap..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)" && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/zig-bootstrap/bootstrap.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/bootstrap: $(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building zig $(KEFIR_EXTERNAL_TEST_ZIG_VERSION) bootstrap..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		"$(realpath $(KEFIR_EXE))" -O2 -fPIC -o bootstrap bootstrap.c

$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/zig2: $(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/bootstrap
	@echo "Bootstrapping zig $(KEFIR_EXTERNAL_TEST_ZIG_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c "ulimit -s unlimited && ./bootstrap"

$(KEFIR_EXTERNAL_TEST_ZIG_DIR)/test.log: $(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/zig2
	@echo "Testing Zig $(KEFIR_EXTERNAL_TEST_ZIG_VERSION)..."
	@$(KEFIR_EXTERNAL_TEST_ZIG_SOURCE_DIR)/zig2 test --show-builtin | tee "$@.tmp"
	@sed -ri '/\.fxsr[,]?/d' "$@.tmp"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/zig-bootstrap.test.done: $(KEFIR_EXTERNAL_TEST_ZIG_DIR)/test.log
	@"$(SOURCE_DIR)/tests/external/zig-bootstrap/validate.sh" "$(KEFIR_EXTERNAL_TEST_ZIG_DIR)/test.log"
	@touch "$@"
	@echo "Successfully validated Zig $(KEFIR_EXTERNAL_TEST_ZIG_VERSION) bootstrap"

EXTERNAL_TESTS_EXTRA_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/zig-bootstrap.test.done
