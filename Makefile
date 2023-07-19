include Makefile.mk
include source/Makefile.mk
include source/tests/Makefile.mk
include source/cc1/Makefile.mk
include source/driver/Makefile.mk
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

all: .BINARIES

generate_test_artifacts: $(TEST_ARTIFACTS)

test: .TESTS
	@echo "Tests succeeded"

clean:
	@echo "Removing $(BIN_DIR)"
	@rm -rf $(BIN_DIR)

.PHONY: all test generate_test_artifacts generate clean .DEPENDENCIES .TEST_ARTIFACTS .ASM_FILES .OBJECT_FILES .BINARIES .TEST_BINARIES .TEST_RESULTS .TESTS
