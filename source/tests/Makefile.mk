VALGRIND_TEST_OPTIONS=-q --trace-children=no --track-origins=yes --leak-check=full --error-exitcode=127
TEST_CFLAGS=-no-pie
KEFIR_TEST_USE_MUSL=no

ifeq ($(PLATFORM),freebsd)
VALGRIND_TEST_OPTIONS+=--suppressions=$(ROOT)/suppressions/freebsd.valgrind
TEST_CFLAGS=
endif

ifneq ($(PLATFORM),openbsd)
VALGRIND_TEST_OPTIONS+=--expensive-definedness-checks=yes
endif

include source/tests/unit/Makefile.mk
include source/tests/integration/Makefile.mk
include source/tests/system/Makefile.mk
include source/tests/end2end/Makefile.mk
include source/tests/external/Makefile.mk
include source/tests/csmith/Makefile.mk