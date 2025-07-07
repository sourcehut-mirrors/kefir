KEFIR_END2END_TEST_NAME := $(patsubst source/tests/end2end/%/Makefile.mk,%,$(lastword $(MAKEFILE_LIST)))
$(KEFIR_END2END_BIN_PATH)/$(KEFIR_END2END_TEST_NAME).test.done: override USE_VALGRIND=no
$(KEFIR_END2END_BIN_PATH)/$(KEFIR_END2END_TEST_NAME)/lib2.test.asmgen.done: override USE_VALGRIND=no
$(KEFIR_END2END_BIN_PATH)/$(KEFIR_END2END_TEST_NAME)/lib3.test.asmgen.done: override USE_VALGRIND=no
$(KEFIR_END2END_BIN_PATH)/$(KEFIR_END2END_TEST_NAME)/lib4.test.asmgen.done: override USE_VALGRIND=no