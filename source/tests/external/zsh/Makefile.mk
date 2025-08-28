
KEFIR_EXTERNAL_TEST_ZSH_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/zsh

KEFIR_EXTERNAL_TEST_ZSH_VERSION := 5.9
KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME := zsh-$(KEFIR_EXTERNAL_TEST_ZSH_VERSION).tar.xz
KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE := $(KEFIR_EXTERNAL_TEST_ZSH_DIR)/$(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_ZSH_DIR)/zsh-$(KEFIR_EXTERNAL_TEST_ZSH_VERSION)
KEFIR_EXTERNAL_TEST_ZSH_URL := https://ftp.funet.fi/pub/unix/shells/zsh/$(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_SHA256 := 9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5

$(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_ZSH_URL)"
	@wget -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_ZSH_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZSH_DIR)" && tar xvf "$(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME)"
	@echo "Patching $(KEFIR_EXTERNAL_TEST_ZSH_ARCHIVE_FILENAME)..."
	@find "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/Test" -type f -exec sed -i 's/\begrep\b/grep -E/g' {} \;
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/configure: $(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Libtoolize zsh $(KEFIR_EXTERNAL_TEST_ZSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		autoreconf -fi
	@sed -i 's|.*ERROR MACROS NOT FOUND.*|ERRNO_H=/usr/include/errno.h|g' "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/configure"

$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/Makefile: $(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/configure
	@echo "Libtoolize zsh $(KEFIR_EXTERNAL_TEST_ZSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./configure

$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/Src/zsh: $(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/Makefile
	@echo "Libtoolize zsh $(KEFIR_EXTERNAL_TEST_ZSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_ZSH_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)/Src/zsh
	@echo "Testing zsh $(KEFIR_EXTERNAL_TEST_ZSH_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_ZSH_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR)):$$LD_LIBRARY_PATH" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		bash -c 'set -o pipefail; $(MAKE) test 2>&1 | tee "$(shell realpath "$@.tmp")"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/zsh.test.done: $(KEFIR_EXTERNAL_TEST_ZSH_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/zsh/validate.sh "$(KEFIR_EXTERNAL_TEST_ZSH_DIR)/tests.log"
	@touch "$@"
	@echo "zsh $(KEFIR_EXTERNAL_TEST_ZSH_VERSION) test successfully finished"

EXTERNAL_TESTS_FAST_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/zsh.test.done
