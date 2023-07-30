$(BOOTSTRAP_DIR)/stage1/kefir: $(BIN_DIR)/kefir $(LIBKEFIRRT_A)
	@echo "Bootstrapping $@"
	@LD_LIBRARY_PATH=$(BIN_DIR)/libs:$LD_LIBRARY_PATH $(MAKE) -f $(ROOT)/bootstrap.mk bootstrap \
		ROOT=$(ROOT) \
		SOURCE=$(SOURCE_DIR) \
		HEADERS=$(HEADERS_DIR) \
		BOOTSTRAP=$(BOOTSTRAP_DIR)/stage1 \
		KEFIRCC=$(BIN_DIR)/kefir \
		KEFIR_HOST_ENV_CONFIG_HEADER=$(KEFIR_HOST_ENV_CONFIG_HEADER) \
		USE_SHARED=$(USE_SHARED)

$(BOOTSTRAP_DIR)/stage2/kefir: $(BOOTSTRAP_DIR)/stage1/kefir $(LIBKEFIRRT_A)
	@echo "Bootstrapping $@"
	@LD_LIBRARY_PATH=$(BOOTSTRAP_DIR)/stage1:$LD_LIBRARY_PATH $(MAKE) -f $(ROOT)/bootstrap.mk bootstrap \
		ROOT=$(ROOT) \
		SOURCE=$(SOURCE_DIR) \
		HEADERS=$(HEADERS_DIR) \
		BOOTSTRAP=$(BOOTSTRAP_DIR)/stage2 \
		KEFIRCC=$(BOOTSTRAP_DIR)/stage1/kefir \
		KEFIR_HOST_ENV_CONFIG_HEADER=$(KEFIR_HOST_ENV_CONFIG_HEADER) \
		USE_SHARED=$(USE_SHARED)

$(BOOTSTRAP_DIR)/stage2/comparison.done: $(BOOTSTRAP_DIR)/stage1/kefir $(BOOTSTRAP_DIR)/stage2/kefir
	@echo "Comparing stage1 and stage2 results"
	@$(SCRIPTS_DIR)/bootstrap_compare.sh "$(BOOTSTRAP_DIR)/stage1" "$(BOOTSTRAP_DIR)/stage2"
	@touch "$@"

BOOTSTRAP += $(BOOTSTRAP_DIR)/stage2/comparison.done
