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
    struct kefir_opt_phi_node *phi = NULL;
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

        struct kefir_codegen_opt_amd64_sysv_storage_location source_location;
        struct kefir_codegen_opt_amd64_sysv_storage_location target_location;

        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
            &source_location, &codegen_func->stack_frame_map, source_allocation));
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
            &target_location, &codegen_func->stack_frame_map, target_allocation));
        REQUIRE_OK(
            kefir_codegen_opt_amd64_sysv_storage_transform_insert(mem, transform, &target_location, &source_location));
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_map_registers(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, const struct kefir_opt_function *function,
    const struct kefir_opt_code_analysis *func_analysis, struct kefir_opt_sysv_amd64_function *codegen_func,
    kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id) {
    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));

    kefir_result_t res =
        map_registers_prepare(mem, function, func_analysis, codegen_func, source_block_id, target_block_id, &transform);
    REQUIRE_CHAIN(&res, kefir_codegen_opt_amd64_sysv_storage_transform_perform(
                            mem, codegen, &codegen_func->storage, &codegen_func->stack_frame_map, &transform));
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
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(mem, codegen, function, func_analysis, codegen_func,
                                                                  instr->block_id,
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

            struct kefir_codegen_opt_amd64_sysv_storage_handle condition_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
                    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                condition_allocation, &condition_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, condition_allocation, &condition_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(condition_handle.location.reg),
                kefir_asm_amd64_xasmgen_operand_reg(condition_handle.location.reg)));

            kefir_bool_t has_mapped_regs;
            kefir_id_t alternative_label;
            REQUIRE_OK(has_mapped_registers(mem, function, func_analysis, codegen_func, instr->block_id,
                                            instr->operation.parameters.branch.alternative_block, &has_mapped_regs));
            kefir_bool_t separate_alternative_jmp =
                has_mapped_regs || KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(&condition_handle);

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
                    kefir_codegen_opt_amd64_sysv_storage_handle_restore_evicted(&codegen->xasmgen, &condition_handle));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JZ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.alternative_block))));
            }

            REQUIRE_OK(
                kefir_codegen_opt_amd64_sysv_storage_handle_restore_evicted(&codegen->xasmgen, &condition_handle));
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(mem, codegen, function, func_analysis, codegen_func,
                                                                  instr->block_id,
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
                    kefir_codegen_opt_amd64_sysv_storage_handle_restore_evicted(&codegen->xasmgen, &condition_handle));
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(
                    mem, codegen, function, func_analysis, codegen_func, instr->block_id,
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

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_handle_mask_evicted(&codegen->xasmgen, &condition_handle));
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &condition_handle));
        } break;

        case KEFIR_OPT_OPCODE_COMPARE_BRANCH: {
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                &codegen_func->register_allocator, instr->operation.parameters.branch.comparison.refs[0],
                &arg1_allocation));
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                &codegen_func->register_allocator, instr->operation.parameters.branch.comparison.refs[1],
                &arg2_allocation));

            struct kefir_codegen_opt_amd64_sysv_storage_handle arg1_handle;
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg2_handle;

            switch (instr->operation.parameters.branch.comparison.type) {
                case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER:
                case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_LESS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE:
                case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW:
                case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS:
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg1_allocation, &arg1_handle, NULL, NULL));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg1_allocation, &arg1_handle.location));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg1_handle.location.reg),
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, arg2_allocation)));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_NOT_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER_OR_EQUALS:
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg1_allocation, &arg1_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
                        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg2_allocation, &arg2_handle, NULL, NULL));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg1_allocation, &arg1_handle.location));
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg2_allocation, &arg2_handle.location));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_UCOMISS(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg1_handle.location.reg),
                        kefir_asm_amd64_xasmgen_operand_reg(arg2_handle.location.reg)));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen,
                                                                            &codegen_func->storage, &arg2_handle));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_NOT_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER_OR_EQUALS:
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg1_allocation, &arg1_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
                        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg2_allocation, &arg2_handle, NULL, NULL));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg1_allocation, &arg1_handle.location));
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg2_allocation, &arg2_handle.location));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_UCOMISD(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg1_handle.location.reg),
                        kefir_asm_amd64_xasmgen_operand_reg(arg2_handle.location.reg)));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen,
                                                                            &codegen_func->storage, &arg2_handle));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS_OR_EQUALS:
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg1_allocation, &arg1_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
                        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg2_allocation, &arg2_handle, NULL, NULL));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg1_allocation, &arg1_handle.location));
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg2_allocation, &arg2_handle.location));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_UCOMISS(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg2_handle.location.reg),
                        kefir_asm_amd64_xasmgen_operand_reg(arg1_handle.location.reg)));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen,
                                                                            &codegen_func->storage, &arg2_handle));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS_OR_EQUALS:
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg1_allocation, &arg1_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
                        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
                        arg2_allocation, &arg2_handle, NULL, NULL));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg1_allocation, &arg1_handle.location));
                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                        &codegen->xasmgen, &codegen_func->stack_frame_map, arg2_allocation, &arg2_handle.location));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_UCOMISD(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg2_handle.location.reg),
                        kefir_asm_amd64_xasmgen_operand_reg(arg1_handle.location.reg)));

                    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen,
                                                                            &codegen_func->storage, &arg2_handle));
                    break;
            }

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg1_handle));

            kefir_bool_t has_mapped_regs;
            kefir_id_t alternative_label;
            REQUIRE_OK(has_mapped_registers(mem, function, func_analysis, codegen_func, instr->block_id,
                                            instr->operation.parameters.branch.alternative_block, &has_mapped_regs));

            char buffer[256];
            struct kefir_asm_amd64_xasmgen_operand alternative_operand;
            const struct kefir_asm_amd64_xasmgen_operand *alternative_oper = NULL;
            if (has_mapped_regs) {
                alternative_label = codegen_func->nonblock_labels++;
                snprintf(buffer, sizeof(buffer), KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL, function->ir_func->name,
                         instr->block_id, alternative_label);
                alternative_oper = kefir_asm_amd64_xasmgen_operand_label(&alternative_operand, buffer);
            } else {
                snprintf(buffer, sizeof(buffer), KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                         instr->operation.parameters.branch.alternative_block);
                alternative_oper = kefir_asm_amd64_xasmgen_operand_label(&alternative_operand, buffer);
            }

            switch (instr->operation.parameters.branch.comparison.type) {
                case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JNE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JLE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JL(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_LESS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JGE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JG(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JBE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JB(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JAE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JA(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JP(&codegen->xasmgen, alternative_oper));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JNE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_NOT_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_NOT_EQUALS: {
                    kefir_id_t label = codegen_func->nonblock_labels++;
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JP(
                        &codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                                   KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                                   function->ir_func->name, instr->block_id, label))));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JE(&codegen->xasmgen, alternative_oper));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen,
                                                         KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                         function->ir_func->name, instr->block_id, label));
                } break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JBE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER_OR_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JB(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JBE(&codegen->xasmgen, alternative_oper));
                    break;

                case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS_OR_EQUALS:
                case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS_OR_EQUALS:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JB(&codegen->xasmgen, alternative_oper));
                    break;
            }

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(mem, codegen, function, func_analysis, codegen_func,
                                                                  instr->block_id,
                                                                  instr->operation.parameters.branch.target_block));

            const struct kefir_opt_code_analysis_block_properties *source_block_props =
                &func_analysis->blocks[instr->block_id];
            const struct kefir_opt_code_analysis_block_properties *target_block_props =
                &func_analysis->blocks[instr->operation.parameters.branch.target_block];

            if (has_mapped_regs || target_block_props->linear_position != source_block_props->linear_position + 1) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.target_block))));
            }

            if (has_mapped_regs) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                     function->ir_func->name, instr->block_id, alternative_label));

                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(
                    mem, codegen, function, func_analysis, codegen_func, instr->block_id,
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
