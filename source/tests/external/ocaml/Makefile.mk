KEFIR_EXTERNAL_TEST_OCAML_DIR := $(KEFIR_EXTERNAL_TESTS_DIR)/ocaml

KEFIR_EXTERNAL_TEST_OCAML_VERSION := 5.4.1
KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_FILENAME := ocaml-$(KEFIR_EXTERNAL_TEST_OCAML_VERSION).tar.gz
KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE := $(KEFIR_EXTERNAL_TEST_OCAML_DIR)/$(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_FILENAME)
KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR := $(KEFIR_EXTERNAL_TEST_OCAML_DIR)/ocaml-$(KEFIR_EXTERNAL_TEST_OCAML_VERSION)
KEFIR_EXTERNAL_TEST_OCAML_URL := https://github.com/ocaml/ocaml/releases/download/$(KEFIR_EXTERNAL_TEST_OCAML_VERSION)/$(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_FILENAME)

KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_SHA256 := d4528517aaa1a44b8e2b1bc109a1ed0a5e0014f3ddc4feb8906b11a7e063e89a

$(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading $(KEFIR_EXTERNAL_TEST_OCAML_URL)"
	@$(WGET) -O "$@.tmp" "$(KEFIR_EXTERNAL_TEST_OCAML_URL)"
	@$(SCRIPTS_DIR)/checksum_sha256.sh "$@.tmp" "$(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_SHA256)"
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/.extracted: $(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE)
	@echo "Extracting $(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_FILENAME)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OCAML_DIR)" && tar xvfz "$(KEFIR_EXTERNAL_TEST_OCAML_ARCHIVE_FILENAME)"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-bytecode"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-domains"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-init-info"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-initializers"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-packed"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-pr4229"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-pr4839"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-pr6950"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-pr9209"
	@rm -rf "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/testsuite/tests/lib-dynlink-private"
	@touch "$@"

$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/config.log: $(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/.extracted $(KEFIR_EXE)
	@echo "Configuring ocaml $(KEFIR_EXTERNAL_TEST_OCAML_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		./configure CC="$(realpath $(KEFIR_EXE))" --enable-ocamltest --disable-ocamldoc --disable-debugger --disable-native-compiler

$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/ocaml: $(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/config.log
	@echo "Building ocaml $(KEFIR_EXTERNAL_TEST_OCAML_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		$(MAKE)

$(KEFIR_EXTERNAL_TEST_OCAML_DIR)/tests.log: $(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)/ocaml
	@echo "Testing ocaml $(KEFIR_EXTERNAL_TEST_OCAML_VERSION)..."
	@cd "$(KEFIR_EXTERNAL_TEST_OCAML_SOURCE_DIR)" && \
		LD_LIBRARY_PATH="$(realpath $(LIB_DIR))$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))" \
		KEFIR_RTINC="$(realpath $(HEADERS_DIR))/kefir/runtime" \
		CC="$(realpath $(KEFIR_EXE))" \
		KEFIR_DRIVER_CLI_QUIET=yes \
		bash -c 'set -o pipefail; $(MAKE) tests 2>&1 | tee "$(shell realpath $@.tmp)"'
	@mv "$@.tmp" "$@"

$(KEFIR_EXTERNAL_TESTS_DIR)/ocaml.test.done: $(KEFIR_EXTERNAL_TEST_OCAML_DIR)/tests.log
	@$(SOURCE_DIR)/tests/external/ocaml/validate.sh "$(KEFIR_EXTERNAL_TEST_OCAML_DIR)/tests.log"
	@touch "$@"
	@echo "Successfully tested ocaml $(KEFIR_EXTERNAL_TEST_OCAML_VERSION)"

EXTERNAL_TESTS_SLOW_SUITE += $(KEFIR_EXTERNAL_TESTS_DIR)/ocaml.test.done
