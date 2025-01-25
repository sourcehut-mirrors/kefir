KEFIR_CSMITH_TEST_SEEDS=5730988164224951711 \
						9366056735358650314
KEFIR_CSMITH_TESTS_DONE := $(KEFIR_CSMITH_TEST_SEEDS:%=$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done)

$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: CSMITH_SEED=$(patsubst seed-%.test.done,%,$(notdir $@))
$(KEFIR_BIN_DIR)/tests/csmith/seed-%.test.done: $(KEFIR_EXE)
	@mkdir -p "$(shell dirname $@)"
	@echo "Running csmith test seed $(CSMITH_SEED)..."
	@"$(SOURCE_DIR)/tests/csmith/csmith_driver.py" --csmith "$(CSMITH)" \
		--kefir "$(KEFIR_EXE)" \
		--cc "$(CC)" \
		--seed "$(CSMITH_SEED)" \
		--jobs 1 \
		--tests 1 \
		--out "$(KEFIR_BIN_DIR)/tests/csmith/out-$(CSMITH_SEED)" \
		> "$@.tmp" 2>&1 || (cat "$@.tmp"; exit 1)
	@mv "$@.tmp" "$@"

CSMITH_TESTS += $(KEFIR_CSMITH_TESTS_DONE)