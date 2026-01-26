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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/numbering.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t do_placeholder_sink(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref,
    struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_numbering *numbering, struct kefir_hashset *placeholders) {
    REQUIRE(kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref), KEFIR_OK);
    REQUIRE_OK(kefir_hashset_clear(mem, placeholders));

    kefir_result_t res;
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;) {
        const struct kefir_codegen_target_ir_instruction *placeholder_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &placeholder_instr));
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
        if (placeholder_instr->operation.opcode != code->klass->placeholder_opcode) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *value_type;
        res = kefir_codegen_target_ir_code_instruction_output(code, placeholder_instr->instr_ref, 0, NULL, &value_type);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        if (value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            REQUIRE_OK(kefir_hashset_add(mem, placeholders, (kefir_hashset_key_t) placeholder_instr->instr_ref));
        }
    }

    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(placeholders, &iter, &key);
        res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, instr_ref, key);
        const struct kefir_codegen_target_ir_instruction *placeholder_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &placeholder_instr));

        kefir_codegen_target_ir_instruction_ref_t closest_local_user_ref = KEFIR_ID_NONE;
        kefir_size_t closest_local_user_index = ~0ull;

        kefir_result_t res;
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, placeholder_instr->instr_ref, &use_instr_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, NULL)) {
            const struct kefir_codegen_target_ir_instruction *use_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &use_instr));
            if (use_instr->block_ref != placeholder_instr->block_ref) {
                continue;
            }

            kefir_size_t index = 0;
            REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(numbering, use_instr_ref, &index));
            if (index < closest_local_user_index) {
                closest_local_user_index = index;
                closest_local_user_ref = use_instr_ref;
            }
        }

        if (closest_local_user_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_codegen_target_ir_code_move_after(code, placeholder_instr->block_ref, kefir_codegen_target_ir_code_control_prev(code, closest_local_user_ref), placeholder_instr->instr_ref));
        } else {
            REQUIRE_OK(kefir_codegen_target_ir_code_move_after(code, placeholder_instr->block_ref, kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)), placeholder_instr->instr_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_placeholder_sink(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_numbering numbering;
    struct kefir_hashset placeholders;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_init(&numbering));
    REQUIRE_OK(kefir_hashset_init(&placeholders, &kefir_hashtable_uint_ops));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_codegen_target_ir_control_flow_build(mem, &control_flow));
    REQUIRE_CHAIN(&res, kefir_codegen_target_ir_numbering_build(mem, &numbering, code));
    for (kefir_size_t i = 0; res == KEFIR_OK && i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_CHAIN(&res, do_placeholder_sink(mem, code, block_ref, &control_flow, &numbering, &placeholders));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &placeholders);
        kefir_codegen_target_ir_numbering_free(mem, &numbering);
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &placeholders);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_numbering_free(mem, &numbering);
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_codegen_target_ir_numbering_free(mem, &numbering);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}
