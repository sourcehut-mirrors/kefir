KEFIR_BIN_LIBGCC_474_DIR=$(KEFIR_BIN_DIR)/libgcc-474

$(KEFIR_BIN_LIBGCC_474_DIR)/libgcc.a: $(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc
	@mkdir -p "$(shell dirname $@)"
	@echo "Copying $@"
	@cp "$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/lib/gcc/x86_64-unknown-linux-gnu/4.7.4/libgcc.a" "$@"

$(KEFIR_BIN_LIBGCC_474_DIR)/libgcc_eh.a: $(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/bin/gcc
	@mkdir -p "$(shell dirname $@)"
	@echo "Copying $@"
	@cp "$(KEFIR_EXTERNAL_TEST_GCC_474_INSTALL_DIR)/lib/gcc/x86_64-unknown-linux-gnu/4.7.4/libgcc_eh.a" "$@"

BOOTSTRAP_LIBGCC_474 += $(KEFIR_BIN_LIBGCC_474_DIR)/libgcc.a
BOOTSTRAP_LIBGCC_474 += $(KEFIR_BIN_LIBGCC_474_DIR)/libgcc_eh.a
