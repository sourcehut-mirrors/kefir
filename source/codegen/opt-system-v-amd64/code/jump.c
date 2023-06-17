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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t reg_allocation_index(struct kefir_opt_sysv_amd64_function *codegen_func,
                                           const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_alloc,
                                           kefir_size_t *index) {
    switch (reg_alloc->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
            for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
                if (KefirOptSysvAmd64GeneralPurposeRegisters[i] == reg_alloc->result.reg) {
                    *index = i;
                    return KEFIR_OK;
                }
            }
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected register allocation");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfFloatingPointRegisters; i++) {
                if (KefirOptSysvAmd64FloatingPointRegisters[i] == reg_alloc->result.reg) {
                    *index = i + KefirOptSysvAmd64NumOfGeneralPurposeRegisters;
                    return KEFIR_OK;
                }
            }
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected register allocation");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            *index = reg_alloc->result.spill_index + KefirOptSysvAmd64NumOfGeneralPurposeRegisters +
                     KefirOptSysvAmd64NumOfFloatingPointRegisters;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
            *index = reg_alloc->result.register_aggregate.index + codegen_func->stack_frame.spill_area_size +
                     KefirOptSysvAmd64NumOfGeneralPurposeRegisters + KefirOptSysvAmd64NumOfFloatingPointRegisters;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            *index = (((kefir_size_t) reg_alloc->result.indirect.base_register) << 16) +
                     reg_alloc->result.indirect.offset + codegen_func->stack_frame.register_aggregate_area_size +
                     codegen_func->stack_frame.spill_area_size + KefirOptSysvAmd64NumOfGeneralPurposeRegisters +
                     KefirOptSysvAmd64NumOfFloatingPointRegisters;
            break;
    }
    return KEFIR_OK;
}

struct register_mapping {
    kefir_size_t source_index;
    kefir_size_t target_index;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation;

    kefir_size_t stack_tmp_position;
};

static kefir_result_t push_register_allocation(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg)));
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg)));
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT: {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));

            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        reg_allocation, tmp_reg.reg));
            if (tmp_reg.evicted) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
            }

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t pop_register_allocation(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation, kefir_size_t stack_tmp_position,
    kefir_size_t total_tmp_positions) {
    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    KEFIR_AMD64_SYSV_ABI_QWORD * (total_tmp_positions - stack_tmp_position - 1))));
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    KEFIR_AMD64_SYSV_ABI_QWORD * (total_tmp_positions - stack_tmp_position - 1))));
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA: {
            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    KEFIR_AMD64_SYSV_ABI_QWORD *
                        (total_tmp_positions - stack_tmp_position - (tmp_reg.evicted ? 0 : 1)))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, reg_allocation),
                kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_register_allocation(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation) {
    switch (target_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
            if (source_allocation->result.type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_allocation->result.reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, source_allocation)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_allocation->result.reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, source_allocation)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (source_allocation->result.type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_allocation->result.reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, source_allocation)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_allocation->result.reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, source_allocation)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            switch (source_allocation->result.type) {
                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, target_allocation),
                        kefir_asm_amd64_xasmgen_operand_reg(source_allocation->result.reg)));
                    break;

                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                        &codegen->xasmgen,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, target_allocation),
                        kefir_asm_amd64_xasmgen_operand_reg(source_allocation->result.reg)));
                    break;

                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
                case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA: {
                    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
                    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                        mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, source_allocation)));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, target_allocation),
                        kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

                    REQUIRE_OK(
                        kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));

                } break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t map_registers_impl(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                         const struct kefir_opt_function *function,
                                         const struct kefir_opt_code_analysis *func_analysis,
                                         struct kefir_opt_sysv_amd64_function *codegen_func,
                                         kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id,
                                         struct kefir_hashtree *mappings) {
    struct kefir_opt_code_block *target_block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(&function->code, target_block_id, &target_block));

    kefir_size_t num_of_stack_tmp = 0;

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

        kefir_size_t source_index;
        kefir_size_t target_index;

        REQUIRE_OK(reg_allocation_index(codegen_func, source_allocation, &source_index));
        REQUIRE_OK(reg_allocation_index(codegen_func, source_allocation, &target_index));

        kefir_size_t stack_tmp_position = 0;
        if (source_index < target_index) {
            stack_tmp_position = num_of_stack_tmp++;
            REQUIRE_OK(push_register_allocation(mem, codegen, codegen_func, source_allocation));
        }

        struct register_mapping *mapping = KEFIR_MALLOC(mem, sizeof(struct register_mapping));
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate jump register mapping"));

        mapping->source_index = source_index;
        mapping->target_index = target_index;
        mapping->source_allocation = source_allocation;
        mapping->target_allocation = target_allocation;
        mapping->stack_tmp_position = stack_tmp_position;

        res =
            kefir_hashtree_insert(mem, mappings, (kefir_hashtree_key_t) target_index, (kefir_hashtree_value_t) mapping);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, mapping);
            return res;
        });
    }
    REQUIRE_OK(res);

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(mappings, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct register_mapping *, mapping, node->value);

        if (mapping->source_index < mapping->target_index) {
            REQUIRE_OK(pop_register_allocation(mem, codegen, codegen_func, mapping->target_allocation,
                                               mapping->stack_tmp_position, num_of_stack_tmp));
        } else if (mapping->source_index > mapping->target_index) {
            REQUIRE_OK(load_register_allocation(mem, codegen, codegen_func, mapping->target_allocation,
                                                mapping->source_allocation));
        }
    }

    if (num_of_stack_tmp > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                num_of_stack_tmp * KEFIR_AMD64_SYSV_ABI_QWORD)));
    }
    return KEFIR_OK;
}

static kefir_result_t free_register_mapping(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                            kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct register_mapping *, mapping, value);
    REQUIRE(mapping != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid register mapping"));

    memset(mapping, 0, sizeof(struct register_mapping));
    KEFIR_FREE(mem, mapping);
    return KEFIR_OK;
}

static kefir_result_t map_registers(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                    const struct kefir_opt_function *function,
                                    const struct kefir_opt_code_analysis *func_analysis,
                                    struct kefir_opt_sysv_amd64_function *codegen_func,
                                    kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id) {
    struct kefir_hashtree mapping;
    REQUIRE_OK(kefir_hashtree_init(&mapping, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&mapping, free_register_mapping, NULL));

    kefir_result_t res = map_registers_impl(mem, codegen, function, func_analysis, codegen_func, source_block_id,
                                            target_block_id, &mapping);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &mapping);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &mapping));
    return KEFIR_OK;
}

static kefir_result_t has_mapped_registers(const struct kefir_opt_function *function,
                                           const struct kefir_opt_code_analysis *func_analysis,
                                           struct kefir_opt_sysv_amd64_function *codegen_func,
                                           kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id,
                                           kefir_bool_t *result) {
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

        kefir_size_t source_index;
        kefir_size_t target_index;

        REQUIRE_OK(reg_allocation_index(codegen_func, source_allocation, &source_index));
        REQUIRE_OK(reg_allocation_index(codegen_func, source_allocation, &target_index));

        if (source_index != target_index) {
            *result = true;
            return KEFIR_OK;
        }
    }
    REQUIRE_OK(res);

    *result = false;
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

            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register condition_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, condition_allocation, codegen_func, &condition_reg, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        condition_allocation, condition_reg.reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(condition_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(condition_reg.reg)));

            kefir_bool_t has_mapped_regs;
            kefir_id_t alternative_label;
            REQUIRE_OK(has_mapped_registers(function, func_analysis, codegen_func, instr->block_id,
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
                    kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &condition_reg));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JZ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                            instr->operation.parameters.branch.alternative_block))));
            }

            REQUIRE_OK(
                kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &condition_reg));
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
                    kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &condition_reg));
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
