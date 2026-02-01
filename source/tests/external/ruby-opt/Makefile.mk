
KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/ruby-opt
KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/ruby-$(KEFIR_EXTERNAL_TEST_RUBY_VERSION_UNDERSCORE)

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME)..."
	@mkdir -p "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)"
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)" && tar xvfz "$(shell realpath $(KEFIR_EXTERNAL_TEST_RUBY_DIR)/$(KEFIR_EXTERNAL_TEST_RUBY_ARCHIVE_FILENAME))"
	@echo "Pathcing Ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) -O1..."
	@cd $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR) && patch -p0 < "$(realpath $(SOURCE_DIR))/tests/external/ruby/ruby.patch"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		sh autogen.sh

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/configure
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		./configure --disable-install-rdoc

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/ruby: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/Makefile
	@echo "Building ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) -O1..."
	@cd "$(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		LC_ALL=C.UTF-8 \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -g -isystem $(realpath $(SOURCE_DIR))/tests/external/ruby/include" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_RUBY_OPT_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_RUBY_OPT_SOURCE_DIR)/ruby
	@echo "Testing ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) -O1..."
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
	@echo "Ruby $(KEFIR_EXTERNAL_TEST_RUBY_VERSION) test suite successfully finished"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/ruby-opt.test.done
