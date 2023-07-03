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

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &arg2_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle quotient_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle remainder_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle divisor_handle;

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RAX) |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &quotient_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RDX) |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &remainder_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
        arg2_allocation, &divisor_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg1_allocation, &quotient_handle.location));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg2_allocation, &divisor_handle.location));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_DIV: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CQO(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IDIV(
                &codegen->xasmgen, kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                       &codegen->xasmgen_helpers.operands[0], &divisor_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(
                &codegen->xasmgen, &codegen_func->stack_frame_map, result_allocation, &quotient_handle.location));
        } break;

        case KEFIR_OPT_OPCODE_UINT_DIV: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(remainder_handle.location.reg),
                kefir_asm_amd64_xasmgen_operand_reg(remainder_handle.location.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_DIV(
                &codegen->xasmgen, kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                       &codegen->xasmgen_helpers.operands[0], &divisor_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(
                &codegen->xasmgen, &codegen_func->stack_frame_map, result_allocation, &quotient_handle.location));
        } break;

        case KEFIR_OPT_OPCODE_INT_MOD: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CQO(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_IDIV(
                &codegen->xasmgen, kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                       &codegen->xasmgen_helpers.operands[0], &divisor_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(
                &codegen->xasmgen, &codegen_func->stack_frame_map, result_allocation, &remainder_handle.location));
        } break;

        case KEFIR_OPT_OPCODE_UINT_MOD: {
            REQUIRE(arg2_allocation->result.type !=
                        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                    KEFIR_SET_ERROR(
                        KEFIR_INVALID_STATE,
                        "Expected non-floating-point allocation for the second argument of integral operation"));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(remainder_handle.location.reg),
                kefir_asm_amd64_xasmgen_operand_reg(remainder_handle.location.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_DIV(
                &codegen->xasmgen, kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                       &codegen->xasmgen_helpers.operands[0], &divisor_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(
                &codegen->xasmgen, &codegen_func->stack_frame_map, result_allocation, &remainder_handle.location));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &divisor_handle));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                            &remainder_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &quotient_handle));
    return KEFIR_OK;
}
