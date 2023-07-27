include Makefile.mk
include source/Makefile.mk
include source/tests/Makefile.mk
include source/cc1/Makefile.mk
include source/driver/Makefile.mk
include source/emscripten/Makefile.mk
include install.mk
include self.mk

include $(wildcard $(DEPENDENCIES))

.DEPENDENCIES: $(DEPENDENCIES)
.TEST_ARTIFACTS: $(TEST_ARTIFACTS)
.ASM_FILES: $(ASM_FILES)
.OBJECT_FILES: $(OBJECT_FILES)
.BINARIES: $(BINARIES)
.TEST_BINARIES: $(TEST_BINARIES)
.TEST_RESULTS: $(TEST_RESULTS)
.TESTS: $(TESTS)
.BOOTSTRAP: $(BOOTSTRAP)
.WEB: $(WEB)

all: .BINARIES

generate_test_artifacts: .TEST_ARTIFACTS

test: .TESTS
	@echo "Tests succeeded"

bootstrap: .BOOTSTRAP
	@echo "Bootstrap successfully finished"

web: .WEB

clean:
	@echo "Removing $(BIN_DIR)"
	@rm -rf $(BIN_DIR)

clean_bootstrap:
	@echo "Removing $(BOOTSTRAP_DIR)"
	@rm -rf $(BOOTSTRAP_DIR)

.PHONY: all test generate_test_artifacts bootstrap web clean clean_bootstrap .DEPENDENCIES .TEST_ARTIFACTS .ASM_FILES .OBJECT_FILES .BINARIES .TEST_BINARIES .TEST_RESULTS .TESTS .BOOTSTRAP
