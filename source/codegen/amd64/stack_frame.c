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

#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_amd64_stack_frame_init(struct kefir_codegen_amd64_stack_frame *frame) {
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 stack frame"));

    memset(frame, 0, sizeof(struct kefir_codegen_amd64_stack_frame));
    REQUIRE_OK(kefir_hashtreeset_init(&frame->requirements.used_registers, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_free(struct kefir_mem *mem,
                                                    struct kefir_codegen_amd64_stack_frame *frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &frame->requirements.used_registers));
    memset(frame, 0, sizeof(struct kefir_codegen_amd64_stack_frame));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_ensure_spill_area(struct kefir_codegen_amd64_stack_frame *frame,
                                                                 kefir_size_t slots) {
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    frame->requirements.spill_area_slots = MAX(frame->requirements.spill_area_slots, slots);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_ensure_temporary_area(struct kefir_codegen_amd64_stack_frame *frame,
                                                                     kefir_size_t size, kefir_size_t alignment) {
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    frame->requirements.temporary_area_size = MAX(frame->requirements.temporary_area_size, size);
    frame->requirements.temporary_area_alignment = MAX(frame->requirements.temporary_area_alignment, alignment);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_use_register(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_stack_frame *frame,
                                                            kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    if (!kefir_hashtreeset_has(&frame->requirements.used_registers, (kefir_hashtreeset_entry_t) reg)) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &frame->requirements.used_registers, (kefir_hashtreeset_entry_t) reg));
        frame->requirements.num_of_used_registers++;
    }
    return KEFIR_OK;
}

static kefir_result_t calculate_sizes(kefir_abi_amd64_variant_t abi_variant, const struct kefir_ir_type *locals_type,
                                      const struct kefir_abi_amd64_type_layout *locals_type_layout,
                                      struct kefir_codegen_amd64_stack_frame *frame) {
    frame->sizes.preserved_regs = 0;
    for (kefir_size_t i = 0; i < kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(abi_variant); i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(abi_variant, i, &reg));

        if (kefir_hashtreeset_has(&frame->requirements.used_registers, (kefir_hashtreeset_entry_t) reg)) {
            frame->sizes.preserved_regs += KEFIR_AMD64_ABI_QWORD;
        }
    }

    REQUIRE_OK(kefir_abi_amd64_calculate_type_properties(locals_type, locals_type_layout, &frame->sizes.local_area,
                                                         &frame->sizes.local_area_alignment));

    frame->sizes.spill_area = frame->requirements.spill_area_slots * KEFIR_AMD64_ABI_QWORD;
    frame->sizes.temporary_area = frame->requirements.temporary_area_size;
    frame->sizes.temporary_area_alignment = frame->requirements.temporary_area_alignment;

    return KEFIR_OK;
}

#define PAD_NEGATIVE(_x, _align) (-(kefir_int64_t) kefir_target_abi_pad_aligned((kefir_size_t) (-(_x)), (_align)))

static kefir_result_t calculate_offsets(struct kefir_codegen_amd64_stack_frame *frame) {
    frame->offsets.previous_base = 0;
    frame->offsets.preserved_regs = frame->offsets.previous_base - (kefir_int64_t) frame->sizes.preserved_regs;
    frame->offsets.local_area = frame->offsets.preserved_regs - (kefir_int64_t) frame->sizes.local_area;
    frame->offsets.local_area = PAD_NEGATIVE(frame->offsets.local_area, frame->sizes.local_area_alignment);
    frame->offsets.spill_area = frame->offsets.local_area - (kefir_int64_t) frame->sizes.spill_area;
    frame->offsets.temporary_area = frame->offsets.spill_area - (kefir_int64_t) frame->sizes.temporary_area;
    frame->offsets.temporary_area = PAD_NEGATIVE(frame->offsets.temporary_area, frame->sizes.temporary_area_alignment);
    frame->offsets.top_of_frame = PAD_NEGATIVE(frame->offsets.temporary_area, 2 * KEFIR_AMD64_ABI_QWORD);
    frame->sizes.allocated_size = -(frame->offsets.top_of_frame - frame->offsets.preserved_regs);
    frame->sizes.total_size = -frame->offsets.top_of_frame;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_calculate(kefir_abi_amd64_variant_t abi_variant,
                                                         const struct kefir_ir_type *locals_type,
                                                         const struct kefir_abi_amd64_type_layout *locals_type_layout,
                                                         struct kefir_codegen_amd64_stack_frame *frame) {
    REQUIRE(locals_type_layout != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function locals type layout"));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    REQUIRE_OK(calculate_sizes(abi_variant, locals_type, locals_type_layout, frame));
    REQUIRE_OK(calculate_offsets(frame));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_prologue(struct kefir_amd64_xasmgen *xasmgen,
                                                        kefir_abi_amd64_variant_t abi_variant,
                                                        const struct kefir_codegen_amd64_stack_frame *frame) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));

    for (kefir_size_t i = 0; i < kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(abi_variant); i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(abi_variant, i, &reg));

        if (kefir_hashtreeset_has(&frame->requirements.used_registers, (kefir_hashtreeset_entry_t) reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        }
    }

    struct kefir_asm_amd64_xasmgen_operand operands[3];
    if (frame->sizes.allocated_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&operands[0], frame->sizes.allocated_size)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_stack_frame_epilogue(struct kefir_amd64_xasmgen *xasmgen,
                                                        kefir_abi_amd64_variant_t abi_variant,
                                                        const struct kefir_codegen_amd64_stack_frame *frame) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    struct kefir_asm_amd64_xasmgen_operand operands[3];
    if (frame->sizes.allocated_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&operands[0], frame->sizes.allocated_size)));
    }

    const kefir_size_t num_of_callee_preserved_gp =
        kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(abi_variant);
    for (kefir_size_t i = 0; i < num_of_callee_preserved_gp; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(
            abi_variant, num_of_callee_preserved_gp - i - 1, &reg));

        if (kefir_hashtreeset_has(&frame->requirements.used_registers, (kefir_hashtreeset_entry_t) reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        }
    }

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP)));
    return KEFIR_OK;
}
