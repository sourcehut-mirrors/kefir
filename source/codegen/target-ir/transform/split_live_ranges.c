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

#include "kefir/codegen/target-ir/split.h"
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/interference.h"
#include "kefir/codegen/target-ir/update.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct split_payload {
    const struct kefir_codegen_target_ir_split_live_ranges_profile *profile;
    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_liveness liveness;

    struct kefir_hashset alive_values;

    struct kefir_hashtable split_occurences;
    struct kefir_hashset split_candidates;
    struct kefir_hashset splits;

    kefir_size_t num_of_gp;
    kefir_size_t num_of_fp;
};

static kefir_result_t build_update_alive_set(struct kefir_mem *mem, const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_liveness_value_block_ranges *per_block_ranges, struct kefir_hashset *alive_values, kefir_size_t *num_of_gp, kefir_size_t *num_of_fp) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(per_block_ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));
    REQUIRE(alive_values != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR alive value set"));

    if (instr_ref == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_trim(mem, alive_values));
    }

    const struct kefir_codegen_target_ir_liveness_index *liveness_index;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_range_get(liveness, per_block_ranges, instr_ref, &liveness_index));

    kefir_result_t res;
    kefir_hashset_key_t entry;
    struct kefir_hashset_iterator iter;
    if (instr_ref != KEFIR_ID_NONE) {
        for (res = kefir_hashset_iter(&liveness_index->end_liveness, &iter, &entry); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            REQUIRE_OK(kefir_hashset_delete(alive_values, entry));
            kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(entry);
            const struct kefir_codegen_target_ir_value_type *value_type = NULL;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
            if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE) {
                (*num_of_gp)--;
            } else if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT) {
                (*num_of_fp)--;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (res = kefir_hashset_iter(&liveness_index->begin_liveness, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        REQUIRE_OK(kefir_hashset_add(mem, alive_values, entry));
        kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(entry);
        const struct kefir_codegen_target_ir_value_type *value_type = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
        if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE) {
            (*num_of_gp)++;
        } else if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT) {
            (*num_of_fp)++;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t collect_split_candidates(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct split_payload *payload, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(payload->num_of_gp >= payload->profile->general_purpose_interference_threshold ||
        payload->num_of_fp >= payload->profile->floating_point_interference_threshold, KEFIR_OK);

    kefir_size_t split_gp = 0, split_fp = 0;
    if (payload->num_of_gp > payload->profile->general_purpose_interference_threshold) {
        split_gp = payload->num_of_gp - payload->profile->general_purpose_interference_threshold;
    }
    if (payload->num_of_fp > payload->profile->floating_point_interference_threshold) {
        split_fp = payload->num_of_fp - payload->profile->floating_point_interference_threshold;
    }

    for (kefir_size_t live_in_idx = 0; (split_gp != 0 || split_fp != 0) && live_in_idx < payload->liveness.blocks[block_ref].live_in.length; live_in_idx++) {
        kefir_codegen_target_ir_value_ref_t value_ref = payload->liveness.blocks[block_ref].live_in.content[live_in_idx];
        if (!kefir_hashset_has(&payload->alive_values, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) ||
            kefir_hashset_has(&payload->split_candidates, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));

        if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            continue;
        } else if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE) {
            if (split_gp == 0) {
                continue;
            }
        } else if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT) {
            if (split_fp == 0) {
                continue;
            }
        } else {
            continue;
        }
        
        kefir_size_t num_of_uses = kefir_codegen_target_ir_code_num_of_uses(code, value_ref);
        kefir_size_t max_splits = MAX(num_of_uses * payload->profile->max_splits_per_use_pct / 100, payload->profile->max_splits_baseline);

        if (kefir_hashtable_has(&payload->split_occurences, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
            kefir_hashtable_value_t *table_value_ptr;
            REQUIRE_OK(kefir_hashtable_at_mut(&payload->split_occurences, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value_ptr));
            if (*table_value_ptr >= max_splits) {
                continue;
            }
            (*table_value_ptr)++;
        } else if (max_splits > 0) {
            REQUIRE_OK(kefir_hashtable_insert(mem, &payload->split_occurences, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) 1));
        } else {
            continue;
        }

        if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE) {
            split_gp--;
        } else if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT) {
            split_fp--;
        }

        REQUIRE_OK(kefir_hashset_add(mem, &payload->split_candidates, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
    }

    return KEFIR_OK;
}

static kefir_result_t split_live_ranges(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct split_payload *payload) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, &payload->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(mem, &payload->control_flow, &payload->liveness));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE(kefir_hashset_size(&payload->control_flow.blocks[block_ref].predecessors) < payload->profile->max_branching, KEFIR_OK);
    }

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(&payload->control_flow, block_ref) ||
            kefir_codegen_target_ir_code_is_gate_block(code, block_ref)) {
            continue;
        }

        const struct kefir_codegen_target_ir_liveness_value_block_ranges *liveness_ranges;
        REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(mem, &payload->control_flow, &payload->liveness, block_ref, &liveness_ranges));

        payload->num_of_gp = 0;
        payload->num_of_fp = 0;
        REQUIRE_OK(kefir_hashset_clear(mem, &payload->split_candidates));
        REQUIRE_OK(build_update_alive_set(mem, code, &payload->liveness, KEFIR_ID_NONE, liveness_ranges, &payload->alive_values, &payload->num_of_gp, &payload->num_of_fp));

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
            REQUIRE_OK(build_update_alive_set(mem, code, &payload->liveness, instr_ref, liveness_ranges, &payload->alive_values, &payload->num_of_gp, &payload->num_of_fp));

            REQUIRE_OK(collect_split_candidates(mem, code, payload, block_ref));
        }
        
        for (kefir_size_t live_in_idx = 0; live_in_idx < payload->liveness.blocks[block_ref].live_in.length; live_in_idx++) {
            kefir_codegen_target_ir_value_ref_t value_ref = payload->liveness.blocks[block_ref].live_in.content[live_in_idx];
            if (!kefir_hashset_has(&payload->split_candidates, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
            
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
            struct kefir_codegen_target_ir_value_type value_type_copy = *value_type;

            kefir_codegen_target_ir_instruction_ref_t split_instr_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                KEFIR_ID_NONE,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->assign_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct.value_ref = value_ref,
                        .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, &metadata, &split_instr_ref));
            kefir_codegen_target_ir_value_ref_t split_value_ref = {
                .instr_ref = split_instr_ref,
                .aspect = value_ref.aspect
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, split_value_ref, &value_type_copy));

            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                split_instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->touch_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct.value_ref = split_value_ref,
                        .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, NULL, NULL));

            REQUIRE_OK(kefir_hashset_add(mem, &payload->splits, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&split_value_ref)));
        }
        
        for (kefir_size_t live_out_idx = 0; live_out_idx < payload->liveness.blocks[block_ref].live_out.length; live_out_idx++) {
            kefir_codegen_target_ir_value_ref_t value_ref = payload->liveness.blocks[block_ref].live_out.content[live_out_idx];
            if (!kefir_hashset_has(&payload->split_candidates, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
            
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
            struct kefir_codegen_target_ir_value_type value_type_copy = *value_type;

            kefir_codegen_target_ir_instruction_ref_t split_instr_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->assign_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct.value_ref = value_ref,
                        .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, &metadata, &split_instr_ref));
            kefir_codegen_target_ir_value_ref_t split_value_ref = {
                .instr_ref = split_instr_ref,
                .aspect = value_ref.aspect
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, split_value_ref, &value_type_copy));

            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                split_instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->touch_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct.value_ref = split_value_ref,
                        .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, NULL, NULL));

            REQUIRE_OK(kefir_hashset_add(mem, &payload->splits, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&split_value_ref)));
        }
    }

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    for (res = kefir_hashset_iter(&payload->splits, &iter, &iter_key); res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &iter_key)) {
        kefir_codegen_target_ir_value_ref_t split_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(iter_key);

        const struct kefir_codegen_target_ir_instruction *split_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, split_value_ref.instr_ref, &split_instr));

        REQUIRE_OK(kefir_codegen_target_ir_partial_replace_value(mem, code, &payload->control_flow, split_value_ref, split_instr->operation.parameters[0].direct.value_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_codegen_target_ir_transform_phi_removal(mem, code, false));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_split_live_ranges(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_split_live_ranges_profile *profile) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR split live ranges profile"));
    REQUIRE(kefir_codegen_target_ir_code_block_count(code) <= profile->max_blocks, KEFIR_OK);

    struct split_payload payload = {
        .profile = profile
    };
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&payload.control_flow, code));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_init(&payload.liveness));
    REQUIRE_OK(kefir_hashset_init(&payload.splits, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&payload.split_candidates, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&payload.alive_values, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&payload.split_occurences, &kefir_hashtable_uint_ops));

    kefir_result_t res = split_live_ranges(mem, code, &payload);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &payload.split_occurences);
        kefir_hashset_free(mem, &payload.alive_values);
        kefir_hashset_free(mem, &payload.split_candidates);
        kefir_hashset_free(mem, &payload.splits);
        kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &payload.split_occurences);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &payload.alive_values);
        kefir_hashset_free(mem, &payload.split_candidates);
        kefir_hashset_free(mem, &payload.splits);
        kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &payload.alive_values);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &payload.split_candidates);
        kefir_hashset_free(mem, &payload.splits);
        kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &payload.split_candidates);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &payload.splits);
        kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &payload.splits);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    res = kefir_codegen_target_ir_liveness_free(mem, &payload.liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow);
        return res;
    });
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_free(mem, &payload.control_flow));

    return KEFIR_OK;
}
