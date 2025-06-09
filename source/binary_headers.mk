$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: $(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h --zero
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: $(GENERATED_HELP_DIR)/kefir-cc1.1.txt
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir-cc1.1.txt --zero
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: $(GENERATED_HELP_DIR)/kefir.1.txt
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir.1.txt --zero

$(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.h: $(wildcard $(HEADERS_DIR)/kefir_bigint/*.h)
	@echo "Generating $@"
	@mkdir -p "$(shell dirname $@)"
	@$(CC) -E -P -I "$(HEADERS_DIR)" \
		-D__KEFIR_BIGINT_USE_BIGINT_IMPL__  \
		-D__KEFIR_BIGINT_CHAR_BIT=8 \
		-D__KEFIR_BIGINT_FLT_MANT_DIG=__FLT_MANT_DIG__ \
		-D__KEFIR_BIGINT_DBL_MANT_DIG=__DBL_MANT_DIG__ \
		-D__KEFIR_BIGINT_LDBL_MANT_DIG=__LDBL_MANT_DIG__ \
		"$(HEADERS_DIR)/kefir_bigint/bigint.h" > "$@.tmp"
	@mv "$@.tmp" "$@"
$(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.binary.h: $(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.h
$(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.h --zero

$(BIN_HEADERS_DESTDIR)/compiler/compiler.deps: $(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo "-I$(shell dirname "$@") -DKEFIR_COMPILER_PREDEFINED_DEFS_INCLUDE=predefined_defs.binary.h" > $@
	
$(BIN_HEADERS_DESTDIR)/cc1/cc1.deps: $(BIN_HEADERS_DESTDIR)/cc1/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@") -DKEFIR_CC1_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/driver/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@") -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/web/main.deps: $(BIN_HEADERS_DESTDIR)/driver/help.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo '-I$(shell dirname "$@")/../driver -DKEFIR_DRIVER_HELP_INCLUDE=help.binary.h' > $@

$(BIN_HEADERS_DESTDIR)/codegen/codegen.deps: $(BIN_HEADERS_DESTDIR)/codegen/kefir_bigint.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo "-I$(shell dirname "$@") -DKEFIR_CODEGEN_KEFIR_BIGINT_INCLUDE=kefir_bigint.binary.h" >> $@