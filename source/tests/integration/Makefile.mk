KEFIR_INTEGRATION_TESTS_SOURCES := $(wildcard \
	$(SOURCE_DIR)/tests/integration/ast/*.test.c \
	$(SOURCE_DIR)/tests/integration/codegen_x86_64/*.test.c \
	$(SOURCE_DIR)/tests/integration/lexer/*.test.c \
	$(SOURCE_DIR)/tests/integration/misc/*.test.c \
	$(SOURCE_DIR)/tests/integration/parser/*.test.c \
	$(SOURCE_DIR)/tests/integration/preprocessor/*.test.c)
KEFIR_INTEGRATION_TEST_ALL_SOURCES := $(KEFIR_INTEGRATION_TESTS_SOURCES)
KEFIR_INTEGRATION_TEST_ALL_SOURCES += $(SOURCE_DIR)/tests/int_test.c
KEFIR_INTEGRATION_TEST_COMPILE_DEPS := $(KEFIR_INTEGRATION_TEST_ALL_SOURCES:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.deps)
KEFIR_INTEGRATION_TEST_DEPENDENCIES := $(KEFIR_INTEGRATION_TEST_ALL_SOURCES:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_INTEGRATION_TEST_OBJECT_FILES := $(KEFIR_INTEGRATION_TEST_ALL_SOURCES:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.o)
KEFIR_INTEGRATION_TEST_BINARIES := $(KEFIR_INTEGRATION_TESTS_SOURCES:$(SOURCE_DIR)/tests/integration/%.c=$(BIN_DIR)/tests/integration/%)
KEFIR_INTEGRATION_TEST_DONE := $(KEFIR_INTEGRATION_TESTS_SOURCES:$(SOURCE_DIR)/tests/integration/%.c=$(BIN_DIR)/tests/integration/%.done)
KEFIR_INTEGRATION_TEST_RESULTS := $(KEFIR_INTEGRATION_TESTS_SOURCES:$(SOURCE_DIR)/tests/integration/%.test.c=$(SOURCE_DIR)/tests/integration/%.test.result)

KEFIR_INTEGRATION_TEST_LIBS=
ifeq ($(SANITIZE),undefined)
KEFIR_INTEGRATION_TEST_LIBS=-fsanitize=undefined
endif

ifeq ($(USE_SHARED),yes)
KEFIR_INTEGRATION_TEST_LIBS+=-L $(LIB_DIR) -lkefir
else
KEFIR_INTEGRATION_TEST_LIBS+=$(LIBKEFIR_A)
endif

$(BIN_DIR)/tests/integration/%: $(BIN_DIR)/tests/integration/%.o $(LIBKEFIR_DEPENDENCY) \
                                     $(BIN_DIR)/tests/int_test.o \
									 $(BIN_DIR)/tests/util/util.o
	@mkdir -p $(@D)
	@echo "Linking $@"
	@$(CC) -o $@ $(BIN_DIR)/tests/int_test.o \
	                             $(BIN_DIR)/tests/util/util.o \
								 $< \
								 $(KEFIR_INTEGRATION_TEST_LIBS)
								 

$(BIN_DIR)/tests/integration/%.done: $(BIN_DIR)/tests/integration/%
	@LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(LIB_DIR) \
	 VALGRIND_TEST_OPTIONS="$(VALGRIND_TEST_OPTIONS)" \
	 MEMCHECK="$(MEMCHECK)" \
		$(SOURCE_DIR)/tests/integration/run.sh $<
	@touch $@

$(SOURCE_DIR)/tests/integration/%.test.result: $(BIN_DIR)/tests/integration/%.test
	@echo "Rebuilding $@"
	@LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(LIB_DIR) KEFIR_DISABLE_LONG_DOUBLE=1 $^ > $@

TEST_ARTIFACTS += $(KEFIR_INTEGRATION_TEST_RESULTS)
COMPILE_DEPS += $(KEFIR_INTEGRATION_TEST_COMPILE_DEPS)
DEPENDENCIES += $(KEFIR_INTEGRATION_TEST_DEPENDENCIES)
OBJECT_FILES += $(KEFIR_INTEGRATION_TEST_OBJECT_FILES)
TEST_BINARIES += $(KEFIR_INTEGRATION_TEST_BINARIES)
TESTS += $(KEFIR_INTEGRATION_TEST_DONE)
