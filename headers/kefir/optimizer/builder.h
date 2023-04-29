/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

kefir_result_t kefir_opt_code_builder_is_finalized(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                                   kefir_bool_t *);

kefir_result_t kefir_opt_code_builder_finalize_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                    kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                    kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_indirect_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                             kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                             kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_branch(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                      kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_return(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_instruction_ref_t *);

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
kefir_result_t kefir_opt_code_builder_string_reference(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_id_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_block_label(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                  kefir_opt_instruction_ref_t *);

#define UNARY_OP(_id)                                                                                  \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *, \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,     \
                                                kefir_opt_instruction_ref_t *)

UNARY_OP(int_not);
UNARY_OP(int_neg);
UNARY_OP(bool_not);

UNARY_OP(int64_zero_extend_1bit);
UNARY_OP(int64_sign_extend_8bits);
UNARY_OP(int64_sign_extend_16bits);
UNARY_OP(int64_sign_extend_32bits);

#undef UNARY_OP

#define BINARY_OP(_id)                                                                                 \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *, struct kefir_opt_code_container *, \
                                                kefir_opt_block_id_t, kefir_opt_instruction_ref_t,     \
                                                kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t *)

BINARY_OP(int_add);
BINARY_OP(int_sub);
BINARY_OP(int_mul);
BINARY_OP(int_div);
BINARY_OP(int_mod);
BINARY_OP(uint_div);
BINARY_OP(uint_mod);
BINARY_OP(int_and);
BINARY_OP(int_or);
BINARY_OP(int_xor);
BINARY_OP(int_lshift);
BINARY_OP(int_rshift);
BINARY_OP(int_arshift);
BINARY_OP(int_equals);
BINARY_OP(int_greater);
BINARY_OP(int_lesser);
BINARY_OP(int_above);
BINARY_OP(int_below);
BINARY_OP(bool_and);
BINARY_OP(bool_or);

#undef BINARY_OP

#endif
