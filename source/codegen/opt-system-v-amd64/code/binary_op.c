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

DEFINE_TRANSLATOR(binary_op) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &arg2_allocation));

    REQUIRE(
        arg2_allocation->result.type != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
        KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral add operation"));

    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &result_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg1_allocation, &result_handle.location));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_ADD:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_SUB:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_MUL:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IMUL(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_AND:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_OR:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_XOR:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            break;

        case KEFIR_OPT_OPCODE_BOOL_AND: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle,
                kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
                (void *) (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg1_allocation, NULL}));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg)));

            kefir_asm_amd64_xasmgen_register_t result_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_handle.location.reg, &result_variant));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            if (arg2_allocation->result.type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                    &codegen->xasmgen, &codegen_func->stack_frame_map, arg2_allocation, &tmp_handle.location));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
                    kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg2_allocation->result.reg),
                    kefir_asm_amd64_xasmgen_operand_reg(arg2_allocation->result.reg)));
            }

            kefir_asm_amd64_xasmgen_register_t tmp_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(tmp_handle.location.reg, &tmp_variant));
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_variant),
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &tmp_handle));
        } break;

        case KEFIR_OPT_OPCODE_BOOL_OR: {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg2_allocation)));
            kefir_asm_amd64_xasmgen_register_t result_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_handle.location.reg, &result_variant));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));
    return KEFIR_OK;
}
