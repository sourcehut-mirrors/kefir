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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t generate_slot_to_local_var(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_ref,
                                                 kefir_opt_instruction_ref_t insert_after_ref,
                                                 kefir_opt_instruction_ref_t slot_ref,
                                                 kefir_opt_instruction_ref_t location_ref) {
    kefir_opt_instruction_ref_t location_write_ref, slot_read_ref;
    REQUIRE_OK(kefir_opt_code_builder_int_slot_read(mem, code, block_ref, slot_ref, &slot_read_ref));
    REQUIRE_OK(kefir_opt_code_container_insert_control(code, block_ref, insert_after_ref, slot_read_ref));
    REQUIRE_OK(kefir_opt_code_builder_int64_store(mem, code, block_ref, location_ref, slot_read_ref,
                                                  &(struct kefir_opt_memory_access_flags) {0}, &location_write_ref));
    REQUIRE_OK(kefir_opt_code_container_insert_control(code, block_ref, slot_read_ref, location_write_ref));
    return KEFIR_OK;
}

static kefir_result_t process_slot(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                   const struct kefir_opt_inline_assembly_node *inline_asm_node,
                                   kefir_opt_instruction_ref_t location_ref, kefir_bool_t integer,
                                   kefir_opt_instruction_ref_t *slot_ref) {
    if (integer) {
        REQUIRE_OK(kefir_opt_code_builder_int_slot(mem, code, inline_asm_node->block_id, slot_ref));
    } else {
        REQUIRE_OK(kefir_opt_code_builder_float_slot(mem, code, inline_asm_node->block_id, slot_ref));
    }

    kefir_opt_instruction_ref_t location_read_ref, slot_write_ref, insert_after_ref;
    REQUIRE_OK(kefir_opt_instruction_prev_control(code, inline_asm_node->output_ref, &insert_after_ref));
    REQUIRE_OK(kefir_opt_code_builder_int64_load(mem, code, inline_asm_node->block_id, location_ref,
                                                 &(struct kefir_opt_memory_access_flags) {0}, &location_read_ref));
    REQUIRE_OK(
        kefir_opt_code_container_insert_control(code, inline_asm_node->block_id, insert_after_ref, location_read_ref));
    REQUIRE_OK(kefir_opt_code_builder_int_slot_write(mem, code, inline_asm_node->block_id, *slot_ref, location_read_ref,
                                                     &slot_write_ref));
    REQUIRE_OK(
        kefir_opt_code_container_insert_control(code, inline_asm_node->block_id, location_read_ref, slot_write_ref));

    if (inline_asm_node->default_jump_target != KEFIR_ID_NONE) {
        REQUIRE_OK(generate_slot_to_local_var(mem, code, inline_asm_node->default_jump_target, KEFIR_ID_NONE, *slot_ref,
                                              location_ref));
        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm_node->jump_targets, &iter);
             node != NULL; node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
            REQUIRE_OK(generate_slot_to_local_var(mem, code, target_block, KEFIR_ID_NONE, *slot_ref, location_ref));
        }

    } else {
        REQUIRE_OK(generate_slot_to_local_var(mem, code, inline_asm_node->block_id, inline_asm_node->output_ref,
                                              *slot_ref, location_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t inline_asm_impl(struct kefir_mem *mem, const struct kefir_ir_module *ir_module,
                                      struct kefir_opt_code_container *code,
                                      kefir_opt_instruction_ref_t inline_asm_instr_ref) {
    UNUSED(mem);

    const struct kefir_opt_instruction *inline_asm_instr;
    const struct kefir_opt_inline_assembly_node *inline_asm_node;
    REQUIRE_OK(kefir_opt_code_container_instr(code, inline_asm_instr_ref, &inline_asm_instr));
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, inline_asm_instr->operation.parameters.inline_asm_ref,
                                                        &inline_asm_node));
    const struct kefir_ir_inline_assembly *ir_inline_asm =
        kefir_ir_module_get_inline_assembly(ir_module, inline_asm_node->inline_asm_id);
    REQUIRE(ir_inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR inline assembly"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&ir_inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);
        const struct kefir_opt_inline_assembly_parameter *parameter;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_get_parameter(code, inline_asm_instr_ref,
                                                                          ir_asm_param->parameter_id, &parameter));

        if (!(ir_asm_param->constraints.general_purpose_register ||
              ir_asm_param->constraints.floating_point_register) ||
            ir_asm_param->constraints.memory_location || parameter->location_ref == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_opt_instruction *location_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, parameter->location_ref, &location_instr));
        if (location_instr->operation.opcode != KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
            continue;
        }
        const struct kefir_ir_type *alloc_type =
            kefir_ir_module_get_named_type(ir_module, location_instr->operation.parameters.type.type_id);
        REQUIRE(alloc_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type"));

        kefir_bool_t same_type;
        REQUIRE_OK(kefir_ir_type_same(alloc_type, location_instr->operation.parameters.type.type_index,
                                      ir_asm_param->location_type.type, ir_asm_param->location_type.index, &same_type));
        if (!same_type) {
            continue;
        }

        const struct kefir_ir_typeentry *typeentry =
            kefir_ir_type_at(alloc_type, location_instr->operation.parameters.type.type_index);
        if (typeentry == NULL) {
            continue;
        }

        switch (typeentry->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64: {
                kefir_opt_instruction_ref_t slot_ref = KEFIR_ID_NONE;
                REQUIRE_OK(process_slot(mem, code, inline_asm_node, parameter->location_ref, true, &slot_ref));

                struct kefir_opt_inline_assembly_parameter new_parameter = {.value_ref = parameter->value_ref,
                                                                            .location_ref = slot_ref};
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
                    mem, code, inline_asm_instr_ref, ir_asm_param->parameter_id, &new_parameter));
            } break;

            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64: {
                kefir_opt_instruction_ref_t slot_ref = KEFIR_ID_NONE;
                REQUIRE_OK(process_slot(mem, code, inline_asm_node, parameter->location_ref, false, &slot_ref));

                struct kefir_opt_inline_assembly_parameter new_parameter = {.value_ref = parameter->value_ref,
                                                                            .location_ref = slot_ref};
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
                    mem, code, inline_asm_instr_ref, ir_asm_param->parameter_id, &new_parameter));
            } break;

            default:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t inline_asm_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_inline_assembly_head(&func->code, block->id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_inline_assembly_next_sibling(&func->code, instr_ref, &instr_ref)) {
            REQUIRE_OK(inline_asm_impl(mem, module->ir_module, &func->code, instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassInlineAsm = {
    .name = "inline-asm", .apply = inline_asm_apply, .payload = NULL};
