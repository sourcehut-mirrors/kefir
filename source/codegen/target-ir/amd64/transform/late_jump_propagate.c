/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/codegen/target-ir/amd64/late_transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t do_jump_propagation(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc *regalloc,
                                          kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_codegen_target_ir_instruction_ref_t tail_ref =
        kefir_codegen_target_ir_code_block_control_tail(code, block_ref);
    REQUIRE(tail_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *tail_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, tail_ref, &tail_instr));
    struct kefir_codegen_target_ir_instruction_metadata tail_metadata = tail_instr->metadata;

    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(code->klass->is_block_terminator(code, tail_instr, &terminator_props, code->klass->payload));
    REQUIRE(terminator_props.block_terminator && !terminator_props.function_terminator &&
                !terminator_props.undefined_target,
            KEFIR_OK);

    kefir_bool_t do_replace = false;
    struct kefir_codegen_target_ir_operation replacement = tail_instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
        if (tail_instr->operation.parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF) {
            continue;
        }

        kefir_codegen_target_ir_instruction_ref_t target_tail_ref =
            kefir_codegen_target_ir_code_block_control_tail(code, tail_instr->operation.parameters[i].block_ref);

        const struct kefir_codegen_target_ir_instruction *target_tail_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, target_tail_ref, &target_tail_instr));

        struct kefir_codegen_target_ir_block_terminator_props target_terminator_props;
        REQUIRE_OK(
            code->klass->is_block_terminator(code, target_tail_instr, &target_terminator_props, code->klass->payload));

        if (!target_terminator_props.block_terminator || target_terminator_props.function_terminator ||
            target_terminator_props.undefined_target || target_terminator_props.branch ||
            target_terminator_props.target_block_refs[1] != KEFIR_ID_NONE) {
            continue;
        }

        kefir_bool_t skip = false;
        for (kefir_codegen_target_ir_instruction_ref_t iter_ref = kefir_codegen_target_ir_code_block_control_head(code, tail_instr->operation.parameters[i].block_ref);
            iter_ref != KEFIR_ID_NONE && !skip;
            iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
            if (iter_ref == target_tail_ref) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *iter_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &iter_instr));
            if (iter_instr->operation.opcode != code->klass->upsilon_opcode) {
                skip = true;
                break;
            }

            kefir_codegen_target_ir_regalloc_allocation_t src_alloc, dst_alloc;
            kefir_result_t res = kefir_codegen_target_ir_regalloc_get(regalloc, iter_instr->operation.parameters[0].upsilon_ref, &dst_alloc);
            if (res == KEFIR_NOT_FOUND) {
                skip = true;
                break;
            }
            REQUIRE_OK(res);

            res = kefir_codegen_target_ir_regalloc_get(regalloc, iter_instr->operation.parameters[1].direct.value_ref, &src_alloc);
            if (res == KEFIR_NOT_FOUND) {
                skip = true;
                break;
            }
            REQUIRE_OK(res);

            if (dst_alloc != src_alloc) {
                skip = true;
            }
        }
        if (skip) {
            continue;
        }

        replacement.parameters[i].block_ref = target_terminator_props.target_block_refs[0];
        do_replace = true;
    }

    if (do_replace) {
        kefir_codegen_target_ir_instruction_ref_t new_tail_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, tail_ref, &replacement,
                                                                &tail_metadata, &new_tail_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, new_tail_ref, tail_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, tail_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_late_jump_propagation(struct kefir_mem *mem,
                                                                struct kefir_codegen_target_ir_code *code,
                                                                const struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(do_jump_propagation(mem, code, regalloc, block_ref));
    }
    return KEFIR_OK;
}
