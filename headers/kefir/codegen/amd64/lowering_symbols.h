/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KEFIR_CODEGEN_AMD64_LOWERING_SYMBOLS_H_
#define KEFIR_CODEGEN_AMD64_LOWERING_SYMBOLS_H_

#define BIGINT_GET_SET_SIGNED_INTEGER_FN "__kefir_bigint_set_signed_integer"
#define BIGINT_GET_SET_UNSIGNED_INTEGER_FN "__kefir_bigint_set_unsigned_integer"
#define BIGINT_CAST_SIGNED_FN "__kefir_bigint_cast_signed"
#define BIGINT_CAST_UNSIGNED_FN "__kefir_bigint_cast_unsigned"
#define BIGINT_SIGNED_TO_FLOAT_FN "__kefir_bigint_signed_to_float"
#define BIGINT_UNSIGNED_TO_FLOAT_FN "__kefir_bigint_unsigned_to_float"
#define BIGINT_SIGNED_TO_DOUBLE_FN "__kefir_bigint_signed_to_double"
#define BIGINT_UNSIGNED_TO_DOUBLE_FN "__kefir_bigint_unsigned_to_double"
#define BIGINT_SIGNED_TO_LONG_DOUBLE_FN "__kefir_bigint_signed_to_long_double"
#define BIGINT_UNSIGNED_TO_LONG_DOUBLE_FN "__kefir_bigint_unsigned_to_long_double"
#define BIGINT_SIGNED_FROM_FLOAT_FN "__kefir_bigint_signed_from_float"
#define BIGINT_SIGNED_FROM_DOUBLE_FN "__kefir_bigint_signed_from_double"
#define BIGINT_SIGNED_FROM_LONG_DOUBLE_FN "__kefir_bigint_signed_from_long_double"
#define BIGINT_UNSIGNED_FROM_FLOAT_FN "__kefir_bigint_unsigned_from_float"
#define BIGINT_UNSIGNED_FROM_DOUBLE_FN "__kefir_bigint_unsigned_from_double"
#define BIGINT_UNSIGNED_FROM_LONG_DOUBLE_FN "__kefir_bigint_unsigned_from_long_double"
#define BIGINT_IS_ZERO_FN "__kefir_bigint_is_zero"
#define BIGINT_NEGATE_FN "__kefir_bigint_negate"
#define BIGINT_INVERT_FN "__kefir_bigint_invert"
#define BIGINT_ADD_FN "__kefir_bigint_add"
#define BIGINT_SUBTRACT_FN "__kefir_bigint_subtract"
#define BIGINT_UNSIGNED_MULTIPLY_FN "__kefir_bigint_unsigned_multiply"
#define BIGINT_SIGNED_MULTIPLY_FN "__kefir_bigint_signed_multiply"
#define BIGINT_UNSIGNED_DIVIDE_FN "__kefir_bigint_unsigned_divide"
#define BIGINT_SIGNED_DIVIDE_FN "__kefir_bigint_signed_divide"
#define BIGINT_AND_FN "__kefir_bigint_and"
#define BIGINT_OR_FN "__kefir_bigint_or"
#define BIGINT_XOR_FN "__kefir_bigint_xor"
#define BIGINT_LEFT_SHIFT_FN "__kefir_bigint_left_shift"
#define BIGINT_RIGHT_SHIFT_FN "__kefir_bigint_right_shift"
#define BIGINT_ARITHMETIC_RIGHT_SHIFT_FN "__kefir_bigint_arithmetic_right_shift"
#define BIGINT_UNSIGNED_COMPARE_FN "__kefir_bigint_unsigned_compare"
#define BIGINT_SIGNED_COMPARE_FN "__kefir_bigint_signed_compare"
#define BIGINT_LEAST_SIGNIFICANT_NONZERO_FN "__kefir_bigint_least_significant_nonzero"
#define BIGINT_LEADING_ZEROS_FN "__kefir_bigint_leading_zeros"
#define BIGINT_TRAILING_ZEROS_FN "__kefir_bigint_trailing_zeros"
#define BIGINT_REDUNDANT_SIGN_BITS_FN "__kefir_bigint_redundant_sign_bits"
#define BIGINT_NONZERO_COUNT_FN "__kefir_bigint_nonzero_count"
#define BIGINT_PARITY_FN "__kefir_bigint_parity"

#define BUILTIN_FFS_FN "__kefir_builtin_ffs"
#define BUILTIN_FFSL_FN "__kefir_builtin_ffsl"
#define BUILTIN_CLZ_FN "__kefir_builtin_clz"
#define BUILTIN_CLZL_FN "__kefir_builtin_clzl"
#define BUILTIN_CTZ_FN "__kefir_builtin_ctz"
#define BUILTIN_CTZL_FN "__kefir_builtin_ctzl"
#define BUILTIN_CLRSB_FN "__kefir_builtin_clrsb"
#define BUILTIN_CLRSBL_FN "__kefir_builtin_clrsbl"
#define BUILTIN_POPCOUNT_FN "__kefir_builtin_popcount"
#define BUILTIN_POPCOUNTL_FN "__kefir_builtin_popcountl"
#define BUILTIN_PARITY_FN "__kefir_builtin_parity"
#define BUILTIN_PARITYL_FN "__kefir_builtin_parityl"
#define BUILTIN_ATOMIC_FETCH_ADD8_FN "__kefir_builtin_atomic_fetch_add8"
#define BUILTIN_ATOMIC_FETCH_ADD16_FN "__kefir_builtin_atomic_fetch_add16"
#define BUILTIN_ATOMIC_FETCH_ADD32_FN "__kefir_builtin_atomic_fetch_add32"
#define BUILTIN_ATOMIC_FETCH_ADD64_FN "__kefir_builtin_atomic_fetch_add64"

#define LIBATOMIC_SEQ_CST 5

#define LIBATOMIC_LOAD_N(_n) "__atomic_load_" #_n
#define LIBATOMIC_STORE_N(_n) "__atomic_store_" #_n
#define LIBATOMIC_LOAD "__atomic_load"
#define LIBATOMIC_STORE "__atomic_store"
#define LIBATOMIC_CMPXCHG_N(_n) "__atomic_compare_exchange_" #_n
#define LIBATOMIC_CMPXCHG "__atomic_compare_exchange"

#define KEFIR_SOFTFLOAT_COMPLEX_FLOAT_MUL "__kefir_softfloat_complex_float_mul"
#define KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_MUL "__kefir_softfloat_complex_double_mul"
#define KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_MUL "__kefir_softfloat_complex_long_double_mul"
#define KEFIR_SOFTFLOAT_COMPLEX_FLOAT_DIV "__kefir_softfloat_complex_float_div"
#define KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_DIV "__kefir_softfloat_complex_double_div"
#define KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_DIV "__kefir_softfloat_complex_long_double_div"

#define LIBGCC_UDIVTI3 "__udivti3"
#define LIBGCC_DIVTI3 "__divti3"
#define LIBGCC_UMODTI3 "__umodti3"
#define LIBGCC_MODTI3 "__modti3"

#define LIBGCC_FLOATTISF "__floattisf"
#define LIBGCC_FLOATTIDF "__floattidf"
#define LIBGCC_FLOATTIXF "__floattixf"
#define LIBGCC_FLOATUNTISF "__floatuntisf"
#define LIBGCC_FLOATUNTIDF "__floatuntidf"
#define LIBGCC_FLOATUNTIXF "__floatuntixf"

#define LIBGCC_FIXSFTI "__fixsfti"
#define LIBGCC_FIXDFTI "__fixdfti"
#define LIBGCC_FIXXFTI "__fixxfti"
#define LIBGCC_FIXUNSSFTI "__fixunssfti"
#define LIBGCC_FIXUNSDFTI "__fixunsdfti"
#define LIBGCC_FIXUNSXFTI "__fixunsxfti"

#define LIBGCC_DECIMAL_BID "__bid"
#define LIBGCC_DECIMAL_DPD "__dpd"
#define LIBGCC_DECIMAL_ADDSD3(_encoding) _encoding "_addsd3"
#define LIBGCC_DECIMAL_ADDDD3(_encoding) _encoding "_adddd3"
#define LIBGCC_DECIMAL_ADDTD3(_encoding) _encoding "_addtd3"
#define LIBGCC_DECIMAL_SUBSD3(_encoding) _encoding "_subsd3"
#define LIBGCC_DECIMAL_SUBDD3(_encoding) _encoding "_subdd3"
#define LIBGCC_DECIMAL_SUBTD3(_encoding) _encoding "_subtd3"
#define LIBGCC_DECIMAL_MULSD3(_encoding) _encoding "_mulsd3"
#define LIBGCC_DECIMAL_MULDD3(_encoding) _encoding "_muldd3"
#define LIBGCC_DECIMAL_MULTD3(_encoding) _encoding "_multd3"
#define LIBGCC_DECIMAL_DIVSD3(_encoding) _encoding "_divsd3"
#define LIBGCC_DECIMAL_DIVDD3(_encoding) _encoding "_divdd3"
#define LIBGCC_DECIMAL_DIVTD3(_encoding) _encoding "_divtd3"
#define LIBGCC_DECIMAL_EQSD3(_encoding) _encoding "_eqsd2"
#define LIBGCC_DECIMAL_EQDD3(_encoding) _encoding "_eqdd2"
#define LIBGCC_DECIMAL_EQTD3(_encoding) _encoding "_eqtd2"
#define LIBGCC_DECIMAL_GTSD3(_encoding) _encoding "_gtsd2"
#define LIBGCC_DECIMAL_GTDD3(_encoding) _encoding "_gtdd2"
#define LIBGCC_DECIMAL_GTTD3(_encoding) _encoding "_gttd2"
#define LIBGCC_DECIMAL_LTSD3(_encoding) _encoding "_ltsd2"
#define LIBGCC_DECIMAL_LTDD3(_encoding) _encoding "_ltdd2"
#define LIBGCC_DECIMAL_LTTD3(_encoding) _encoding "_lttd2"
// Decimal to decimal
#define LIBGCC_DECIMAL_EXTENDSDDD2(_encoding) _encoding "_extendsddd2"
#define LIBGCC_DECIMAL_EXTENDSDTD2(_encoding) _encoding "_extendsdtd2"
#define LIBGCC_DECIMAL_EXTENDDDTD2(_encoding) _encoding "_extendddtd2"
#define LIBGCC_DECIMAL_TRUNCDDSD2(_encoding) _encoding "_truncddsd2"
#define LIBGCC_DECIMAL_TRUNCTDSD2(_encoding) _encoding "_trunctdsd2"
#define LIBGCC_DECIMAL_TRUNCTDDD2(_encoding) _encoding "_trunctddd2"
// Decimal32 to ...
#define LIBGCC_DECIMAL_TRUNCSDSF(_encoding) _encoding "_truncsdsf"
#define LIBGCC_DECIMAL_EXTENDSDDF(_encoding) _encoding "_extendsddf"
#define LIBGCC_DECIMAL_EXTENDSDXF(_encoding) _encoding "_extendsdxf"
// Decimal64 to ...
#define LIBGCC_DECIMAL_TRUNCDDSF(_encoding) _encoding "_truncddsf"
#define LIBGCC_DECIMAL_TRUNCDDDF(_encoding) _encoding "_truncdddf"
#define LIBGCC_DECIMAL_EXTENDDDXF(_encoding) _encoding "_extendddxf"
// Decimal128 to ...
#define LIBGCC_DECIMAL_TRUNCTDSF(_encoding) _encoding "_trunctdsf"
#define LIBGCC_DECIMAL_TRUNCTDDF(_encoding) _encoding "_trunctddf"
#define LIBGCC_DECIMAL_TRUNCTDXF(_encoding) _encoding "_trunctdxf"
// Decimal32 from ...
#define LIBGCC_DECIMAL_EXTENDSFSD(_encoding) _encoding "_extendsfsd"
#define LIBGCC_DECIMAL_TRUNCDFSD(_encoding) _encoding "_truncdfsd"
#define LIBGCC_DECIMAL_TRUNCXFSD(_encoding) _encoding "_truncxfsd"
// Decimal64 from ...
#define LIBGCC_DECIMAL_EXTENDSFDD(_encoding) _encoding "_extendsfdd"
#define LIBGCC_DECIMAL_EXTENDDFDD(_encoding) _encoding "_extenddfdd"
#define LIBGCC_DECIMAL_TRUNCXFDD(_encoding) _encoding "_truncxfdd"
// Decimal128 from ...
#define LIBGCC_DECIMAL_EXTENDSFTD(_encoding) _encoding "_extendsftd"
#define LIBGCC_DECIMAL_EXTENDDFTD(_encoding) _encoding "_extenddftd"
#define LIBGCC_DECIMAL_EXTENDXFTD(_encoding) _encoding "_extendxftd"
// ... to long
#define LIBGCC_DECIMAL_FIXSDDI(_encoding) _encoding "_fixsddi"
#define LIBGCC_DECIMAL_FIXDDDI(_encoding) _encoding "_fixdddi"
#define LIBGCC_DECIMAL_FIXTDDI(_encoding) _encoding "_fixtddi"
// ... to unsigned long
#define LIBGCC_DECIMAL_FIXUNSSDDI(_encoding) _encoding "_fixunssddi"
#define LIBGCC_DECIMAL_FIXUNSDDDI(_encoding) _encoding "_fixunsdddi"
#define LIBGCC_DECIMAL_FIXUNSTDDI(_encoding) _encoding "_fixunstddi"
// long from ...
#define LIBGCC_DECIMAL_FLOATDISD(_encoding) _encoding "_floatdisd"
#define LIBGCC_DECIMAL_FLOATDIDD(_encoding) _encoding "_floatdidd"
#define LIBGCC_DECIMAL_FLOATDITD(_encoding) _encoding "_floatditd"
// unsigned  long from ...
#define LIBGCC_DECIMAL_FLOATUNSDISD(_encoding) _encoding "_floatunsdisd"
#define LIBGCC_DECIMAL_FLOATUNSDIDD(_encoding) _encoding "_floatunsdidd"
#define LIBGCC_DECIMAL_FLOATUNSDITD(_encoding) _encoding "_floatunsditd"

#define LIBGCC_DECIMAL_FLOATBITINTSD(_encoding) _encoding "_floatbitintsd"
#define LIBGCC_DECIMAL_FLOATBITINTDD(_encoding) _encoding "_floatbitintdd"
#define LIBGCC_DECIMAL_FLOATBITINTTD(_encoding) _encoding "_floatbitinttd"

#define LIBGCC_DECIMAL_FIXSDBITINT(_encoding) _encoding "_fixsdbitint"
#define LIBGCC_DECIMAL_FIXDDBITINT(_encoding) _encoding "_fixddbitint"
#define LIBGCC_DECIMAL_FIXTDBITINT(_encoding) _encoding "_fixtdbitint"

#define LIBGCC_DECIMAL_UNORDSD2(_encoding) _encoding "_unordsd2"
#define LIBGCC_DECIMAL_UNORDDD2(_encoding) _encoding "_unorddd2"
#define LIBGCC_DECIMAL_UNORDTD2(_encoding) _encoding "_unordtd2"

#define LIBGCC_DECIMAL_FLOATTISD(_encoding) _encoding "_floattisd"
#define LIBGCC_DECIMAL_FLOATTIDD(_encoding) _encoding "_floattidd"
#define LIBGCC_DECIMAL_FLOATTITD(_encoding) _encoding "_floattitd"
#define LIBGCC_DECIMAL_FLOATUNSTISD(_encoding) _encoding "_floatunstisd"
#define LIBGCC_DECIMAL_FLOATUNSTIDD(_encoding) _encoding "_floatunstidd"
#define LIBGCC_DECIMAL_FLOATUNSTITD(_encoding) _encoding "_floatunstitd"

#define LIBGCC_DECIMAL_FIXSDTI(_encoding) _encoding "_fixsdti"
#define LIBGCC_DECIMAL_FIXDDTI(_encoding) _encoding "_fixddti"
#define LIBGCC_DECIMAL_FIXTDTI(_encoding) _encoding "_fixtdti"
#define LIBGCC_DECIMAL_FIXUNSSDTI(_encoding) _encoding "_fixunssdti"
#define LIBGCC_DECIMAL_FIXUNSDDTI(_encoding) _encoding "_fixunsddti"
#define LIBGCC_DECIMAL_FIXUNSTDTI(_encoding) _encoding "_fixunstdti"


#endif
