
KEFIR_EXTERNAL_TEST_RUBY_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/ruby

KEFIR_EXTERNAL_TEST_RUBY_VERSION := 3.4.5
KEFIR_EXTERNAL_TEST_RUBY_VERSION_UNDERSCORE := $(subst .,_,$(KEFIR_EXTERNAL_TEST_RUBY_VERSION))
KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME := v$(KEFIR_EXTERNAL_TEST_RUBY_VERSION_UNDERSCORE).tar.gz
KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE := $(KEFIR_EXTERNAL_TEST_RUBY_DIR)/$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_RUBY_DIR)/ruby-$(KEFIR_EXTERNAL_TEST_RUBY_VERSION_UNDERSCORE)
KEFIR_EXTERNAL_TEST_RUBY_URL := https://github.com/ruby/ruby/archive/refs/tags/$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_SHA256 := 0c0680e4bce7a709208009506e286fd387cd16f290f00606133f735943ab75d5

$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_RUBY_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_RUBY_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME)"
	@echo "Pathcing Ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION)..."
	@cd $(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR) && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/ruby/ruby.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/ruby/include/kefir_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		sh autogen.sh

$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/configure
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/ruby/include/kefir_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		./configure --disable-install-rdoc

$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/ruby: $(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/Makefile
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/ruby/include/kefir_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_RUBY_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)/ruby
	@echo "Testing ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-include $(realpath $(SOURCE_DIR))/tests/external/ruby/include/kefir_builtins.h -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/ruby.test.done: $(KEFIR_EXTERNAL_TEST_RUBY_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/ruby/validate.sh "$(KEFIR_EXTERNAL_TEST_RUBY_DIR)/tests.log"
	@touch "$@"
	@echo "Ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/ruby.test.done
