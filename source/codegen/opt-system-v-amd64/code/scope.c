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

DEFINE_TRANSLATOR(push_scope) {
    DEFINE_TRANSLATOR_PROLOGUE;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "PUSH SCOPE"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 codegen_func->stack_frame_map.offset.dynamic_scope)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                 result_reg.evicted ? KEFIR_AMD64_SYSV_ABI_QWORD : 0),
        kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                            &codegen_func->stack_frame_map, result_allocation),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 codegen_func->stack_frame_map.offset.dynamic_scope),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(pop_scope) {
    DEFINE_TRANSLATOR_PROLOGUE;

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "POP SCOPE"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));
    // REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
    //     kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
    //     kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
    //         &codegen_func->stack_frame_map,
    //         arg_allocation)));

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_CMP(&codegen->xasmgen,
                                      kefir_asm_amd64_xasmgen_operand_pointer(
                                          &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                                          kefir_asm_amd64_xasmgen_operand_indirect(
                                              &codegen->xasmgen_helpers.operands[1],
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                              codegen_func->stack_frame_map.offset.dynamic_scope)),
                                      kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[2], 0)));

    kefir_id_t label = codegen_func->nonblock_labels++;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JE(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_asm_amd64_xasmgen_helpers_format(
                                   &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                   function->ir_func->name, instr->block_id, label))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                            &codegen_func->stack_frame_map, arg_allocation)));

    struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg_allocation, &arg_reg, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                 arg_reg.evicted ? KEFIR_AMD64_SYSV_ABI_QWORD : 0)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 codegen_func->stack_frame_map.offset.dynamic_scope),
        kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg_reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                         function->ir_func->name, instr->block_id, label));

    // REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMOVL(&codegen->xasmgen,
    //     kefir_asm_amd64_xasmgen_operand_reg(alignment_reg.reg),
    //     kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
    //     &codegen_func->stack_frame_map,
    //         alignment_allocation)));

    // REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
    //     kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
    //     kefir_asm_amd64_xasmgen_operand_indirect(
    //                      &codegen->xasmgen_helpers.operands[0],
    //                      kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
    //                      codegen_func->stack_frame_map.offset.dynamic_scope)));

    // struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;
    // REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
    //     mem, &codegen->xasmgen, &codegen_func->storage, arg_allocation, &arg_reg, NULL, NULL));

    // REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
    //                                                              arg_allocation, arg_reg.reg));

    // REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
    //     kefir_asm_amd64_xasmgen_operand_indirect(
    //                      &codegen->xasmgen_helpers.operands[0],
    //                      kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
    //                      codegen_func->stack_frame_map.offset.dynamic_scope),
    //     kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg)));

    // REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
    //                                                                  &arg_reg));
    return KEFIR_OK;
}
