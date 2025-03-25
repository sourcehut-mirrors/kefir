ifeq ($(USE_EXTENSION_SUPPORT),yes)
$(KEFIR_END2END_BIN_PATH)/extensions1/lib.kefir.o: $(KEFIR_END2END_BIN_PATH)/extensions1_lib.so

ifeq ($(shell echo "__KEFIRCC__" | $(CC) -E -o- - | tr -d '\n'),1)
KEFIR_END2END_EXTENSIONS1_LIB_EXTRA_CFLAGS := --disable-atomics
else
KEFIR_END2END_EXTENSIONS1_LIB_EXTRA_CFLAGS :=
endif

$(KEFIR_END2END_BIN_PATH)/extensions1_lib.so: $(SOURCE_DIR)/tests/end2end/extensions1/extension.c
	@echo "Building $@"
	@$(CC) $(CFLAGS) -I$(HEADERS_DIR) -include "$(KEFIR_HOST_ENV_CONFIG_HEADER)" $(KEFIR_END2END_EXTENSIONS1_LIB_EXTRA_CFLAGS) -fPIC -shared $< -o $@ $(LDFLAGS)
else
KEFIR_END2END_EXCLUDE_TESTS+=$(SOURCE_DIR)/tests/end2end/extensions1/%
endif