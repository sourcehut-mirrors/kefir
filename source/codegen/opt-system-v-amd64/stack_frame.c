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

#include "kefir/codegen/opt-system-v-amd64/stack_frame.h"
#include "kefir/target/abi/system-v-amd64/qwords.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

const kefir_asm_amd64_xasmgen_register_t KefirCodegenOptSysvAmd64StackFramePreservedRegs[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

_Static_assert(sizeof(KefirCodegenOptSysvAmd64StackFramePreservedRegs) /
                       sizeof(KefirCodegenOptSysvAmd64StackFramePreservedRegs[0]) ==
                   KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs,
               "Optimizer codegen for System-V AMD64: mismatch in preserved register numbers");

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_init(struct kefir_codegen_opt_sysv_amd64_stack_frame *frame) {
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                           "Expected valid pointer to optimizer codegen System-V AMD64 stack frame"));

    REQUIRE_OK(
        kefir_bitset_init_static(&frame->preserve.regs, frame->preserved_regs_content,
                                 sizeof(frame->preserved_regs_content) / sizeof(frame->preserved_regs_content[0]),
                                 KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs));
    frame->preserve.x87_control_word = false;
    frame->preserve.mxcsr_register = false;
    frame->preserve_area_size = 0;
    frame->spill_area_size = 0;
    frame->register_aggregate_area_size = 0;
    frame->local_area_size = 0;
    frame->local_area_alignment = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_register(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));

    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(reg, &reg));
    }
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        if (KefirCodegenOptSysvAmd64StackFramePreservedRegs[i] == reg) {
            REQUIRE_OK(kefir_bitset_set(&frame->preserve.regs, i, true));
            break;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_x87cw(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));

    if (!frame->preserve.x87_control_word) {
        frame->preserve_area_size++;
    }
    frame->preserve.x87_control_word = true;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_mxcsr(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));

    if (!frame->preserve.mxcsr_register) {
        frame->preserve_area_size++;
    }
    frame->preserve.mxcsr_register = true;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_ensure_spill(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, kefir_size_t spill_area) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));

    frame->spill_area_size = MAX(frame->spill_area_size, spill_area);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_allocate_register_aggregate(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, kefir_size_t size, kefir_size_t *offset) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));
    REQUIRE(size != 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen register aggregate size"));
    REQUIRE(offset != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to codegen register aggregate offset"));

    *offset = frame->register_aggregate_area_size;
    frame->register_aggregate_area_size += size;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_allocate_set_locals(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, const struct kefir_ir_type *locals_type,
    const struct kefir_abi_sysv_amd64_type_layout *locals_layout) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));

    if (locals_type != NULL) {
        REQUIRE(locals_layout != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid System-V AMD64 type layout"));
        REQUIRE_OK(kefir_abi_sysv_amd64_calculate_type_properties(locals_type, locals_layout, &frame->local_area_size,
                                                                  &frame->local_area_alignment));
    } else {
        frame->local_area_size = 0;
        frame->local_area_alignment = 0;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_compute(
    const struct kefir_codegen_opt_sysv_amd64_stack_frame *frame,
    struct kefir_codegen_opt_sysv_amd64_stack_frame_map *map) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid pointer optimizer codegen System-V AMD64 stack frame map"));

    map->offset.previous_base = 0;

    map->offset.preserved_general_purpose_registers = map->offset.previous_base;
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        kefir_bool_t preserve_reg;
        REQUIRE_OK(kefir_bitset_get(&frame->preserve.regs, i, &preserve_reg));
        if (preserve_reg) {
            map->offset.preserved_general_purpose_registers -= KEFIR_AMD64_SYSV_ABI_QWORD;
        }
    }

    map->offset.x87_control_word = map->offset.preserved_general_purpose_registers;
    if (frame->preserve.x87_control_word) {
        map->offset.x87_control_word -= KEFIR_AMD64_SYSV_ABI_QWORD;
    }

    map->offset.mxcsr = map->offset.x87_control_word;
    if (frame->preserve.mxcsr_register) {
        map->offset.mxcsr -= KEFIR_AMD64_SYSV_ABI_QWORD;
    }

    map->offset.spill_area = map->offset.mxcsr - (frame->spill_area_size * KEFIR_AMD64_SYSV_ABI_QWORD);
    map->offset.register_aggregate_area =
        map->offset.spill_area - (frame->register_aggregate_area_size * KEFIR_AMD64_SYSV_ABI_QWORD);

    map->offset.local_area = map->offset.register_aggregate_area - frame->local_area_size;
    if (frame->local_area_size > 0) {
        map->offset.local_area = -(kefir_int64_t) kefir_target_abi_pad_aligned(
            (kefir_size_t) -map->offset.local_area, MAX(frame->local_area_alignment, 2 * KEFIR_AMD64_SYSV_ABI_QWORD));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_prologue(
    const struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));

    struct kefir_asm_amd64_xasmgen_operand xasmgen_oper[1];

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));

    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        kefir_bool_t preserve_reg;
        REQUIRE_OK(kefir_bitset_get(&frame->preserve.regs, i, &preserve_reg));
        if (preserve_reg) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KefirCodegenOptSysvAmd64StackFramePreservedRegs[i])));
        }
    }

    struct kefir_codegen_opt_sysv_amd64_stack_frame_map frame_map;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_compute(frame, &frame_map));
    kefir_size_t stack_frame_size =
        (kefir_size_t) - (frame_map.offset.local_area - frame_map.offset.preserved_general_purpose_registers);
    if (stack_frame_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&xasmgen_oper[0], stack_frame_size)));
    }

    if (frame->preserve.x87_control_word) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTCW(
            xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                         &xasmgen_oper[0], kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                         frame_map.offset.x87_control_word)));
    }

    if (frame->preserve.mxcsr_register) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_STMXCSR(
            xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                         &xasmgen_oper[0], kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                         frame_map.offset.mxcsr)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_epilogue(
    const struct kefir_codegen_opt_sysv_amd64_stack_frame *frame, struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(frame != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen System-V AMD64 stack frame"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));

    struct kefir_asm_amd64_xasmgen_operand xasmgen_oper[1];

    struct kefir_codegen_opt_sysv_amd64_stack_frame_map frame_map;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_compute(frame, &frame_map));
    kefir_size_t stack_frame_size =
        (kefir_size_t) - (frame_map.offset.local_area - frame_map.offset.preserved_general_purpose_registers);

    if (frame->preserve.x87_control_word) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLDCW(
            xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                         &xasmgen_oper[0], kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                         frame_map.offset.x87_control_word)));
    }

    if (frame->preserve.mxcsr_register) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LDMXCSR(
            xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                         &xasmgen_oper[0], kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                         frame_map.offset.mxcsr)));
    }

    if (stack_frame_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&xasmgen_oper[0], stack_frame_size)));
    }

    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        kefir_bool_t preserve_reg;
        REQUIRE_OK(kefir_bitset_get(&frame->preserve.regs, i, &preserve_reg));
        if (preserve_reg) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
                xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KefirCodegenOptSysvAmd64StackFramePreservedRegs[i])));
        }
    }

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP)));

    return KEFIR_OK;
}
