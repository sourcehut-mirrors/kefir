KEFIR_END2END_BIN_PATH=$(BIN_DIR)/tests/end2end
KEFIR_END2END_TEST_LIBS=-pthread

ifeq ($(PLATFORM),freebsd)
KEFIR_END2END_TEST_LIBS+=-lstdthreads
endif

KEFIR_END2END_ASMGEN_EXPECTED_FILES :=

$(BIN_DIR)/%.o: CFLAGS += -Wno-int-in-bool-context

$(BIN_DIR)/%.kefir.o: $(SOURCE_DIR)/%.kefir.c $(BIN_DIR)/kefir
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Compile $@"
	@AS="$(AS)" \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
	 PLATFORM="$(PLATFORM)" \
	 $(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

$(KEFIR_END2END_BIN_PATH)/%.test: $(LIBKEFIRRT_A)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(CC) $(TEST_CFLAGS) $(SANFLAGS) $(KEFIR_END2END_TEST_OBJS) $(LIBKEFIRRT_A) -o $@ $(KEFIR_END2END_TEST_LIBS)

$(KEFIR_END2END_BIN_PATH)/%.test.done: $(KEFIR_END2END_BIN_PATH)/%.test
	@echo "Running $<"
ifeq ($(MEMCHECK),yes)
	@valgrind $(VALGRIND_TEST_OPTIONS) $<
else
	$<
endif
	@touch $@

$(KEFIR_END2END_BIN_PATH)/%.asmgen.output: $(SOURCE_DIR)/tests/end2end/%.kefir.asmgen.c $(BIN_DIR)/kefir
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Translate $@"
	@ASMGEN=yes \
	 AS="$(AS)" \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
	 PLATFORM="$(PLATFORM)" \
	 	$(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

$(KEFIR_END2END_BIN_PATH)/%.also.asmgen.output: $(SOURCE_DIR)/tests/end2end/%.kefir.c $(BIN_DIR)/kefir
	@mkdir -p $(shell dirname "$@")
	@echo "Kefir-Translate $@"
	@ASMGEN=yes \
	 AS="$(AS)" \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
	 PLATFORM="$(PLATFORM)" \
	 	$(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

$(KEFIR_END2END_BIN_PATH)/%.test.asmgen.done: $(KEFIR_END2END_BIN_PATH)/%.asmgen.output
	@echo "Asmgen-Diff $^"
	@diff -u $(SOURCE_DIR)/tests/end2end/$*.asmgen.expected $<
	@touch $@

$(KEFIR_END2END_BIN_PATH)/%.test.also.asmgen.done: $(KEFIR_END2END_BIN_PATH)/%.also.asmgen.output
	@echo "Asmgen-Diff $^"
	@diff -u $(SOURCE_DIR)/tests/end2end/$*.also.asmgen.expected $<
	@touch $@

$(SOURCE_DIR)/tests/end2end/%.asmgen.expected: $(SOURCE_DIR)/tests/end2end/%.kefir.asmgen.c $(BIN_DIR)/kefir
	@echo "Rebuilding $@"
	@ASMGEN=yes \
	 AS="$(AS)" \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
	 PLATFORM="$(PLATFORM)" \
	 $(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

$(SOURCE_DIR)/tests/end2end/%.also.asmgen.expected: $(SOURCE_DIR)/tests/end2end/%.kefir.c $(BIN_DIR)/kefir
	@echo "Rebuilding $@"
	@ASMGEN=yes \
	 AS="$(AS)" \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
	 PLATFORM="$(PLATFORM)" \
	 $(SOURCE_DIR)/tests/end2end/compile.sh $(BIN_DIR) $< $@

include source/tests/end2end/*/Makefile.mk

TEST_ARTIFACTS += $(KEFIR_END2END_ASMGEN_EXPECTED_FILES)
