KEFIR_EXTERNAL_TESTS_MAKEFILES := $(wildcard $(SOURCE_DIR)/tests/external/*/Makefile.mk)
KEFIR_EXTERNAL_TESTS_DONE = $(KEFIR_EXTERNAL_TESTS_MAKEFILES:$(SOURCE_DIR)/tests/external/%/Makefile.mk=$(KEFIR_BIN_DIR)/tests/external/%.test.done)

KEFIR_EXTERNAL_TESTS_DIR := $(KEFIR_BIN_DIR)/tests/external

include $(KEFIR_EXTERNAL_TESTS_MAKEFILES)
