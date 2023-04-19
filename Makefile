include Makefile.mk
include source/Makefile.mk
include source/tests/Makefile.mk
include source/cc1/Makefile.mk
include source/driver/Makefile.mk
include install.mk
include self.mk

.DEPENDENCIES: $(DEPENDENCIES)
.ASM_FILES: $(ASM_FILES)
.OBJECT_FILES: $(OBJECT_FILES)
.BINARIES: $(BINARIES)
.TEST_BINARIES: $(TEST_BINARIES)
.TEST_RESULTS: $(TEST_RESULTS)
.TESTS: $(TESTS)

all: .BINARIES

test: .TESTS
	@echo "Tests succeeded"

clean:
	@echo "Removing $(BIN_DIR)"
	@rm -rf $(BIN_DIR)

.PHONY: all test generate clean .DEPENDENCIES .ASM_FILES .OBJECT_FILES .BINARIES .TEST_BINARIES .TEST_RESULTS .TESTS
