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
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/interference.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t process_block_at(struct kefir_mem *mem, struct kefir_hashset *preserve, struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness,
    kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_opcode_t preserve_virtual_regs_opcode, struct kefir_hashtree *per_block_ranges, struct kefir_hashset *alive_values) {
    REQUIRE_OK(kefir_codegen_target_ir_interference_build_per_block_liveness(mem, control_flow, liveness, block_ref, per_block_ranges));

    REQUIRE_OK(kefir_codegen_target_ir_interference_build_update_alive_set(mem, KEFIR_ID_NONE, per_block_ranges, alive_values));
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;) {
        REQUIRE_OK(kefir_codegen_target_ir_interference_build_update_alive_set(mem, instr_ref, per_block_ranges, alive_values));

        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
        if (instr->operation.opcode == preserve_virtual_regs_opcode) {
            REQUIRE_OK(kefir_hashset_merge(mem, preserve, alive_values));
            kefir_codegen_target_ir_instruction_ref_t next_instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref);
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            instr_ref = next_instr_ref;
        } else {
            instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_virtual_regs(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashset *preserve, struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_opcode_t preserve_vregs_opcode) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(mem, control_flow, liveness));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) ||
            kefir_codegen_target_ir_code_is_gate_block(control_flow->code, block_ref)) {
            continue;
        }

        struct kefir_hashtree per_block_ranges;
        REQUIRE_OK(kefir_hashtree_init(&per_block_ranges, &kefir_hashtree_uint_ops));
        REQUIRE_OK(kefir_hashtree_on_removal(&per_block_ranges, kefir_codegen_target_ir_interference_free_liveness_index, NULL));

        struct kefir_hashset alive_values;
        REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));

        kefir_result_t res = process_block_at(mem, preserve, code, control_flow, liveness, block_ref, preserve_vregs_opcode, &per_block_ranges, &alive_values);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashset_free(mem, &alive_values);
            kefir_hashtree_free(mem, &per_block_ranges);
            return res;
        });
        res = kefir_hashset_free(mem, &alive_values);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_free(mem, &per_block_ranges);
            return res;
        });
        REQUIRE_OK(kefir_hashtree_free(mem, &per_block_ranges));
    }

    REQUIRE(kefir_hashset_size(preserve) > 0, KEFIR_OK);
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) ||
            kefir_codegen_target_ir_code_is_gate_block(control_flow->code, block_ref) ||
            kefir_hashset_size(&control_flow->blocks[block_ref].successors) > 0) {
            continue;
        }

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(preserve, &iter, &key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(key);

            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->touch_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct = {
                            .value_ref = value_ref,
                            .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                        }
                    }
                }, NULL, NULL));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_preserve_virtual_regs(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_opcode_t preserve_vregs_opcode) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashset preserved;
    REQUIRE_OK(kefir_hashset_init(&preserved, &kefir_hashtable_uint_ops));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));

    struct kefir_codegen_target_ir_liveness liveness;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_init(&liveness));

    kefir_result_t res = preserve_virtual_regs(mem, code, &preserved, &control_flow, &liveness, preserve_vregs_opcode);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_liveness_free(mem, &liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashset_free(mem, &preserved);
        return res;
    });
    res = kefir_codegen_target_ir_liveness_free(mem, &liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashset_free(mem, &preserved);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &preserved);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &preserved));
    return KEFIR_OK;
}
