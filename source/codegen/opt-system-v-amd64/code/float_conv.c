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

DEFINE_TRANSLATOR(float_conv) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_TO_FLOAT32:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SS(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg_allocation)));
            break;

        case KEFIR_OPT_OPCODE_INT_TO_FLOAT64:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                    &codegen_func->stack_frame_map, arg_allocation)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_TO_INT: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY,
                arg_allocation, &arg_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, arg_allocation, &arg_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTTSS2SI(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &arg_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg_handle));
        } break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_INT: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY,
                arg_allocation, &arg_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, arg_allocation, &arg_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTTSD2SI(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &arg_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg_handle));
        } break;

        case KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY,
                arg_allocation, &arg_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, arg_allocation, &arg_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSS2SD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &arg_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg_handle));
        } break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
                    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY,
                arg_allocation, &arg_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, arg_allocation, &arg_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSD2SS(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &arg_handle.location)));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg_handle));
        } break;

        case KEFIR_OPT_OPCODE_UINT_TO_FLOAT32:
        case KEFIR_OPT_OPCODE_UINT_TO_FLOAT64: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &arg_handle, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(
                &codegen->xasmgen, &codegen_func->stack_frame_map, arg_allocation, &arg_handle.location));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PXOR(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                               kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                   &codegen->xasmgen_helpers.operands[0], &arg_handle.location),
                                               kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                   &codegen->xasmgen_helpers.operands[1], &arg_handle.location)));

            kefir_id_t sign_label = codegen_func->nonblock_labels++;
            kefir_id_t nosign_label = codegen_func->nonblock_labels++;

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JS(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                       &codegen->xasmgen_helpers.operands[0],
                                       kefir_asm_amd64_xasmgen_helpers_format(
                                           &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                           function->ir_func->name, instr->block_id, sign_label))));

            if (instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_TO_FLOAT32) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SS(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                          &arg_handle.location)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                          &arg_handle.location)));
            }

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                       &codegen->xasmgen_helpers.operands[0],
                                       kefir_asm_amd64_xasmgen_helpers_format(
                                           &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                           function->ir_func->name, instr->block_id, nosign_label))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                 function->ir_func->name, instr->block_id, sign_label));

            struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
                mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                     kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                         &codegen->xasmgen_helpers.operands[0], &tmp_handle.location),
                                                     kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                         &codegen->xasmgen_helpers.operands[1], &arg_handle.location)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
                &codegen->xasmgen,
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &arg_handle.location),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 1)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHR(
                &codegen->xasmgen,
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &tmp_handle.location),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 1)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(&codegen->xasmgen,
                                                    kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                        &codegen->xasmgen_helpers.operands[0], &tmp_handle.location),
                                                    kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                        &codegen->xasmgen_helpers.operands[1], &arg_handle.location)));

            if (instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_TO_FLOAT32) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SS(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                          &tmp_handle.location)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADDSS(&codegen->xasmgen,
                                                           kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                           kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                          &tmp_handle.location)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADDSD(&codegen->xasmgen,
                                                           kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                           kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));
            }

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &tmp_handle));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                 function->ir_func->name, instr->block_id, nosign_label));

            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                    &arg_handle));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(float_to_uint_conv) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        result_allocation, &result_handle, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_XMM0),
        arg_allocation, &arg_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg_allocation, &arg_handle.location));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_FLOAT32_TO_UINT:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_FLOAT32_TO_UINT)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_UINT:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_FLOAT64_TO_UINT)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &arg_handle));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));
    return KEFIR_OK;
}
