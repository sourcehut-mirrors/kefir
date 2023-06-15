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

DEFINE_TRANSLATOR(div_mod) {
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

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register quotient_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
        mem, codegen, reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX, codegen_func, &quotient_reg));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register remainder_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
        mem, codegen, reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RDX, codegen_func, &remainder_reg));

    if (arg1_allocation->result.type != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
        arg1_allocation->result.reg != quotient_reg.reg) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation_into(codegen, &codegen_func->stack_frame_map,
                                                                         arg1_allocation, quotient_reg.reg));
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_DIV: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CQO(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IDIV(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));

            if (quotient_reg.borrow) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
                    codegen, &codegen_func->stack_frame_map, reg_allocation, quotient_reg.reg));
            }
        } break;

        case KEFIR_OPT_OPCODE_UINT_DIV: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(remainder_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(remainder_reg.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_DIV(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));

            if (quotient_reg.borrow) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
                    codegen, &codegen_func->stack_frame_map, reg_allocation, quotient_reg.reg));
            }
        } break;

        case KEFIR_OPT_OPCODE_INT_MOD: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CQO(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IDIV(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));

            if (remainder_reg.borrow) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
                    codegen, &codegen_func->stack_frame_map, reg_allocation, remainder_reg.reg));
            }
        } break;

        case KEFIR_OPT_OPCODE_UINT_MOD: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(remainder_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(remainder_reg.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_DIV(
                &codegen->xasmgen,
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));

            if (remainder_reg.borrow) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
                    codegen, &codegen_func->stack_frame_map, reg_allocation, remainder_reg.reg));
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &remainder_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &quotient_reg));
    return KEFIR_OK;
}
