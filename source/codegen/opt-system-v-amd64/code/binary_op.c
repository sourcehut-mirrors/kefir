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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t filter_regs_allocation(kefir_asm_amd64_xasmgen_register_t reg, kefir_bool_t *success,
                                             void *payload) {
    REQUIRE(success != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to payload"));

    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation **, allocation_iter, payload);
    for (; *allocation_iter != NULL; ++allocation_iter) {
        if ((*allocation_iter)->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            (*allocation_iter)->result.reg == reg) {
            *success = false;
            return KEFIR_OK;
        }
    }
    *success = true;
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(binary_op) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &reg_allocation));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &arg2_allocation));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
        mem, codegen, reg_allocation, codegen_func, &result_reg, filter_regs_allocation,
        (void *) (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

    if (arg1_allocation->result.type != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
        arg1_allocation->result.reg != result_reg.reg) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation_into(codegen, &codegen_func->stack_frame_map,
                                                                         arg1_allocation, result_reg.reg));
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_ADD:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral add operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_SUB:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_MUL:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IMUL(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_AND:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_OR:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_XOR:
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_BOOL_AND: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &tmp_reg, filter_regs_allocation,
                (void *) (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg1_allocation, NULL}));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));

            kefir_asm_amd64_xasmgen_register_t result_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_reg.reg, &result_variant));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            if (arg2_allocation->result.type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation_into(
                    codegen, &codegen_func->stack_frame_map, arg2_allocation, tmp_reg.reg));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                                                          kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg2_allocation->result.reg),
                    kefir_asm_amd64_xasmgen_operand_reg(arg2_allocation->result.reg)));
            }

            kefir_asm_amd64_xasmgen_register_t tmp_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(tmp_reg.reg, &tmp_variant));
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_variant),
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
        } break;

        case KEFIR_OPT_OPCODE_BOOL_OR: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            kefir_asm_amd64_xasmgen_register_t result_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_reg.reg, &result_variant));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    if (result_reg.borrow) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(codegen, &codegen_func->stack_frame_map,
                                                                          reg_allocation, result_reg.reg));
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &result_reg));
    return KEFIR_OK;
}
