/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t do_jump_propagation(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_codegen_target_ir_instruction_ref_t tail_ref = kefir_codegen_target_ir_code_block_control_tail(code, block_ref);
    REQUIRE(tail_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *tail_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, tail_ref, &tail_instr));

    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(code->klass->is_block_terminator(code, tail_instr, &terminator_props, code->klass->payload));
    REQUIRE(terminator_props.block_terminator && !terminator_props.function_terminator && !terminator_props.undefined_target, KEFIR_OK);

    kefir_bool_t do_replace = false;
    struct kefir_codegen_target_ir_operation replacement = tail_instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
        if (tail_instr->operation.parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF) {
            continue;
        }

        kefir_codegen_target_ir_block_ref_t target_block_ref = tail_instr->operation.parameters[i].block_ref;
        kefir_codegen_target_ir_instruction_ref_t target_tail_ref = kefir_codegen_target_ir_code_block_control_tail(code, tail_instr->operation.parameters[i].block_ref);
        if (target_tail_ref == KEFIR_ID_NONE ||
            kefir_codegen_target_ir_code_block_control_head(code, tail_instr->operation.parameters[i].block_ref) != target_tail_ref) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *target_tail_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, target_tail_ref, &target_tail_instr));
        
        struct kefir_codegen_target_ir_block_terminator_props target_terminator_props;
        REQUIRE_OK(code->klass->is_block_terminator(code, target_tail_instr, &target_terminator_props, code->klass->payload));

        if (!target_terminator_props.block_terminator || target_terminator_props.function_terminator || target_terminator_props.undefined_target || target_terminator_props.branch || target_terminator_props.target_block_refs[1] != KEFIR_ID_NONE) {
            continue;
        }

        kefir_result_t res;
        struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
        kefir_codegen_target_ir_instruction_ref_t phi_ref;
        kefir_bool_t has_link = false;
        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, target_terminator_props.target_block_refs[0], &phi_ref);
            res == KEFIR_OK && !has_link;
            res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
            kefir_codegen_target_ir_value_ref_t link_value_ref;
            res = kefir_codegen_target_ir_code_phi_link_for(code, phi_ref, block_ref, &link_value_ref);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                has_link = true;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        if (has_link) {
            continue;
        }

        replacement.parameters[i].block_ref = target_terminator_props.target_block_refs[0];
        do_replace = true;

        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, target_terminator_props.target_block_refs[0], &phi_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
            kefir_codegen_target_ir_value_ref_t link_value_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(code, phi_ref, target_block_ref, &link_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, phi_ref, block_ref, link_value_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    if (do_replace) {
        kefir_codegen_target_ir_instruction_ref_t new_tail_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, tail_ref, &replacement, &new_tail_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, new_tail_ref, tail_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, tail_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_jump_propagate(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(do_jump_propagation(mem, code, block_ref));
    }
    return KEFIR_OK;
}
