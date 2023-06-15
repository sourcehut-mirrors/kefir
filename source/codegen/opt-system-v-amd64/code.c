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

#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
#include "kefir/codegen/opt-system-v-amd64/code.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_opt_sysv_amd64_function *codegen_func,
    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen function"));
    REQUIRE(tmp_reg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to AMD64 System-V optimizer codegen temporary register"));

    kefir_bool_t filter_success = true;
    if (reg_allocation != NULL &&
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {

        if (filter_callback != NULL) {
            REQUIRE_OK(filter_callback(reg_allocation->result.reg, &filter_success, filter_payload));
        }

        if (filter_success) {
            tmp_reg->borrow = false;
            tmp_reg->evicted = false;
            tmp_reg->reg = reg_allocation->result.reg;
            return KEFIR_OK;
        }
    }

    tmp_reg->borrow = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64GeneralPurposeRegisters[i];
        if (!kefir_hashtreeset_has(&codegen_func->occupied_general_purpose_regs,
                                   (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtreeset_has(&codegen_func->borrowed_general_purpose_regs,
                                   (kefir_hashtreeset_entry_t) candidate)) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }

            if (filter_success) {
                REQUIRE_OK(kefir_hashtreeset_add(mem, &codegen_func->borrowed_general_purpose_regs,
                                                 (kefir_hashtreeset_entry_t) candidate));
                tmp_reg->evicted = false;
                tmp_reg->reg = candidate;
                return KEFIR_OK;
            }
        }
    }

    tmp_reg->evicted = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64GeneralPurposeRegisters[i];
        if (!kefir_hashtreeset_has(&codegen_func->borrowed_general_purpose_regs,
                                   (kefir_hashtreeset_entry_t) candidate)) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }
            if (filter_success) {
                REQUIRE_OK(kefir_hashtreeset_add(mem, &codegen_func->borrowed_general_purpose_regs,
                                                 (kefir_hashtreeset_entry_t) candidate));
                tmp_reg->reg = candidate;
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(tmp_reg->reg)));
                return KEFIR_OK;
            }
        }
    }

    return KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to obtain a temporary register");
}

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t reg, struct kefir_opt_sysv_amd64_function *codegen_func,
    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *tmp_reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen function"));
    REQUIRE(tmp_reg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to AMD64 System-V optimizer codegen temporary register"));

    if (reg_allocation != NULL &&
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
        reg_allocation->result.reg == reg) {
        tmp_reg->borrow = false;
        tmp_reg->evicted = false;
        tmp_reg->reg = reg_allocation->result.reg;
        return KEFIR_OK;
    }

    REQUIRE(!kefir_hashtreeset_has(&codegen_func->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested temporary register has already been borrowed"));

    tmp_reg->borrow = true;
    if (!kefir_hashtreeset_has(&codegen_func->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg)) {
        tmp_reg->evicted = false;
    } else {
        tmp_reg->evicted = true;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }

    REQUIRE_OK(
        kefir_hashtreeset_add(mem, &codegen_func->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg));
    tmp_reg->reg = reg;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_register_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *tmp_reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V optimizer codegen function"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid AMD64 System-V optimizer codegen temporary register"));

    if (tmp_reg->borrow) {
        REQUIRE_OK(kefir_hashtreeset_delete(mem, &codegen_func->borrowed_general_purpose_regs,
                                            (kefir_hashtreeset_entry_t) tmp_reg->reg));
    }
    if (tmp_reg->evicted) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg->reg)));
    }
    return KEFIR_OK;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
    struct kefir_asm_amd64_xasmgen_operand *operand,
    struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    REQUIRE(operand != NULL, NULL);
    REQUIRE(stack_frame_map != NULL, NULL);
    REQUIRE(reg_allocation != NULL, NULL);

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return NULL;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            return kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg);

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                stack_frame_map->offset.spill_area + reg_allocation->result.spill_index * KEFIR_AMD64_SYSV_ABI_QWORD);

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                stack_frame_map->offset.register_aggregate_area +
                    reg_allocation->result.register_aggregate.index * KEFIR_AMD64_SYSV_ABI_QWORD);

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.indirect.base_register),
                reg_allocation->result.indirect.offset);
    }
    return NULL;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_load_reg_allocation_into(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t target_reg) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to load floating-point register into 8/16-bit register");
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t source_reg) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to store 8/16-bit register into floating-point register");
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t update_translate_context_liveness(
    struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
    struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_opt_code_analysis_instruction_properties *instr_props,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&codegen_func->alive_instr); iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);

        const struct kefir_opt_instruction_liveness_interval *liveness =
            &func_analysis->liveness.intervals[func_analysis->instructions[instr_ref].linear_position];
        if (liveness->range.end <= instr_props->linear_position + 1) {
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *instr_reg_allocation = NULL;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                           &instr_reg_allocation));

            if (instr_reg_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                REQUIRE_OK(kefir_hashtreeset_delete(mem, &codegen_func->occupied_general_purpose_regs,
                                                    (kefir_hashtreeset_entry_t) instr_reg_allocation->result.reg));
            }

            const struct kefir_list_entry *next_iter = iter->next;
            REQUIRE_OK(kefir_list_pop(mem, &codegen_func->alive_instr, (struct kefir_list_entry *) iter));
            iter = next_iter;
        } else {
            kefir_list_next(&iter);
        }
    }

    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(kefir_list_insert_after(mem, &codegen_func->alive_instr, kefir_list_tail(&codegen_func->alive_instr),
                                           (void *) (kefir_uptr_t) instr_props->instr_ref));

        REQUIRE_OK(kefir_hashtreeset_add(mem, &codegen_func->occupied_general_purpose_regs,
                                         (kefir_hashtreeset_entry_t) reg_allocation->result.reg));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instr(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                      struct kefir_opt_module *module, const struct kefir_opt_function *function,
                                      const struct kefir_opt_code_analysis *func_analysis,
                                      struct kefir_opt_sysv_amd64_function *codegen_func,
                                      const struct kefir_opt_code_analysis_instruction_properties *instr_props) {
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator,
                                                                   instr_props->instr_ref, &reg_allocation));

    REQUIRE_OK(update_translate_context_liveness(mem, func_analysis, codegen_func, instr_props, reg_allocation));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &codegen_func->borrowed_general_purpose_regs,
                                         (kefir_hashtreeset_entry_t) reg_allocation->result.reg));
    }

#define INVOKE_TRANSLATOR(_id)                                                                                 \
    (kefir_codegen_opt_sysv_amd64_translate_##_id(mem, codegen, module, function, func_analysis, codegen_func, \
                                                  instr_props->instr_ref))

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_GET_ARGUMENT:
            REQUIRE_OK(INVOKE_TRANSLATOR(get_argument));
            break;

        case KEFIR_OPT_OPCODE_GET_LOCAL:
            REQUIRE_OK(INVOKE_TRANSLATOR(get_local));
            break;

        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
            REQUIRE_OK(INVOKE_TRANSLATOR(store));
            break;

        case KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT64_LOAD:
            REQUIRE_OK(INVOKE_TRANSLATOR(load));
            break;

        case KEFIR_OPT_OPCODE_INT_CONST:
        case KEFIR_OPT_OPCODE_UINT_CONST:
            REQUIRE_OK(INVOKE_TRANSLATOR(constant));
            break;

        case KEFIR_OPT_OPCODE_INT_ADD:
        case KEFIR_OPT_OPCODE_INT_SUB:
        case KEFIR_OPT_OPCODE_INT_MUL:
        case KEFIR_OPT_OPCODE_INT_AND:
        case KEFIR_OPT_OPCODE_INT_OR:
        case KEFIR_OPT_OPCODE_INT_XOR:
            REQUIRE_OK(INVOKE_TRANSLATOR(binary_op));
            break;

        case KEFIR_OPT_OPCODE_INT_DIV:
        case KEFIR_OPT_OPCODE_INT_MOD:
        case KEFIR_OPT_OPCODE_UINT_DIV:
        case KEFIR_OPT_OPCODE_UINT_MOD:
            REQUIRE_OK(INVOKE_TRANSLATOR(div_mod));
            break;

        case KEFIR_OPT_OPCODE_INT_LSHIFT:
        case KEFIR_OPT_OPCODE_INT_RSHIFT:
        case KEFIR_OPT_OPCODE_INT_ARSHIFT:
            REQUIRE_OK(INVOKE_TRANSLATOR(bitshift));
            break;

        case KEFIR_OPT_OPCODE_INT_NOT:
        case KEFIR_OPT_OPCODE_BOOL_NOT:
            REQUIRE_OK(INVOKE_TRANSLATOR(unary_op));
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
            REQUIRE_OK(INVOKE_TRANSLATOR(int_extend));
            break;

        case KEFIR_OPT_OPCODE_RETURN:
            REQUIRE_OK(INVOKE_TRANSLATOR(return));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED,
                                   "Code generation for provided optimizer opcode is not implemented yet");
    }
#undef INVOKE_TRANSLATOR

    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(kefir_hashtreeset_delete(mem, &codegen_func->borrowed_general_purpose_regs,
                                            (kefir_hashtreeset_entry_t) reg_allocation->result.reg));
    }
    REQUIRE(kefir_hashtreeset_empty(&codegen_func->borrowed_general_purpose_regs),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected set of borrowed general purpose registers to be empty"));
    return KEFIR_OK;
}

static kefir_result_t init_translator(struct kefir_mem *mem, struct kefir_opt_sysv_amd64_function *codegen_func) {
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        kefir_bool_t preserved;
        REQUIRE_OK(kefir_bitset_get(&codegen_func->stack_frame.preserve.regs, i, &preserved));
        if (!preserved) {
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &codegen_func->occupied_general_purpose_regs,
                                      (kefir_hashtreeset_entry_t) KefirCodegenOptSysvAmd64StackFramePreservedRegs[i]));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_code(struct kefir_mem *mem,
                                                           struct kefir_codegen_opt_amd64 *codegen,
                                                           struct kefir_opt_module *module,
                                                           const struct kefir_opt_function *function,
                                                           const struct kefir_opt_code_analysis *func_analysis,
                                                           struct kefir_opt_sysv_amd64_function *codegen_func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function"));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", function->ir_func->name));
    if (function->ir_func->locals != NULL) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_allocate_set_locals(
            &codegen_func->stack_frame, function->ir_func->locals, &codegen_func->locals_layout));
    }
    if (codegen_func->declaration.returns.implicit_parameter) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_preserve_implicit_parameter(&codegen_func->stack_frame));
    }
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_prologue(&codegen_func->stack_frame, &codegen->xasmgen));

    REQUIRE_OK(
        kefir_codegen_opt_sysv_amd64_stack_frame_compute(&codegen_func->stack_frame, &codegen_func->stack_frame_map));

    REQUIRE_OK(init_translator(mem, codegen_func));
    for (kefir_size_t block_idx = 0; block_idx < func_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func_analysis->block_linearization[block_idx];
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK,
                                             function->ir_func->name, block_props->block_id));

        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             instr_idx < block_props->linear_range.end_index; instr_idx++) {
            REQUIRE_OK(translate_instr(mem, codegen, module, function, func_analysis, codegen_func,
                                       func_analysis->linearization[instr_idx]));
        }
    }
    return KEFIR_OK;
}