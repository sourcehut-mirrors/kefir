
KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/emacs-opt

KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION := 30.2
KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_FILENAME := emacs-(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE := $(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)/$(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)/emacs-$(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION)
KEFIR_EXTERNAL_TEST_EMACS_OPT_URL := https://gnuftp.uib.no/emacs/emacs-$(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION).tar.gz

KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_SHA256 := 1d79a4ba4d6596f302a7146843fe59cf5caec798190bcc07c907e7ba244b076d

$(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_EMACS_OPT_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_ARCHIVE_FILENAME)"
# Tests below fail with gcc too in kefir testing environment
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/progmodes/flymake-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/progmodes/eglot-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/progmodes/project-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/uniquify-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/proced-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/net/shr-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/gnus/mml-sec-tests.el"
	@rm -f "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/test/lisp/epg-tests.el"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Building emacs-opt $(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -fPIC -g" \
		./configure --with-x-toolkit=no --with-xpm=ifavailable --with-jpeg=ifavailable --with-png=ifavailable --with-gif=ifavailable --with-tiff=ifavailable --with-gnutls=ifavailable

$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/src/emacs-opt: $(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/Makefile
	@echo "Building emacs-opt $(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		CFLAGS="-O1 -fPIC -g" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)/src/emacs-opt
	@echo "Testing emacs-opt $(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) check -j1 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/emacs-opt.test.done: $(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/emacs-opt/validate.sh "$(KEFIR_EXTERNAL_TEST_EMACS_OPT_DIR)/tests.log"
	@touch "$@"
	@echo "GNU Emacs optimized $(KEFIR_EXTERNAL_TEST_EMACS_OPT_VERSION) test successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/emacs-opt.test.done
