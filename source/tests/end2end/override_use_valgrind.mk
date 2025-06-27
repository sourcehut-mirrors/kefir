ifeq ($(KEFIR_END2END_SELECTIVE_VALGRIND),yes)
KEFIR_END2END_TEST_NAME := $(patsubst source/tests/end2end/%/Makefile.mk,%,$(lastword $(MAKEFILE_LIST)))
$(KEFIR_END2END_BIN_PATH)/$(KEFIR_END2END_TEST_NAME).test.done: override USE_VALGRIND=no
endif