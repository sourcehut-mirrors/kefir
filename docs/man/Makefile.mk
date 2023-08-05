KEFIR_MAN_PAGES=$(DOCS_MAN_DIR)/kefir.1 \
                $(DOCS_MAN_DIR)/kefir-cc1.1 \
                $(DOCS_MAN_DIR)/kefir-detect-host-env.1
KEFIR_MAN_GZIPPED_PAGES=$(KEFIR_MAN_PAGES:$(DOCS_MAN_DIR)/%=$(KEFIR_BIN_DIR)/man/%.gz)

$(KEFIR_BIN_DIR)/man/%.gz: $(DOCS_MAN_DIR)/%
	@mkdir -p $(shell dirname "$@")
	@echo "Gzipping $@"
	@$(GZIP) -c "$^" > "$@"

$(GENERATED_HELP_DIR)/%.txt: $(DOCS_MAN_DIR)/%
	@mkdir -p $(shell dirname "$@")
	@echo "Generating $@"
	@$(MDOC_CONV) -Tascii "$^" | sed "s,\x1B\[[0-9;]*[a-zA-Z],,g" > "$@"

MAN_PAGES += $(KEFIR_MAN_GZIPPED_PAGES)
