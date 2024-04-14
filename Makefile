include Makefile.mk
include util/Makefile.mk
include source/Makefile.mk
include source/tests/Makefile.mk
include source/cc1/Makefile.mk
include source/driver/Makefile.mk
include source/web/Makefile.mk
include docs/man/Makefile.mk
include install.mk
include self.mk

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(wildcard $(DEPENDENCIES))
endif

.COMPILE_DEPS: $(COMPILE_DEPS)
.DEPENDENCIES: $(DEPENDENCIES)
.TEST_ARTIFACTS: $(TEST_ARTIFACTS)
.ASM_FILES: $(ASM_FILES)
.OBJECT_FILES: $(OBJECT_FILES)
.BINARIES: $(BINARIES)
.TEST_BINARIES: $(TEST_BINARIES)
.TEST_RESULTS: $(TEST_RESULTS)
.TESTS: $(TESTS)
.EXTERNAL_TESTS_BASE_SUITE: $(EXTERNAL_TESTS_BASE_SUITE)
.EXTERNAL_TESTS_FAST_SUITE: $(EXTERNAL_TESTS_FAST_SUITE)
.EXTERNAL_TESTS_SLOW_SUITE: $(EXTERNAL_TESTS_SLOW_SUITE)
.BOOTSTRAP: $(BOOTSTRAP)
.WEB: $(WEB)
.WEBAPP: $(WEBAPP)
.MAN_PAGES: $(MAN_PAGES)

all: .BINARIES .MAN_PAGES

generate_test_artifacts: .TEST_ARTIFACTS

test: .TESTS
	@echo "Tests succeeded"

bootstrap: .BOOTSTRAP
	@echo "Bootstrap successfully finished"

web: .WEB

webapp: .WEBAPP

clean:
	@echo "Removing $(KEFIR_BIN_DIR)"
	@rm -rf $(KEFIR_BIN_DIR)

clean_bootstrap:
	@echo "Removing $(BOOTSTRAP_DIR)"
	@rm -rf $(BOOTSTRAP_DIR)

help:
	@cat $(DOCS_DIR)/Makefile.help.txt

.PHONY: all test generate_test_artifacts bootstrap web webapp clean clean_bootstrap help \
        .DEPENDENCIES .COMPILE_DEPS .TEST_ARTIFACTS .ASM_FILES .OBJECT_FILES .BINARIES .TEST_BINARIES .TEST_RESULTS .TESTS .EXTERNAL_TESTS_BASE_SUITE .EXTERNAL_TESTS_FAST_SUITE .EXTERNAL_TESTS_SLOW_SUITE .BOOTSTRAP .MAN_PAGES
