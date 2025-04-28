include Makefile.mk
include util/Makefile.mk
include source/Makefile.mk
include source/tests/Makefile.mk
include source/cc1/Makefile.mk
include source/driver/Makefile.mk
include source/web/Makefile.mk
include docs/man/Makefile.mk
include install.mk

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
.CSMITH_TESTS: $(CSMITH_TESTS)
.EXTERNAL_TESTS_BASE_SUITE: $(EXTERNAL_TESTS_BASE_SUITE)
.EXTERNAL_TESTS_FAST_SUITE: $(EXTERNAL_TESTS_FAST_SUITE)
.EXTERNAL_TESTS_SLOW_SUITE: $(EXTERNAL_TESTS_SLOW_SUITE)
.EXTERNAL_TESTS_EXTRA_SUITE: $(EXTERNAL_TESTS_EXTRA_SUITE)
.EXTERNAL_TESTS_SUITE: .EXTERNAL_TESTS_BASE_SUITE .EXTERNAL_TESTS_FAST_SUITE .EXTERNAL_TESTS_SLOW_SUITE
.COMPILE_COMMANDS_JSON: $(COMPILE_COMMANDS_JSON)
.BOOTSTRAP_TEST: $(BOOTSTRAP_TEST)
.WEB: $(WEB)
.WEBAPP: $(WEBAPP)
.MAN_PAGES: $(MAN_PAGES)

all: .BINARIES .MAN_PAGES

generate_test_artifacts: .TEST_ARTIFACTS

test: .TESTS
	@echo "Tests succeeded"

csmith_test: .CSMITH_TESTS
	@echo "CSmith tests succeeded"

torture_test: $(KEFIR_GCC_TORTURE_TEST)

external_test: .EXTERNAL_TESTS_SUITE

external_extra_test: .EXTERNAL_TESTS_EXTRA_SUITE

bootstrap_test: .BOOTSTRAP_TEST
	@echo "Bootstrap successfully finished"

portable:
	@$(MAKE) -f dist/portable/Makefile BOOTSTRAP=no all

portable_bootstrap:
	@$(MAKE) -f dist/portable/Makefile BOOTSTRAP=yes all

web: .WEB

webapp: .WEBAPP

coverage: $(COVERAGE_HTML)

clean:
	@echo "Removing $(KEFIR_BIN_DIR)"
	@rm -rf $(KEFIR_BIN_DIR)

help:
	@cat $(DOCS_DIR)/Makefile.help.txt

compile_commands: $(COMPILE_COMMANDS_JSON)

.NOTPARALLEL: .EXTERNAL_TESTS_SUITE .EXTERNAL_TESTS_BASE_SUITE .EXTERNAL_TESTS_FAST_SUITE .EXTERNAL_TESTS_SLOW_SUITE .EXTERNAL_TESTS_EXTRA_SUITE $(EXTERNAL_TESTS_BASE_SUITE) $(EXTERNAL_TESTS_FAST_SUITE) $(EXTERNAL_TESTS_SLOW_SUITE) $(EXTERNAL_TESTS_EXTRA_SUITE)

.PHONY: all test generate_test_artifacts bootstrap_test web webapp coverage clean help torture_test csmith_test external_test external_extra_test portable portable_bootstrap compile_commands \
        .DEPENDENCIES .COMPILE_DEPS .TEST_ARTIFACTS .ASM_FILES .OBJECT_FILES .BINARIES .TEST_BINARIES .TEST_RESULTS .TESTS .EXTERNAL_TESTS_BASE_SUITE .EXTERNAL_TESTS_FAST_SUITE .EXTERNAL_TESTS_SLOW_SUITE .EXTERNAL_TESTS_EXTRA_SUITE .EXTERNAL_TESTS_SUITE .BOOTSTRAP .MAN_PAGES

.DEFAULT_GOAL := all