/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#ifndef KEFIR_OPTIMIZER_BUILDER_H_
#define KEFIR_OPTIMIZER_BUILDER_H_

#include "kefir/optimizer/code.h"

kefir_result_t kefir_opt_code_builder_add_instruction(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, const struct kefir_opt_operation *,
                                                      kefir_bool_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_add_control(struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                                  kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_builder_add_control_side_effect_free(struct kefir_opt_code_container *,
                                                                   kefir_opt_block_id_t, kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_builder_is_finalized(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                                   kefir_bool_t *);

kefir_result_t kefir_opt_code_builder_finalize_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                    kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                    kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_indirect_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                             kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                             kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_branch(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_branch_condition_variant_t,
                                                      kefir_opt_instruction_ref_t, kefir_opt_block_id_t,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_return(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_get_argument(struct kefir_mem *, struct kefir_opt_code_container *,
                                                   kefir_opt_block_id_t, kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_phi(struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                          kefir_opt_phi_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_inline_assembly(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_inline_assembly_id_t,
                                                      kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_invoke(struct kefir_mem *, struct kefir_opt_code_container *,
                                             kefir_opt_block_id_t, kefir_opt_call_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_invoke_virtual(struct kefir_mem *, struct kefir_opt_code_container *,
                                                     kefir_opt_block_id_t, kefir_opt_call_id_t,
                                                     kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_int_constant(struct kefir_mem *, struct kefir_opt_code_container *,
                                                   kefir_opt_block_id_t, kefir_int64_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_uint_constant(struct kefir_mem *, struct kefir_opt_code_container *,
                                                    kefir_opt_block_id_t, kefir_uint64_t,
                                                    kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_float32_constant(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_float32_t,
                                                       kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_float64_constant(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_float64_t,
                                                       kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_long_double_constant(struct kefir_mem *, struct kefir_opt_code_container *,
                                                           kefir_opt_block_id_t, kefir_long_double_t,
                                                           kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_string_reference(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_block_label(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                  kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_int_placeholder(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_float32_placeholder(struct kefir_mem *, struct kefir_opt_code_container *,
                                                          kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_float64_placeholder(struct kefir_mem *, struct kefir_opt_code_container *,
                                                          kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_get_global(struct kefir_mem *, struct kefir_opt_code_container *,
                                                 kefir_opt_block_id_t, kefir_id_t, kefir_int64_t,
                                                 kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_get_thread_local(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_id_t, kefir_int64_t,
                                                       kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_get_local(struct kefir_mem *, struct kefir_opt_code_container *,
                                                kefir_opt_block_id_t, kefir_size_t, kefir_int64_t,
                                                kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_zero_memory(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_instruction_ref_t, kefir_id_t,
                                                  kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_copy_memory(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t, kefir_id_t, kefir_size_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_bits_extract_signed(struct kefir_mem *, struct kefir_opt_code_container *,
                                                          kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                          kefir_size_t, kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_bits_extract_unsigned(struct kefir_mem *, struct kefir_opt_code_container *,
                                                            kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                            kefir_size_t, kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_bits_insert(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t, kefir_size_t, kefir_size_t,
                                                  kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_vararg_get(struct kefir_mem *, struct kefir_opt_code_container *,
                                                 kefir_opt_block_id_t, kefir_opt_instruction_ref_t, kefir_id_t,
                                                 kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_stack_alloc(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t, kefir_bool_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_scope_push(struct kefir_mem *, struct kefir_opt_code_container *,
                                                 kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_atomic_copy_memory_from(struct kefir_mem *, struct kefir_opt_code_container *,
                                                              kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                              kefir_opt_instruction_ref_t, kefir_opt_memory_order_t,
                                                              kefir_id_t, kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_atomic_copy_memory_to(struct kefir_mem *, struct kefir_opt_code_container *,
                                                            kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                            kefir_opt_instruction_ref_t, kefir_opt_memory_order_t,
                                                            kefir_id_t, kefir_size_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_atomic_compare_exchange_memory(
    struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
    kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t, kefir_opt_memory_order_t, kefir_id_t, kefir_size_t,
    kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_fenv_save(struct kefir_mem *, struct kefir_opt_code_container *,
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_fenv_clear(struct kefir_mem *, struct kefir_opt_code_container *,
                                                 kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_fenv_update(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t *);

#define UNARY_OP(_id)                                                                                  \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *, \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,     \
                                                kefir_opt_instruction_ref_t *)

UNARY_OP(int8_not);
UNARY_OP(int16_not);
UNARY_OP(int32_not);
UNARY_OP(int64_not);
UNARY_OP(int8_neg);
UNARY_OP(int16_neg);
UNARY_OP(int32_neg);
UNARY_OP(int64_neg);
UNARY_OP(int8_bool_not);
UNARY_OP(int16_bool_not);
UNARY_OP(int32_bool_not);
UNARY_OP(int64_bool_not);

UNARY_OP(int8_to_bool);
UNARY_OP(int16_to_bool);
UNARY_OP(int32_to_bool);
UNARY_OP(int64_to_bool);
UNARY_OP(int64_zero_extend_8bits);
UNARY_OP(int64_zero_extend_16bits);
UNARY_OP(int64_zero_extend_32bits);
UNARY_OP(int64_sign_extend_8bits);
UNARY_OP(int64_sign_extend_16bits);
UNARY_OP(int64_sign_extend_32bits);

UNARY_OP(vararg_start);
UNARY_OP(vararg_end);
UNARY_OP(scope_pop);

UNARY_OP(float32_neg);
UNARY_OP(float64_neg);
UNARY_OP(long_double_neg);

UNARY_OP(float32_to_int);
UNARY_OP(float64_to_int);
UNARY_OP(float32_to_uint);
UNARY_OP(float64_to_uint);
UNARY_OP(int_to_float32);
UNARY_OP(int_to_float64);
UNARY_OP(uint_to_float32);
UNARY_OP(uint_to_float64);
UNARY_OP(float32_to_float64);
UNARY_OP(float64_to_float32);
UNARY_OP(long_double_truncate_1bit);
UNARY_OP(long_double_to_int);
UNARY_OP(long_double_to_uint);
UNARY_OP(long_double_to_float32);
UNARY_OP(long_double_to_float64);

UNARY_OP(int_to_long_double);
UNARY_OP(uint_to_long_double);
UNARY_OP(float32_to_long_double);
UNARY_OP(float64_to_long_double);

UNARY_OP(complex_float32_real);
UNARY_OP(complex_float32_imaginary);
UNARY_OP(complex_float64_real);
UNARY_OP(complex_float64_imaginary);
UNARY_OP(complex_long_double_real);
UNARY_OP(complex_long_double_imaginary);

UNARY_OP(complex_float32_truncate_1bit);
UNARY_OP(complex_float64_truncate_1bit);
UNARY_OP(complex_long_double_truncate_1bit);

UNARY_OP(complex_float32_neg);
UNARY_OP(complex_float64_neg);
UNARY_OP(complex_long_double_neg);

#undef UNARY_OP

#define BINARY_OP(_id)                                                                                 \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *, \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,     \
                                                kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t *)

BINARY_OP(int8_add);
BINARY_OP(int16_add);
BINARY_OP(int32_add);
BINARY_OP(int64_add);
BINARY_OP(int8_sub);
BINARY_OP(int16_sub);
BINARY_OP(int32_sub);
BINARY_OP(int64_sub);
BINARY_OP(int8_mul);
BINARY_OP(int16_mul);
BINARY_OP(int32_mul);
BINARY_OP(int64_mul);
BINARY_OP(int8_div);
BINARY_OP(int16_div);
BINARY_OP(int32_div);
BINARY_OP(int64_div);
BINARY_OP(int8_mod);
BINARY_OP(int16_mod);
BINARY_OP(int32_mod);
BINARY_OP(int64_mod);
BINARY_OP(uint8_div);
BINARY_OP(uint16_div);
BINARY_OP(uint32_div);
BINARY_OP(uint64_div);
BINARY_OP(uint8_mod);
BINARY_OP(uint16_mod);
BINARY_OP(uint32_mod);
BINARY_OP(uint64_mod);
BINARY_OP(int8_and);
BINARY_OP(int16_and);
BINARY_OP(int32_and);
BINARY_OP(int64_and);
BINARY_OP(int8_or);
BINARY_OP(int16_or);
BINARY_OP(int32_or);
BINARY_OP(int64_or);
BINARY_OP(int8_xor);
BINARY_OP(int16_xor);
BINARY_OP(int32_xor);
BINARY_OP(int64_xor);
BINARY_OP(int8_lshift);
BINARY_OP(int16_lshift);
BINARY_OP(int32_lshift);
BINARY_OP(int64_lshift);
BINARY_OP(int8_rshift);
BINARY_OP(int16_rshift);
BINARY_OP(int32_rshift);
BINARY_OP(int64_rshift);
BINARY_OP(int8_arshift);
BINARY_OP(int16_arshift);
BINARY_OP(int32_arshift);
BINARY_OP(int64_arshift);
BINARY_OP(int8_equals);
BINARY_OP(int16_equals);
BINARY_OP(int32_equals);
BINARY_OP(int64_equals);
BINARY_OP(int8_greater);
BINARY_OP(int16_greater);
BINARY_OP(int32_greater);
BINARY_OP(int64_greater);
BINARY_OP(int8_lesser);
BINARY_OP(int16_lesser);
BINARY_OP(int32_lesser);
BINARY_OP(int64_lesser);
BINARY_OP(int8_above);
BINARY_OP(int16_above);
BINARY_OP(int32_above);
BINARY_OP(int64_above);
BINARY_OP(int8_below);
BINARY_OP(int16_below);
BINARY_OP(int32_below);
BINARY_OP(int64_below);
BINARY_OP(int8_bool_and);
BINARY_OP(int16_bool_and);
BINARY_OP(int32_bool_and);
BINARY_OP(int64_bool_and);
BINARY_OP(int8_bool_or);
BINARY_OP(int16_bool_or);
BINARY_OP(int32_bool_or);
BINARY_OP(int64_bool_or);

BINARY_OP(vararg_copy);

BINARY_OP(float32_add);
BINARY_OP(float32_sub);
BINARY_OP(float32_mul);
BINARY_OP(float32_div);
BINARY_OP(float64_add);
BINARY_OP(float64_sub);
BINARY_OP(float64_mul);
BINARY_OP(float64_div);

BINARY_OP(float32_equals);
BINARY_OP(float32_greater);
BINARY_OP(float32_lesser);
BINARY_OP(float64_equals);
BINARY_OP(float64_greater);
BINARY_OP(float64_lesser);
BINARY_OP(long_double_equals);
BINARY_OP(long_double_greater);
BINARY_OP(long_double_lesser);

BINARY_OP(long_double_add);
BINARY_OP(long_double_sub);
BINARY_OP(long_double_mul);
BINARY_OP(long_double_div);

BINARY_OP(complex_float32_from);
BINARY_OP(complex_float64_from);
BINARY_OP(complex_long_double_from);

BINARY_OP(complex_float32_equals);
BINARY_OP(complex_float64_equals);
BINARY_OP(complex_long_double_equals);

BINARY_OP(complex_float32_add);
BINARY_OP(complex_float64_add);
BINARY_OP(complex_long_double_add);
BINARY_OP(complex_float32_sub);
BINARY_OP(complex_float64_sub);
BINARY_OP(complex_long_double_sub);
BINARY_OP(complex_float32_mul);
BINARY_OP(complex_float64_mul);
BINARY_OP(complex_long_double_mul);
BINARY_OP(complex_float32_div);
BINARY_OP(complex_float64_div);
BINARY_OP(complex_long_double_div);

#undef BINARY_OP

#define ATOMIC_LOAD_OP(_id)                                                                            \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *, \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,     \
                                                kefir_opt_memory_order_t, kefir_opt_instruction_ref_t *)
ATOMIC_LOAD_OP(atomic_load8);
ATOMIC_LOAD_OP(atomic_load16);
ATOMIC_LOAD_OP(atomic_load32);
ATOMIC_LOAD_OP(atomic_load64);
ATOMIC_LOAD_OP(atomic_load_long_double);
ATOMIC_LOAD_OP(atomic_load_complex_float32);
ATOMIC_LOAD_OP(atomic_load_complex_float64);
ATOMIC_LOAD_OP(atomic_load_complex_long_double);

#undef ATOMIC_LOAD_OP

#define ATOMIC_STORE_OP(_id)                                                                                      \
    kefir_result_t kefir_opt_code_builder_##_id(                                                                  \
        struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t, kefir_opt_instruction_ref_t, \
        kefir_opt_instruction_ref_t, kefir_opt_memory_order_t, kefir_opt_instruction_ref_t *)
ATOMIC_STORE_OP(atomic_store8);
ATOMIC_STORE_OP(atomic_store16);
ATOMIC_STORE_OP(atomic_store32);
ATOMIC_STORE_OP(atomic_store64);
ATOMIC_STORE_OP(atomic_store_long_double);

#undef ATOMIC_STORE_OP

#define ATOMIC_CMPXCHG_OP(_id)                                                                            \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *,    \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,        \
                                                kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t, \
                                                kefir_opt_memory_order_t, kefir_opt_instruction_ref_t *)

ATOMIC_CMPXCHG_OP(atomic_compare_exchange8);
ATOMIC_CMPXCHG_OP(atomic_compare_exchange16);
ATOMIC_CMPXCHG_OP(atomic_compare_exchange32);
ATOMIC_CMPXCHG_OP(atomic_compare_exchange64);
ATOMIC_CMPXCHG_OP(atomic_compare_exchange_long_double);
ATOMIC_CMPXCHG_OP(atomic_compare_exchange_complex_long_double);

#undef ATOMIC_CMPXCHG_OP

#define LOAD_OP(_id)                                                                                              \
    kefir_result_t kefir_opt_code_builder_##_id(                                                                  \
        struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t, kefir_opt_instruction_ref_t, \
        const struct kefir_opt_memory_access_flags *, kefir_opt_instruction_ref_t *)

LOAD_OP(int8_load_signed);
LOAD_OP(int8_load_unsigned);
LOAD_OP(int16_load_signed);
LOAD_OP(int16_load_unsigned);
LOAD_OP(int32_load_signed);
LOAD_OP(int32_load_unsigned);
LOAD_OP(int64_load);
LOAD_OP(long_double_load);

LOAD_OP(complex_float32_load);
LOAD_OP(complex_float64_load);
LOAD_OP(complex_long_double_load);

#undef LOAD_OP

#define STORE_OP(_id)                                                                                             \
    kefir_result_t kefir_opt_code_builder_##_id(                                                                  \
        struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t, kefir_opt_instruction_ref_t, \
        kefir_opt_instruction_ref_t, const struct kefir_opt_memory_access_flags *, kefir_opt_instruction_ref_t *)

STORE_OP(int8_store);
STORE_OP(int16_store);
STORE_OP(int32_store);
STORE_OP(int64_store);
STORE_OP(long_double_store);

STORE_OP(complex_float32_store);
STORE_OP(complex_float64_store);
STORE_OP(complex_long_double_store);

#undef STORE_OP

#define OVERFLOW_ARITH(_id)                                                                                       \
    kefir_result_t kefir_opt_code_builder_##_id##_overflow(                                                       \
        struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_block_id_t, kefir_opt_instruction_ref_t, \
        kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t, kefir_id_t, kefir_size_t, kefir_uint8_t,        \
        kefir_opt_instruction_ref_t *)

OVERFLOW_ARITH(add);
OVERFLOW_ARITH(sub);
OVERFLOW_ARITH(mul);

#undef OVERFLOW_ARITH

#endif
