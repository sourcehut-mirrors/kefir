KEFIR_IR_OPCODES_HEADER_XSL=$(RESOURCES_DIR)/kefir_ir_opcodes_header.xsl
KEFIR_IR_MNEMONIC_HEADER_XSL=$(RESOURCES_DIR)/kefir_ir_mnemonic_header.xsl
KEFIR_IR_MNEMONIC_SOURCE_XSL=$(RESOURCES_DIR)/kefir_ir_mnemonic_source.xsl
KEFIR_IR_FORMAT_IMPL_SOURCE_XSL=$(RESOURCES_DIR)/kefir_ir_format_impl_source.xsl

define generate_from_opcodes
	@echo "Generating $(1)"
	@$(XSLTPROC) $(2) $(RESOURCES_DIR)/opcodes.xml > $(1)
	@$(CLANG_FORMAT) -i --style=file $(1)
endef

rebuild_opcodes:
	@mkdir -p $(@D)
	$(call generate_from_opcodes,$(HEADERS_DIR)/kefir/ir/opcodes.h,$(KEFIR_IR_OPCODES_HEADER_XSL))
	$(call generate_from_opcodes,$(HEADERS_DIR)/kefir/ir/mnemonic.h,$(KEFIR_IR_MNEMONIC_HEADER_XSL))
	$(call generate_from_opcodes,$(SOURCE_DIR)/ir/mnemonic.c,$(KEFIR_IR_MNEMONIC_SOURCE_XSL))
	$(call generate_from_opcodes,$(SOURCE_DIR)/ir/format_impl.c,$(KEFIR_IR_FORMAT_IMPL_SOURCE_XSL))

.PHONY: rebuild_opcodes
