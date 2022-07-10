KEFIR_STANDALONE_SOURCE := $(SOURCE_DIR)/standalone/main.c \
	                       $(SOURCE_DIR)/standalone/options.c \
						   $(SOURCE_DIR)/driver/runner.c \
						   $(SOURCE_DIR)/driver/compiler_options.c

KEFIR_STANDALONE_DEPENDENCIES := $(KEFIR_STANDALONE_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.d)
KEFIR_STANDALONE_OBJECT_FILES := $(KEFIR_STANDALONE_SOURCE:$(SOURCE_DIR)/%.c=$(BIN_DIR)/%.o)
KEFIR_STANDALONE_OBJECT_FILES += $(BIN_DIR)/standalone/help.s.o

KEFIR_STANDALONE_LINKED_LIBS=
ifeq ($(SANITIZE),undefined)
KEFIR_STANDALONE_LINKED_LIBS=-fsanitize=undefined
endif

$(BIN_DIR)/standalone/help.s.o: $(SOURCE_DIR)/standalone/help.txt

$(BIN_DIR)/kefir: $(KEFIR_STANDALONE_OBJECT_FILES) $(LIBKEFIR_SO)
	@mkdir -p $(shell dirname "$@")
	@echo "Linking $@"
	@$(CC) -o $@ $(KEFIR_STANDALONE_OBJECT_FILES) $(KEFIR_STANDALONE_LINKED_LIBS) -L $(LIB_DIR) -lkefir

DEPENDENCIES += $(KEFIR_STANDALONE_DEPENDENCIES)
OBJECT_FILES += $(KEFIR_STANDALONE_OBJECT_FILES)
BINARIES += $(BIN_DIR)/kefir
