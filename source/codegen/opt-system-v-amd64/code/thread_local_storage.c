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
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t initial_exec_tls(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, const struct kefir_opt_module *module,
    struct kefir_opt_sysv_amd64_function *codegen_func, const struct kefir_opt_instruction *instr,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {

    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &result_handle, NULL, NULL));

    const char *identifier = kefir_ir_module_get_named_symbol(module->ir_module, instr->operation.parameters.ir_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    if (!kefir_ir_module_has_external(module->ir_module, identifier) && !codegen->config->position_independent_code) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_THREAD_LOCAL,
                                                           identifier)),
                0)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_segment(
                &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SEGMENT_FS,
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 0))));

    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_pointer(
                &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                kefir_asm_amd64_xasmgen_operand_segment(
                    &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SEGMENT_FS,
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[2], 0)))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_rip_indirection(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_THREAD_LOCAL_GOT,
                                                       identifier))));
    }

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));
    return KEFIR_OK;
}

static kefir_result_t save_regs(struct kefir_codegen_opt_amd64 *codegen,
                                struct kefir_opt_sysv_amd64_function *codegen_func,
                                const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation,
                                kefir_int64_t *offset_ptr) {
    *offset_ptr = 0;
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfCallerSavedRegs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirCodegenOptSysvAmd64StackFrameCallerSavedRegs[i];
        if (result_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            result_allocation->result.reg == reg) {
            continue;
        }
        kefir_bool_t occupied;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&codegen_func->storage, reg, &occupied));
        if (!occupied) {
            continue;
        }
        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                kefir_asm_amd64_xasmgen_operand_reg(reg)));
        }
        *offset_ptr += KEFIR_AMD64_SYSV_ABI_QWORD;
    }

    kefir_int64_t aligned_offset = kefir_target_abi_pad_aligned(*offset_ptr, 2 * KEFIR_AMD64_SYSV_ABI_QWORD);
    if (aligned_offset > *offset_ptr) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], aligned_offset - *offset_ptr)));
    }
    return KEFIR_OK;
}

static kefir_result_t restore_regs(struct kefir_codegen_opt_amd64 *codegen,
                                   struct kefir_opt_sysv_amd64_function *codegen_func,
                                   const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation,
                                   kefir_int64_t offset) {
    kefir_int64_t aligned_offset = kefir_target_abi_pad_aligned(offset, 2 * KEFIR_AMD64_SYSV_ABI_QWORD);
    if (aligned_offset > offset) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], aligned_offset - offset)));
    }

    for (kefir_size_t i = KefirCodegenOptSysvAmd64StackFrameNumOfCallerSavedRegs; i > 0; i--) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirCodegenOptSysvAmd64StackFrameCallerSavedRegs[i - 1];
        if (result_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            result_allocation->result.reg == reg) {
            continue;
        }

        kefir_bool_t occupied;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&codegen_func->storage, reg, &occupied));
        if (!occupied) {
            continue;
        }

        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t general_dynamic_tls(
    struct kefir_codegen_opt_amd64 *codegen, const struct kefir_opt_module *module,
    struct kefir_opt_sysv_amd64_function *codegen_func, const struct kefir_opt_instruction *instr,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    kefir_int64_t offset;
    REQUIRE_OK(save_regs(codegen, codegen_func, result_allocation, &offset));

    const char *identifier = kefir_ir_module_get_named_symbol(module->ir_module, instr->operation.parameters.ir_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_DATA16(&codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
        kefir_asm_amd64_xasmgen_operand_rip_indirection(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_TLSGD, identifier))));

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                                 kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x6666)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_REXW(&codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_TLS_GET_ADDR)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(restore_regs(codegen, codegen_func, result_allocation, offset));
    return KEFIR_OK;
}

static kefir_result_t emulated_tls(struct kefir_codegen_opt_amd64 *codegen, const struct kefir_opt_module *module,
                                   struct kefir_opt_sysv_amd64_function *codegen_func,
                                   const struct kefir_opt_instruction *instr,
                                   const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    kefir_int64_t offset;
    REQUIRE_OK(save_regs(codegen, codegen_func, result_allocation, &offset));

    const char *identifier = kefir_ir_module_get_named_symbol(module->ir_module, instr->operation.parameters.ir_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    if (!kefir_ir_module_has_external(module->ir_module, identifier) && !codegen->config->position_independent_code) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[1],
                                                      kefir_asm_amd64_xasmgen_helpers_format(
                                                          &codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_V, identifier)),
                0)));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
            kefir_asm_amd64_xasmgen_operand_pointer(
                &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                kefir_asm_amd64_xasmgen_operand_rip_indirection(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_GOT,
                                                           identifier)))));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_EMUTLS_GET_ADDR)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(restore_regs(codegen, codegen_func, result_allocation, offset));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(thread_local_storage) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    if (codegen->config->emulated_tls) {
        REQUIRE_OK(emulated_tls(codegen, module, codegen_func, instr, result_allocation));
    } else if (codegen->config->position_independent_code) {
        REQUIRE_OK(general_dynamic_tls(codegen, module, codegen_func, instr, result_allocation));
    } else {
        REQUIRE_OK(initial_exec_tls(mem, codegen, module, codegen_func, instr, result_allocation));
    }
    return KEFIR_OK;
}
