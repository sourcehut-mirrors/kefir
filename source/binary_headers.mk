$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: $(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h
$(BIN_HEADERS_DESTDIR)/compiler/predefined_defs.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_SRCDIR)/compiler/predefined_defs.h --zero
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: $(GENERATED_HELP_DIR)/kefir-cc1.1.txt
$(BIN_HEADERS_DESTDIR)/cc1/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir-cc1.1.txt --zero
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: $(GENERATED_HELP_DIR)/kefir.1.txt
$(BIN_HEADERS_DESTDIR)/driver/help.binary.h: BINARY_HEADER_CONTENT=$(GENERATED_HELP_DIR)/kefir.1.txt --zero

$(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.h: $(wildcard $(BIN_HEADERS_INCDIR)/kefir_bigint/*.h)
	@echo "Generating $@"
	@mkdir -p "$(shell dirname $@)"
	@$(BIN_HEADERS_CC) -E -I "$(BIN_HEADERS_INCDIR)" \
		-D__KEFIR_BIGINT_USE_BIGINT_IMPL__  \
		-D__KEFIR_BIGINT_CHAR_BIT=8 \
		-D__KEFIR_BIGINT_FLT_MANT_DIG=__FLT_MANT_DIG__ \
		-D__KEFIR_BIGINT_DBL_MANT_DIG=__DBL_MANT_DIG__ \
		-D__KEFIR_BIGINT_LDBL_MANT_DIG=__LDBL_MANT_DIG__ \
		"$(BIN_HEADERS_INCDIR)/kefir_bigint/bigint.h" > "$@.tmp"
	@mv "$@.tmp" "$@"
$(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.binary.h: $(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.h
$(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.h

$(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.h: $(wildcard $(BIN_HEADERS_INCDIR)/kefir_softfloat/*.h)
	@echo "Generating $@"
	@mkdir -p "$(shell dirname $@)"
	@$(BIN_HEADERS_CC) -E -I "$(BIN_HEADERS_INCDIR)" \
		-D__KEFIR_SOFTFLOAT_USE_SOFTFLOAT_IMPL__  \
		-D__KEFIR_SOFTFLOAT_LDBL_MANT_DIG__="__LDBL_MANT_DIG__" \
		-D__KEFIR_SOFTFLOAT_BOOL_TYPE_T__=_Bool \
		-D__KEFIR_SOFTFLOAT_INT_T__="int" \
		-D__KEFIR_SOFTFLOAT_UINT64_T__="__UINT64_TYPE__" \
		-D__KEFIR_SOFTFLOAT_FLOAT_T__="float" \
		-D__KEFIR_SOFTFLOAT_DOUBLE_T__="double" \
		-D__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__="long double" \
		-D__KEFIR_SOFTFLOAT_COMPLEX_FLOAT_T__="_Complex float" \
		-D__KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_T__="_Complex double" \
		-D__KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_T__="_Complex long double" \
		-D__KEFIR_SOFTFLOAT_ISNAN__=__builtin_isnan \
		-D__KEFIR_SOFTFLOAT_ISINF_SIGN__=__builtin_isinf_sign \
		-D__KEFIR_SOFTFLOAT_COPYSIGNF__=__builtin_copysignf \
		-D__KEFIR_SOFTFLOAT_COPYSIGN__=__builtin_copysign \
		-D__KEFIR_SOFTFLOAT_COPYSIGNL__=__builtin_copysignl \
		-D__KEFIR_SOFTFLOAT_INFINITY__="__builtin_inff()" \
		-D__KEFIR_SOFTFLOAT_ISGREATER__="__builtin_isgreater" \
		-D__KEFIR_SOFTFLOAT_ISLESS__="__builtin_isless" \
		-D__KEFIR_SOFTFLOAT_MAKE_COMPLEX_FLOAT__="__kefir_builtin_construct_complex_float" \
		-D__KEFIR_SOFTFLOAT_MAKE_COMPLEX_DOUBLE__="__kefir_builtin_construct_complex_double" \
		-D__KEFIR_SOFTFLOAT_MAKE_COMPLEX_LONG_DOUBLE__="__kefir_builtin_construct_complex_long_double" \
		"$(BIN_HEADERS_INCDIR)/kefir_softfloat/softfloat.h" > "$@.tmp"
	@mv "$@.tmp" "$@"
$(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.binary.h: $(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.h
$(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.binary.h: BINARY_HEADER_CONTENT=$(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.h

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

$(BIN_HEADERS_DESTDIR)/compiler/runtime.deps: $(BIN_HEADERS_DESTDIR)/compiler/kefir_bigint.binary.h $(BIN_HEADERS_DESTDIR)/compiler/kefir_softfloat.binary.h
	@mkdir -p $(shell dirname "$@")
	@echo "-I$(shell dirname "$@") -DKEFIR_COMPILER_RUNTIME_KEFIR_BIGINT_INCLUDE=kefir_bigint.binary.h" >> $@
	@echo "-I$(shell dirname "$@") -DKEFIR_COMPILER_RUNTIME_KEFIR_SOFTFLOAT_INCLUDE=kefir_softfloat.binary.h" >> $@