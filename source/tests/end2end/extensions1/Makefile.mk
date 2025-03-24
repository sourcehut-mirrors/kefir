ifeq ($(USE_EXTENSION_SUPPORT),yes)
$(KEFIR_END2END_BIN_PATH)/extensions1/lib.kefir.o: $(KEFIR_END2END_BIN_PATH)/extensions1_lib.so

$(KEFIR_END2END_BIN_PATH)/extensions1_lib.so: $(SOURCE_DIR)/tests/end2end/extensions1/extension.c
	@echo "Building $@"
	@$(CC) $(CFLAGS) -I$(HEADERS_DIR) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" -fPIC -shared $< -o $@
else
KEFIR_END2END_EXCLUDE_TESTS+=$(SOURCE_DIR)/tests/end2end/extensions1/%
endif