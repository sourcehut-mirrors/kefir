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

#include "kefir/codegen/amd64/translator.h"
#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"


struct block_translator {
    kefir_asmcmp_label_index_t begin_label;
    kefir_asmcmp_label_index_t end_label;
};


static kefir_result_t free_type_layout(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                       kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_type_layout *, type_layout, value);
    REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 type layout"));

    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, type_layout));
    KEFIR_FREE(mem, type_layout);
    return KEFIR_OK;
}

static kefir_result_t free_block(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                       kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct block_translator *, block, value);
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function block translator"));

    KEFIR_FREE(mem, block);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_state_init(struct kefir_codegen_amd64_function_translator_state *state,
    struct kefir_codegen_target_ir_code_constructor_metadata *metadata) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 codegen function translator state"));
    REQUIRE(metadata != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR metadata"));

    state->target_ir_metadata = metadata;

    state->argument_touch_instr = KEFIR_ASMCMP_INDEX_NONE,
    state->prologue_tail = KEFIR_ASMCMP_INDEX_NONE,
    state->dynamic_scope_vreg = KEFIR_ASMCMP_INDEX_NONE,
    state->vararg_area = KEFIR_ASMCMP_INDEX_NONE;

    REQUIRE_OK(kefir_hashtree_init(&state->constants, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state->blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&state->blocks, free_block, NULL));
    REQUIRE_OK(kefir_hashtable_init(&state->type_layouts, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&state->type_layouts, free_type_layout, NULL));
    REQUIRE_OK(kefir_hashtable_init(&state->virtual_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state->argument_virtual_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state->virtual_registers_alive_at_exit, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state->function_exit_points, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_codegen_amd64_x87_init(&state->x87));
    REQUIRE_OK(kefir_hashtree_init(&state->occupied_x87_stack_slots, &kefir_hashtree_uint_ops));

    state->function_parameters = NULL;
    state->function_parameters_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_state_free(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *state) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator state"));

    REQUIRE_OK(kefir_codegen_amd64_x87_free(mem, &state->x87));
    REQUIRE_OK(kefir_hashset_free(mem, &state->function_exit_points));
    REQUIRE_OK(kefir_hashset_free(mem, &state->virtual_registers_alive_at_exit));
    REQUIRE_OK(kefir_hashtable_free(mem, &state->argument_virtual_registers));
    REQUIRE_OK(kefir_hashtable_free(mem, &state->virtual_registers));
    REQUIRE_OK(kefir_hashtable_free(mem, &state->type_layouts));
    REQUIRE_OK(kefir_hashtable_free(mem, &state->blocks));
    REQUIRE_OK(kefir_hashtree_free(mem, &state->constants));
    REQUIRE_OK(kefir_hashtree_free(mem, &state->occupied_x87_stack_slots));
    KEFIR_FREE(mem, state->function_parameters);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_get_block_label(const struct kefir_codegen_amd64_function_translator_state *translator, kefir_opt_block_id_t block_id, kefir_asmcmp_label_index_t *label_ptr) {
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));
    
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&translator->blocks, (kefir_hashtable_key_t) block_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find amd64 codegen function block");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct block_translator *, block, table_value);
    ASSIGN_PTR(label_ptr, block->begin_label);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_get_block_end_label(const struct kefir_codegen_amd64_function_translator_state *translator, kefir_opt_block_id_t block_id, kefir_asmcmp_label_index_t *label_ptr) {
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));
    
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&translator->blocks, (kefir_hashtable_key_t) block_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find amd64 codegen function block");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct block_translator *, block, table_value);
    ASSIGN_PTR(label_ptr, block->end_label);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_new_block(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_opt_block_id_t block_id, kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));

    struct block_translator *block_translator = KEFIR_MALLOC(mem, sizeof(struct block_translator));
    REQUIRE(block_translator != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 codegen function block"));
    block_translator->begin_label = begin_label;
    block_translator->end_label = end_label;

    kefir_result_t res = kefir_hashtable_insert(mem, &translator->blocks, (kefir_hashtable_key_t) block_id, (kefir_hashtable_value_t) block_translator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, block_translator);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_new_constant(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_asmcmp_label_index_t label, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));

    kefir_result_t res = kefir_hashtree_insert(mem, &translator->constants, (kefir_hashtree_key_t) label, (kefir_hashtree_value_t) instr_ref);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "amd64 codegen function constant with the same label already exists");
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_add_exit_point(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_asmcmp_instruction_index_t instr_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));

    REQUIRE_OK(kefir_hashset_add(mem, &translator->function_exit_points, (kefir_hashset_key_t) instr_idx));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_add_virtual_register_alive_at_exit_point(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_asmcmp_virtual_register_index_t vreg_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));

    REQUIRE_OK(kefir_hashset_add(mem, &translator->virtual_registers_alive_at_exit, (kefir_hashset_key_t) vreg_idx));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_get_argument_register(struct kefir_mem *mem,
                                                         struct kefir_codegen_amd64_function_translator_state *translator,
                                                         struct kefir_asmcmp_amd64 *code,
                                                         kefir_asm_amd64_xasmgen_register_t reg,
                                                         kefir_asmcmp_virtual_register_index_t *vreg_idx_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function translator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp code"));
    REQUIRE(vreg_idx_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp virtual register"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&translator->argument_virtual_registers, (kefir_hashtable_key_t) reg, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *vreg_idx_ptr = (kefir_asmcmp_virtual_register_index_t) table_value;
    } else {
        kefir_asmcmp_virtual_register_index_t arg_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &code->context,
            (kefir_asm_amd64_xasmgen_register_is_floating_point(reg) ? KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT
                                                                     : KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE),
            &arg_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, code, arg_vreg, reg));
        REQUIRE_OK(kefir_asmcmp_amd64_produce_virtual_register(mem, code, translator->argument_touch_instr,
                                                               arg_vreg, &translator->argument_touch_instr));
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, code, translator->argument_touch_instr,
                                                             arg_vreg, &translator->argument_touch_instr));

        REQUIRE_OK(kefir_hashtable_insert(mem, &translator->argument_virtual_registers, (kefir_hashtable_key_t) reg,
                                         (kefir_hashtable_value_t) arg_vreg));
        *vreg_idx_ptr = arg_vreg;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_vreg_of(const struct kefir_codegen_amd64_function_translator_state *translator,
                                                    kefir_opt_instruction_ref_t instr_ref,
                                                    kefir_asmcmp_virtual_register_index_t *vreg) {
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function translator"));
    REQUIRE(vreg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 virtual register index"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&translator->virtual_registers, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find associated virtual register");
    }
    REQUIRE_OK(res);

    *vreg = table_value;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_assign_vreg(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function_translator_state *translator,
                                                        kefir_opt_instruction_ref_t instr_ref,
                                                        kefir_asmcmp_virtual_register_index_t vreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function translator state"));

    kefir_result_t res = kefir_hashtable_insert(mem, &translator->virtual_registers, (kefir_hashtable_key_t) instr_ref,
                                               (kefir_hashtable_value_t) vreg);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Virtual register has already been assigned");
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_codegen_target_ir_code_constructor_metadata_add_value_ref(mem, translator->target_ir_metadata, vreg, (kefir_codegen_target_ir_metadata_value_ref_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_get_local_variable_type_layout(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function_translator_state *translator, const struct kefir_ir_module *module, kefir_abi_amd64_variant_t abi_variant, kefir_id_t type_id,
                                                     const struct kefir_abi_amd64_type_layout **type_layout_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function translator state"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(type_layout_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 type layout"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&translator->type_layouts, (kefir_hashtable_key_t) type_id, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *type_layout_ptr = (const struct kefir_abi_amd64_type_layout *) table_value;
    } else {
        const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(module, type_id);
        REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
        struct kefir_abi_amd64_type_layout *type_layout = KEFIR_MALLOC(mem, sizeof(struct kefir_abi_amd64_type_layout));
        REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 type"));
        kefir_result_t res = kefir_abi_amd64_type_layout(
            mem, abi_variant, KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_STACK, ir_type, type_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, type_layout);
            return res;
        });
        res = kefir_hashtable_insert(mem, &translator->type_layouts, (kefir_hashtable_key_t) type_id,
                                    (kefir_hashtable_value_t) type_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_amd64_type_layout_free(mem, type_layout);
            KEFIR_FREE(mem, type_layout);
            return res;
        });
        *type_layout_ptr = type_layout;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_record_x87_at(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator,
    kefir_opt_instruction_ref_t stack_instr_ref, kefir_opt_instruction_ref_t location_ref, kefir_size_t x87_stack_index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function trasnaltro state"));

    const kefir_hashtree_key_t key =
        (((kefir_uint64_t) stack_instr_ref) << 32) | ((kefir_uint32_t) location_ref);
    REQUIRE_OK(kefir_hashtree_insert(mem, &translator->occupied_x87_stack_slots, key,
                                        (kefir_hashtree_value_t) x87_stack_index));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_x87_locations_iter(
    const struct kefir_codegen_amd64_function_translator_state *translator, kefir_opt_instruction_ref_t instr_ref,
    struct kefir_codegen_amd64_function_x87_locations_iterator *iter, kefir_opt_instruction_ref_t *location_instr_ref,
    kefir_size_t *x87_stack_index) {
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function translator"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid amd64 code generator function x87 locations iterator"));

    kefir_result_t res =
        kefir_hashtree_lower_bound(&translator->occupied_x87_stack_slots,
                                   (kefir_hashtree_key_t) (((kefir_uint64_t) instr_ref) << 32), &iter->node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_min(&translator->occupied_x87_stack_slots, &iter->node));
    } else {
        REQUIRE_OK(res);
    }

    for (; iter->node != NULL;
         iter->node = kefir_hashtree_next_node(&translator->occupied_x87_stack_slots, iter->node)) {
        const kefir_opt_instruction_ref_t stack_instr_ref = ((kefir_uint64_t) iter->node->key) >> 32;
        if (stack_instr_ref == instr_ref) {
            break;
        } else if (stack_instr_ref > instr_ref) {
            iter->node = NULL;
        }
    }

    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
    iter->x87_slots = &translator->occupied_x87_stack_slots;
    iter->instr_ref = instr_ref;
    ASSIGN_PTR(location_instr_ref, ((kefir_uint64_t) iter->node->key) & ((1ull << 32) - 1));
    ASSIGN_PTR(x87_stack_index, iter->node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_x87_locations_next(
    struct kefir_codegen_amd64_function_x87_locations_iterator *iter, kefir_opt_instruction_ref_t *location_instr_ref,
    kefir_size_t *x87_stack_index) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid amd64 code generator function x87 locations iterator"));

    iter->node = kefir_hashtree_next_node(iter->x87_slots, iter->node);
    if (iter->node != NULL) {
        const kefir_opt_instruction_ref_t stack_instr_ref = ((kefir_uint64_t) iter->node->key) >> 32;
        if (stack_instr_ref > iter->instr_ref) {
            iter->node = NULL;
        }
    }

    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(location_instr_ref, ((kefir_uint64_t) iter->node->key) & ((1ull << 32) - 1));
    ASSIGN_PTR(x87_stack_index, iter->node->value);
    return KEFIR_OK;
}

kefir_opt_instruction_ref_t kefir_codegen_amd64_function_translator_get_parameter(const struct kefir_codegen_amd64_function_translator_state *translator, kefir_size_t index) {
    REQUIRE(translator != NULL, KEFIR_ID_NONE);
    REQUIRE(index < translator->function_parameters_length, KEFIR_ID_NONE);

    return translator->function_parameters[index];
}

kefir_result_t kefir_codegen_amd64_function_translator_set_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_size_t index, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function debug state"));

    if (translator->function_parameters_length <= index) {
        kefir_size_t new_length = index + 1;
        kefir_opt_instruction_ref_t *new_params = KEFIR_REALLOC(mem, translator->function_parameters, sizeof(kefir_opt_instruction_ref_t) * new_length);
        REQUIRE(new_params != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 codegen function parameters"));

        for (kefir_size_t i = translator->function_parameters_length; i < new_length; i++) {
            new_params[i] = KEFIR_ID_NONE;
        }

        translator->function_parameters = new_params;
        translator->function_parameters_length = new_length;
    }

    translator->function_parameters[index] = instr_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translator_record_value_ref(struct kefir_mem *mem, struct kefir_codegen_amd64_function_translator_state *translator, kefir_asmcmp_virtual_register_index_t vreg, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function translator state"));

    REQUIRE_OK(kefir_codegen_target_ir_code_constructor_metadata_add_value_ref(mem, translator->target_ir_metadata, vreg, (kefir_codegen_target_ir_metadata_value_ref_t) instr_ref));
    return KEFIR_OK;
}

