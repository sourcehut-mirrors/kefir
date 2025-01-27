KEFIR_CSMITH_TEST_VERSION=2.4.0
KEFIR_CSMITH_TEST_GIT_VERSION=0ec6f1b
KEFIR_CSMITH_TEST_SEEDS=$(shell cat $(SOURCE_DIR)/tests/csmith/seed.list)
KEFIR_CSMITH_TESTS_DONE := $(KEFIR_CSMITH_TEST_SEEDS:%=$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done)

CSMITH_RANDOM_TESTS ?= 10000
CSMITH_RANDOM_JOBS ?= $(shell nproc)

$(KEFIR_BIN_DIR)/tests/csmith/kefir_valgrind: $(KEFIR_EXE)
	@mkdir -p "$(shell dirname $@)"
	@echo "Generating $@..."
	@echo "#!/usr/bin/env bash" > "$@"
	@echo "exec valgrind $(VALGRIND_TEST_OPTIONS) $(shell realpath $(KEFIR_EXE)) \$$@" >> "$@"
	@chmod +x "$@"

ifeq ($(USE_VALGRIND),yes)
CSMITH_KEFIR_RUNNER=$(KEFIR_BIN_DIR)/tests/csmith/kefir_valgrind
else
CSMITH_KEFIR_RUNNER=$(KEFIR_EXE)
endif

$(KEFIR_BIN_DIR)/tests/csmith/version:
	@mkdir -p "$(shell dirname $@)"
	@echo "Checking csmith version..."
	@$(CSMITH) -v > "$@.tmp"
	@grep "csmith $(KEFIR_CSMITH_TEST_VERSION)" "$@.tmp" >/dev/null || (echo "Expected csmith version $(KEFIR_CSMITH_TEST_VERSION)" && exit 1)
	@grep "Git version: $(KEFIR_CSMITH_TEST_GIT_VERSION)" "$@.tmp" >/dev/null || (echo "Expected csmith Git version $(KEFIR_CSMITH_TEST_GIT_VERSION)" && exit 1)
	@mv "$@.tmp" "$@"

$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: CSMITH_SEED=$(patsubst seed-%.test.done,%,$(notdir $@))
.SECONDEXPANSION:
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: $$(CSMITH_KEFIR_RUNNER) $(KEFIR_BIN_DIR)/tests/csmith/version
	@mkdir -p "$(shell dirname $@)"
	@echo "Running csmith test seed $(CSMITH_SEED)..."
	@LD_LIBRARY_PATH=$(KEFIR_BIN_DIR)/libs:$$LD_LIBRARY_PATH \
		KEFIR_RTINC=$(HEADERS_DIR)/kefir/runtime \
		"$(SOURCE_DIR)/tests/csmith/csmith_driver.py" --csmith "$(CSMITH)" \
		--kefir "$(CSMITH_KEFIR_RUNNER)" \
		--cc "$(CC)" \
		--seed "$(CSMITH_SEED)" \
		--jobs 1 \
		--tests 1 \
		--out "$(KEFIR_BIN_DIR)/tests/csmith/out-$(CSMITH_SEED)" \
		> "$@.tmp" 2>&1 || (cat "$@.tmp"; exit 1)
	@mv "$@.tmp" "$@"

.SECONDEXPANSION:
csmith_random_test: $$(CSMITH_KEFIR_RUNNER)
	@mkdir -p "$(KEFIR_BIN_DIR)/tests/csmith/random-out"
	@echo "Running random CSmith test..."
	@LD_LIBRARY_PATH=$(KEFIR_BIN_DIR)/libs:$$LD_LIBRARY_PATH \
		KEFIR_RTINC=$(HEADERS_DIR)/kefir/runtime \
		"$(SOURCE_DIR)/tests/csmith/csmith_driver.py" --csmith "$(CSMITH)" \
		--kefir "$(CSMITH_KEFIR_RUNNER)" \
		--cc "$(CC)" \
		--jobs "$(CSMITH_RANDOM_JOBS)" \
		--tests "$(CSMITH_RANDOM_TESTS)" \
		--out "$(KEFIR_BIN_DIR)/tests/csmith/random-out"

CSMITH_TESTS += $(KEFIR_CSMITH_TESTS_DONE)
ifneq ($(CSMITH),)
TESTS += $(CSMITH_TESTS)
endif

.PHONY: csmith_random_test