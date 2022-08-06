END2END_BIN_PATH=$(BIN_DIR)/tests/end2end
END2END_LIBS=-pthread

ifeq ($(PLATFORM),freebsd)
END2END_LIBS+=-lstdthreads
endif

END2END_ASMGEN_EXPECTED_FILES :=

$(BIN_DIR)/%.kefir.o: $(SOURCE_DIR)/%.kefir.c $(BIN_DIR)/kefir
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Compile $@"
	@VALGRIND_OPTIONS="$(VALGRIND_OPTIONS)" MEMCHECK="$(MEMCHECK)" PLATFORM="$(PLATFORM)" AS="$(AS)" $(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

$(END2END_BIN_PATH)/%.test: $(LIBKEFIRRT_A)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(CC) $(CC_TEST_FLAGS) $(SANFLAGS) $(END2END_TEST_OBJS) $(LIBKEFIRRT_A) -o $@ $(END2END_LIBS)

$(END2END_BIN_PATH)/%.test.done: $(END2END_BIN_PATH)/%.test
	@echo "Running $<"
ifeq ($(MEMCHECK),yes)
	@valgrind $(VALGRIND_OPTIONS) $<
else
	$<
endif
	@touch $@

$(END2END_BIN_PATH)/%.asmgen.output: $(SOURCE_DIR)/tests/end2end/%.kefir.asmgen.c $(BIN_DIR)/kefir
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Translate $@"
	@VALGRIND_OPTIONS="$(VALGRIND_OPTIONS)" MEMCHECK="$(MEMCHECK)" PLATFORM="$(PLATFORM)" AS="$(AS)" $(SOURCE_DIR)/tests/end2end/asmgen.sh $(BIN_DIR) $< $@

$(END2END_BIN_PATH)/%.test.asmgen.done: $(END2END_BIN_PATH)/%.asmgen.output
	@echo "Asmgen-Diff $^"
	@diff -u $(SOURCE_DIR)/tests/end2end/$*.asmgen.expected $<
	@touch $@

$(SOURCE_DIR)/tests/end2end/%.asmgen.expected: $(SOURCE_DIR)/tests/end2end/%.kefir.asmgen.c $(BIN_DIR)/kefir
	@echo "Rebuilding $@"
	@VALGRIND_OPTIONS="$(VALGRIND_OPTIONS)" MEMCHECK="$(MEMCHECK)" PLATFORM="$(PLATFORM)" AS="$(AS)" $(SOURCE_DIR)/tests/end2end/asmgen.sh $(BIN_DIR) $< $@

include source/tests/end2end/*/Makefile.mk

rebuild_end2end_asmgen_tests: $(END2END_ASMGEN_EXPECTED_FILES)