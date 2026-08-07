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

#ifndef KEFIR_CODEGEN_AMD64_TRANSLATOR_H_
#define KEFIR_CODEGEN_AMD64_TRANSLATOR_H_

#include "kefir/core/hashset.h"
#include "kefir/core/hashtree.h"
#include "kefir/optimizer/code.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/x87.h"

typedef struct kefir_codegen_target_ir_code_constructor_metadata kefir_codegen_target_ir_code_constructor_metadata_t;

typedef struct kefir_codegen_amd64_function_translator_state {
    struct kefir_hashtree constants;
    struct kefir_hashtable blocks;
    struct kefir_hashtable type_layouts;
    struct kefir_hashtable virtual_registers;
    struct kefir_hashtable argument_virtual_registers;
    struct kefir_hashset virtual_registers_alive_at_exit;
    struct kefir_hashset function_exit_points;
    struct kefir_codegen_amd64_x87 x87;

    kefir_asmcmp_instruction_index_t argument_touch_instr;
    kefir_asmcmp_instruction_index_t prologue_tail;
    kefir_asmcmp_virtual_register_index_t dynamic_scope_vreg;
    kefir_asmcmp_virtual_register_index_t vararg_area;

    kefir_opt_instruction_ref_t *function_parameters;
    kefir_size_t function_parameters_length;
    struct kefir_hashtree occupied_x87_stack_slots;

    struct kefir_codegen_target_ir_code_constructor_metadata *target_ir_metadata;
} kefir_codegen_amd64_function_translator_state_t;

kefir_result_t kefir_codegen_amd64_function_translator_state_init(struct kefir_codegen_amd64_function_translator_state *, struct kefir_codegen_target_ir_code_constructor_metadata *);
kefir_result_t kefir_codegen_amd64_function_translator_state_free(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *);

kefir_result_t kefir_codegen_amd64_function_translator_get_block_label(const struct kefir_codegen_amd64_function_translator_state *, kefir_opt_block_id_t, kefir_asmcmp_label_index_t *);
kefir_result_t kefir_codegen_amd64_function_translator_get_block_end_label(const struct kefir_codegen_amd64_function_translator_state *, kefir_opt_block_id_t, kefir_asmcmp_label_index_t *);
kefir_result_t kefir_codegen_amd64_function_translator_new_block(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_opt_block_id_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t);
kefir_result_t kefir_codegen_amd64_function_translator_new_constant(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_asmcmp_label_index_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_translator_add_exit_point(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_asmcmp_instruction_index_t);
kefir_result_t kefir_codegen_amd64_function_translator_add_virtual_register_alive_at_exit_point(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_asmcmp_virtual_register_index_t);
kefir_result_t kefir_codegen_amd64_function_translator_get_argument_register(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asm_amd64_xasmgen_register_t,
                                                         kefir_asmcmp_virtual_register_index_t *);

kefir_result_t kefir_codegen_amd64_function_translator_vreg_of(const struct kefir_codegen_amd64_function_translator_state *, kefir_opt_instruction_ref_t,
                                                    kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_codegen_amd64_function_translator_assign_vreg(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *,
                                                        kefir_opt_instruction_ref_t,
                                                        kefir_asmcmp_virtual_register_index_t);


kefir_result_t kefir_codegen_amd64_function_translator_get_local_variable_type_layout(struct kefir_mem *,
                                                     struct kefir_codegen_amd64_function_translator_state *, const struct kefir_ir_module *, kefir_abi_amd64_variant_t, kefir_id_t,
                                                     const struct kefir_abi_amd64_type_layout **);

kefir_result_t kefir_codegen_amd64_function_translator_record_x87_at(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *,
    kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t, kefir_size_t);

typedef struct kefir_codegen_amd64_function_x87_locations_iterator {
    const struct kefir_hashtree *x87_slots;
    struct kefir_hashtree_node *node;
    kefir_opt_instruction_ref_t instr_ref;
} kefir_codegen_amd64_function_x87_locations_iterator_t;

kefir_result_t kefir_codegen_amd64_function_translator_x87_locations_iter(
    const struct kefir_codegen_amd64_function_translator_state *, kefir_opt_instruction_ref_t,
    struct kefir_codegen_amd64_function_x87_locations_iterator *, kefir_opt_instruction_ref_t *, kefir_size_t *);
kefir_result_t kefir_codegen_amd64_function_translator_x87_locations_next(
    struct kefir_codegen_amd64_function_x87_locations_iterator *, kefir_opt_instruction_ref_t *, kefir_size_t *);

kefir_opt_instruction_ref_t kefir_codegen_amd64_function_translator_get_parameter(const struct kefir_codegen_amd64_function_translator_state *, kefir_size_t);
kefir_result_t kefir_codegen_amd64_function_translator_set_parameter(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_size_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_translator_record_value_ref(struct kefir_mem *, struct kefir_codegen_amd64_function_translator_state *, kefir_asmcmp_virtual_register_index_t, kefir_opt_instruction_ref_t);

#endif
