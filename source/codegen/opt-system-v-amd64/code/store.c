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

DEFINE_TRANSLATOR(store) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.memory_access.value, &source_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.memory_access.location, &target_allocation));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register target_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
        mem, codegen, target_allocation, codegen_func, &target_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                target_allocation, target_reg.reg));

    kefir_asm_amd64_xasmgen_pointer_type_t pointer_type;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_STORE:
            pointer_type = KEFIR_AMD64_XASMGEN_POINTER_BYTE;
            break;

        case KEFIR_OPT_OPCODE_INT16_STORE:
            pointer_type = KEFIR_AMD64_XASMGEN_POINTER_WORD;
            break;

        case KEFIR_OPT_OPCODE_INT32_STORE:
            pointer_type = KEFIR_AMD64_XASMGEN_POINTER_DWORD;
            break;

        case KEFIR_OPT_OPCODE_INT64_STORE:
            pointer_type = KEFIR_AMD64_XASMGEN_POINTER_QWORD;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    switch (source_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t source_reg_variant;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(source_allocation->result.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT16_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(source_allocation->result.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT32_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(source_allocation->result.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT64_STORE:
                    source_reg_variant = source_allocation->result.reg;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
            }
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], pointer_type,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg), 0)),
                kefir_asm_amd64_xasmgen_operand_reg(source_reg_variant)));
        } break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT32_STORE:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                        &codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                                 0),
                        kefir_asm_amd64_xasmgen_operand_reg(source_allocation->result.reg)));
                    break;

                case KEFIR_OPT_OPCODE_INT64_STORE:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                        &codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                                 0),
                        kefir_asm_amd64_xasmgen_operand_reg(source_allocation->result.reg)));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_POINTER_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT: {
            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register source_tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &source_tmp_reg, NULL, NULL));

            kefir_asm_amd64_xasmgen_register_t source_reg_variant;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(source_tmp_reg.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT16_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(source_tmp_reg.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT32_STORE:
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(source_tmp_reg.reg, &source_reg_variant));
                    break;

                case KEFIR_OPT_OPCODE_INT64_STORE:
                    source_reg_variant = source_tmp_reg.reg;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
            }

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(source_reg_variant),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], pointer_type,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[1], &codegen_func->stack_frame_map, source_allocation))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], pointer_type,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg), 0)),
                kefir_asm_amd64_xasmgen_operand_reg(source_reg_variant)));

            REQUIRE_OK(
                kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &source_tmp_reg));
        } break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer codegen register allocation");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &target_reg));
    return KEFIR_OK;
}
