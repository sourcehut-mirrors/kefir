KEFIR_STANDALONE_SOURCE := $(SOURCE_DIR)/cc1/main.c \
	                       $(SOURCE_DIR)/cc1/options.c \
						   $(SOURCE_DIR)/driver/runner.c \
						   $(SOURCE_DIR)/driver/compiler_options.c

KEFIR_STANDALONE_COMPILE_DEPS := $(KEFIR_STANDALONE_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.deps)
KEFIR_STANDALONE_DEPENDENCIES := $(KEFIR_STANDALONE_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_STANDALONE_OBJECT_FILES := $(KEFIR_STANDALONE_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.o)

KEFIR_STANDALONE_LIBS=
ifeq ($(SANITIZE),undefined)
KEFIR_STANDALONE_LIBS=-fsanitize=undefined
endif

$(BIN_DIR)/cc1/help.binary.h: $(SOURCE_DIR)/cc1/help.txt
$(BIN_DIR)/cc1/help.binary.h: BINARY_HEADER_CONTENT=$(SOURCE_DIR)/cc1/help.txt --zero

$(BIN_DIR)/cc1/main.deps: $(BIN_DIR)/cc1/help.binary.h
	@echo '-I$(BIN_DIR)/cc1 -DKEFIR_CC1_HELP_INCLUDE=help.binary.h' > $@

$(KEFIR_CC1_EXE): $(KEFIR_STANDALONE_OBJECT_FILES) $(LIBKEFIR_DEPENDENCY)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
ifeq ($(USE_SHARED),yes)
	@$(LD) $(LDFLAGS) -o $@ $(KEFIR_STANDALONE_OBJECT_FILES) $(KEFIR_STANDALONE_LIBS) -L $(LIB_DIR) -lkefir
else
	@$(LD) $(LDFLAGS) -static -o $@ $(KEFIR_STANDALONE_OBJECT_FILES) $(LIBKEFIR_A) $(KEFIR_STANDALONE_LIBS)
endif

COMPILE_DEPS += $(KEFIR_STANDALONE_COMPILE_DEPS)
DEPENDENCIES += $(KEFIR_STANDALONE_DEPENDENCIES)
OBJECT_FILES += $(KEFIR_STANDALONE_OBJECT_FILES)
BINARIES += $(BIN_DIR)/kefir-cc1
