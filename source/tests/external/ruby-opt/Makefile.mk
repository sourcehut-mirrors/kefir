
KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/ruby-opt

KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION := 4.0.1
KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_FILENAME := v$(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE := $(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/$(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/ruby-$(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION)
KEFIR_EXTERNAL_TEST_RUBY_OPT_URL := https://github.com/ruby/ruby/archive/refs/tags/$(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_SHA256 := 52429b111bd0a1ff1e4f94aa971543ff8d0f0a03766a044238c8246470aa0813

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_RUBY_OPT_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_ARCHIVE_FILENAME)"
	@echo "Pathcing Ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) -O1..."
	@cd $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR) && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/ruby/ruby.patch"
	@rm $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/tool/test/test_commit_email.rb
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		sh autogen.sh

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/configure
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		./configure --disable-install-rdoc

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/ruby: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/Makefile
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/ruby
	@echo "Testing ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		bash -c 'set -o pipefail; $(MAKE) check 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/ruby-opt.test.done: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/ruby/validate.sh "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/tests.log"
	@touch "$@"
	@echo "Ruby $(KEFIR_EXTERNAL_TEST_RUBY_OPT_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/ruby-opt.test.done
