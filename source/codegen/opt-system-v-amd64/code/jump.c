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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/codegen/opt-system-v-amd64/storage_transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t map_registers_prepare(struct kefir_mem *mem, const struct kefir_opt_function *function,
                                            const struct kefir_opt_code_analysis *func_analysis,
                                            struct kefir_opt_sysv_amd64_function *codegen_func,
                                            kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id,
                                            struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    struct kefir_opt_code_block *target_block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(&function->code, target_block_id, &target_block));

    kefir_result_t res;
    const struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(&function->code, target_block, &phi); res == KEFIR_OK && phi != NULL;
         res = kefir_opt_phi_next_sibling(&function->code, phi, &phi)) {

        if (!func_analysis->instructions[phi->output_ref].reachable) {
            continue;
        }

        kefir_opt_instruction_ref_t source_ref;
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&function->code, phi->node_id, source_block_id, &source_ref));

        const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;

        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, source_ref,
                                                                       &source_allocation));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator,
                                                                       phi->output_ref, &target_allocation));

        struct kefir_codegen_opt_amd64_sysv_storage_transform_location source_location;
        struct kefir_codegen_opt_amd64_sysv_storage_transform_location target_location;

        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
            &source_location, &codegen_func->stack_frame_map, source_allocation));
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
            &target_location, &codegen_func->stack_frame_map, target_allocation));
        REQUIRE_OK(
            kefir_codegen_opt_amd64_sysv_storage_transform_insert(mem, transform, &target_location, &source_location));
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t map_registers(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                    const struct kefir_opt_function *function,
                                    const struct kefir_opt_code_analysis *func_analysis,
                                    struct kefir_opt_sysv_amd64_function *codegen_func,
                                    kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id) {
    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));

    kefir_result_t res =
        map_registers_prepare(mem, function, func_analysis, codegen_func, source_block_id, target_block_id, &transform);
    REQUIRE_CHAIN(
        &res, kefir_codegen_opt_amd64_sysv_storage_transform_perform(mem, codegen, &codegen_func->storage, &transform));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));
    return KEFIR_OK;
}

static kefir_result_t has_mapped_registers(struct kefir_mem *mem, const struct kefir_opt_function *function,
                                           const struct kefir_opt_code_analysis *func_analysis,
                                           struct kefir_opt_sysv_amd64_function *codegen_func,
                                           kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id,
                                           kefir_bool_t *result) {
    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));

    kefir_size_t num_of_transforms;
    kefir_result_t res =
        map_registers_prepare(mem, function, func_analysis, codegen_func, source_block_id, target_block_id, &transform);
    REQUIRE_CHAIN(&res, kefir_codegen_opt_amd64_sysv_storage_transform_operations(&transform, &num_of_transforms));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));

    *result = num_of_transforms != 0;
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(jump) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(map_registers(mem, codegen, function, func_analysis, codegen_func, instr->block_id,
                                     instr->operation.parameters.branch.target_block));

            const struct kefir_opt_code_analysis_block_properties *source_block_props =
                &func_analysis->blocks[instr->block_id];
            const struct kefir_opt_code_analysis_block_properties *target_block_props =
                &func_analysis->blocks[instr->operation.parameters.branch.target_block];

            if (target_block_props->linear_position != source_block_props->linear_position + 1) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.target_block))));
            }
            break;

        case KEFIR_OPT_OPCODE_BRANCH: {
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *condition_allocation = NULL;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                &codegen_func->register_allocator, instr->operation.parameters.branch.condition_ref,
                &condition_allocation));

            struct kefir_codegen_opt_sysv_amd64_storage_register condition_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_shared_allocated_register(
                mem, &codegen->xasmgen, &codegen_func->storage, condition_allocation, &condition_reg, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        condition_allocation, condition_reg.reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(condition_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(condition_reg.reg)));

            kefir_bool_t has_mapped_regs;
            kefir_id_t alternative_label;
            REQUIRE_OK(has_mapped_registers(mem, function, func_analysis, codegen_func, instr->block_id,
                                            instr->operation.parameters.branch.alternative_block, &has_mapped_regs));
            kefir_bool_t separate_alternative_jmp = has_mapped_regs || condition_reg.evicted;

            if (separate_alternative_jmp) {
                alternative_label = codegen_func->nonblock_labels++;
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JZ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                           &codegen->xasmgen_helpers.operands[0],
                                           kefir_asm_amd64_xasmgen_helpers_format(
                                               &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                               function->ir_func->name, instr->block_id, alternative_label))));
            } else {
                REQUIRE_OK(
                    kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(&codegen->xasmgen, &condition_reg));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JZ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.alternative_block))));
            }

            REQUIRE_OK(
                kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(&codegen->xasmgen, &condition_reg));
            REQUIRE_OK(map_registers(mem, codegen, function, func_analysis, codegen_func, instr->block_id,
                                     instr->operation.parameters.branch.target_block));

            const struct kefir_opt_code_analysis_block_properties *source_block_props =
                &func_analysis->blocks[instr->block_id];
            const struct kefir_opt_code_analysis_block_properties *target_block_props =
                &func_analysis->blocks[instr->operation.parameters.branch.target_block];

            if (separate_alternative_jmp ||
                target_block_props->linear_position != source_block_props->linear_position + 1) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.target_block))));
            }

            if (separate_alternative_jmp) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                     function->ir_func->name, instr->block_id, alternative_label));

                REQUIRE_OK(
                    kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(&codegen->xasmgen, &condition_reg));
                REQUIRE_OK(map_registers(mem, codegen, function, func_analysis, codegen_func, instr->block_id,
                                         instr->operation.parameters.branch.alternative_block));

                const struct kefir_opt_code_analysis_block_properties *alternative_block_props =
                    &func_analysis->blocks[instr->operation.parameters.branch.alternative_block];

                if (alternative_block_props->linear_position != source_block_props->linear_position + 1) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                        &codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[0],
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK,
                                function->ir_func->name, instr->operation.parameters.branch.alternative_block))));
                }
            }

            condition_reg.evicted = false;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen,
                                                                             &codegen_func->storage, &condition_reg));
        } break;

        case KEFIR_OPT_OPCODE_IJUMP: {
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                &codegen_func->register_allocator, instr->operation.parameters.refs[0], &target_allocation));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                    &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, target_allocation)));
        } break;

        default:
            break;
    }
    return KEFIR_OK;
}
