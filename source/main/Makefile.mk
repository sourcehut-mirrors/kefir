KEFIR_STANDALONE_DEPENDENCIES := $(BIN_DIR)/main/standalone.d $(BIN_DIR)/main/runner.d $(BIN_DIR)/main/driver.d
KEFIR_STANDALONE_OBJECT_FILES := $(BIN_DIR)/main/standalone.o $(BIN_DIR)/main/runner.o $(BIN_DIR)/main/driver.o $(BIN_DIR)/main/help.s.o

KEFIR_DRIVER_DEPENDENCIES := $(BIN_DIR)/main/driver_main.d $(BIN_DIR)/main/runner.d $(BIN_DIR)/main/driver.d
KEFIR_DRIVER_OBJECT_FILES := $(BIN_DIR)/main/driver_main.o $(BIN_DIR)/main/runner.o $(BIN_DIR)/main/driver.o $(BIN_DIR)/main/help.s.o

KEFIR_STANDALONE_LINKED_LIBS=
KEFIR_DRIVER_LINKED_LIBS=
ifeq ($(SANITIZE),undefined)
KEFIR_STANDALONE_LINKED_LIBS=-fsanitize=undefined
KEFIR_DRIVER_LINKED_LIBS=-fsanitize=undefined
endif

$(BIN_DIR)/main/help.s.o: $(SOURCE_DIR)/main/help.txt

$(BIN_DIR)/kefir: $(KEFIR_STANDALONE_OBJECT_FILES) $(LIBKEFIR_SO)
	@mkdir -p $(shell dirname "$@")
	$(CC) -o $@ $(KEFIR_STANDALONE_OBJECT_FILES) $(KEFIR_STANDALONE_LINKED_LIBS) -L $(LIB_DIR) -lkefir

$(BIN_DIR)/kefir_driver: $(KEFIR_DRIVER_OBJECT_FILES) $(LIBKEFIR_SO)
	@mkdir -p $(shell dirname "$@")
	$(CC) -o $@ $(KEFIR_DRIVER_OBJECT_FILES) $(KEFIR_DRIVER_LINKED_LIBS) -L $(LIB_DIR) -lkefir

DEPENDENCIES += $(KEFIR_STANDALONE_DEPENDENCIES) $(KEFIR_DRIVER_DEPENDENCIES)
OBJECT_FILES += $(KEFIR_STANDALONE_OBJECT_FILES) $(KEFIR_DRIVER_OBJECT_FILES)
BINARIES += $(BIN_DIR)/kefir $(BIN_DIR)/kefir_driver
