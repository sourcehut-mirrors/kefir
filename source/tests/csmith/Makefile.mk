KEFIR_CSMITH_TEST_SEEDS=$(shell cat $(SOURCE_DIR)/tests/csmith/seed.list)
KEFIR_CSMITH_TESTS_DONE := $(KEFIR_CSMITH_TEST_SEEDS:%=$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done)

$(KEFIR_BIN_DIR)/tests/csmith/kefir_valgrind: $(KEFIR_EXE)
	@mkdir -p "$(shell dirname $@)"
	@echo "Generating $@..."
	@echo "#!/usr/bin/env bash" > "$@"
	@echo "exec valgrind $(VALGRIND_TEST_OPTIONS) $(shell realpath $(KEFIR_EXE)) \$$@" >> "$@"
	@chmod +x "$@"

ifeq ($(USE_VALGRIND),yes)
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: CSMITH_KEFIR_RUNNER=$(KEFIR_BIN_DIR)/tests/csmith/kefir_valgrind
else
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: CSMITH_KEFIR_RUNNER=$(KEFIR_EXE)
endif

.SECONDEXPANSION:
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: CSMITH_SEED=$(patsubst seed-%.test.done,%,$(notdir $@))
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: $$(CSMITH_KEFIR_RUNNER)
	@mkdir -p "$(shell dirname $@)"
	@echo "Running csmith test seed $(CSMITH_SEED)..."
	@"$(SOURCE_DIR)/tests/csmith/csmith_driver.py" --csmith "$(CSMITH)" \
		--kefir "$(CSMITH_KEFIR_RUNNER)" \
		--cc "$(CC)" \
		--seed "$(CSMITH_SEED)" \
		--jobs 1 \
		--tests 1 \
		--out "$(KEFIR_BIN_DIR)/tests/csmith/out-$(CSMITH_SEED)" \
		> "$@.tmp" 2>&1 || (cat "$@.tmp"; exit 1)
	@mv "$@.tmp" "$@"

CSMITH_TESTS += $(KEFIR_CSMITH_TESTS_DONE)

ifneq ($(CSMITH),)
TESTS += $(CSMITH_TESTS)
endif