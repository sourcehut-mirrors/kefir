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

#include "kefir/codegen/target-ir/hotness.h"
#include "kefir/codegen/target-ir/interference.h"
#include "kefir/codegen/target-ir/numbering.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct hotness_payload {
    struct kefir_mem *mem;
    struct kefir_codegen_target_ir_hotness *hotness;
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_liveness *liveness;
    struct kefir_codegen_target_ir_numbering numbering;

    struct kefir_list queue;
    struct kefir_hashtable alive_values;
};

kefir_int_t kefir_codegen_target_ir_value_hotness_compare(struct kefir_codegen_target_ir_value_hotness_fragment left,
    struct kefir_codegen_target_ir_value_hotness_fragment right) {
    
    kefir_uint64_t left_num = ((kefir_uint64_t) left.uses) * right.fragment_length;
    kefir_uint64_t right_num = ((kefir_uint64_t) right.uses) * left.fragment_length;

    if (left_num > right_num) {
        return 1;
    } else if (left_num == right_num) {
        return 0;
    } else {
        return -1;
    }
}

kefir_result_t kefir_codegen_target_ir_hotness_init(struct kefir_codegen_target_ir_hotness *hotness) {
    REQUIRE(hotness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR hotness"));

    REQUIRE_OK(kefir_hashtable_init(&hotness->global_hotness, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_hotness_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_hotness *hotness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hotness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR hotness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &hotness->global_hotness));
    return KEFIR_OK;
}

static kefir_result_t update_value_score(struct hotness_payload *payload, kefir_codegen_target_ir_value_ref_t value_ref, kefir_size_t lifetime_fragment) {
    kefir_hashtable_value_t *table_value_ptr;
    kefir_result_t res = kefir_hashtable_at_mut(&payload->hotness->global_hotness, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value_ptr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        struct kefir_codegen_target_ir_value_hotness_fragment fragment = {
            .hotness = *table_value_ptr
        };
        fragment.fragment_length += lifetime_fragment;
        *table_value_ptr = fragment.hotness;
    } else {
        kefir_uint32_t uses = kefir_codegen_target_ir_code_num_of_uses(payload->control_flow->code, value_ref);
        struct kefir_codegen_target_ir_value_hotness_fragment fragment = {
            .uses = uses,
            .fragment_length = lifetime_fragment
        };
        REQUIRE_OK(kefir_hashtable_insert(payload->mem, &payload->hotness->global_hotness, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) fragment.hotness));
    }
    return KEFIR_OK;
}

static kefir_result_t update_lifetimes(struct hotness_payload *payload, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    const struct kefir_codegen_target_ir_liveness_value_block_ranges *liveness_ranges;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(payload->mem, payload->control_flow, payload->liveness, block_ref, &liveness_ranges));

    const struct kefir_codegen_target_ir_liveness_index *liveness_index;
    kefir_result_t res = kefir_codegen_target_ir_liveness_range_get(payload->liveness, liveness_ranges, instr_ref, &liveness_index);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    kefir_size_t seq_index = 0;
    if (instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&payload->numbering, instr_ref, &seq_index));
        for (res = kefir_hashset_iter(&liveness_index->end_liveness, &iter, &iter_key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &iter_key)) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&payload->alive_values, (kefir_hashtable_key_t) iter_key, &table_value));
            REQUIRE_OK(kefir_hashtable_delete(payload->mem, &payload->alive_values, (kefir_hashtable_key_t) iter_key));
            REQUIRE_OK(update_value_score(payload, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(iter_key), seq_index - table_value));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    for (res = kefir_hashset_iter(&liveness_index->begin_liveness, &iter, &iter_key); res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &iter_key)) {
        REQUIRE_OK(kefir_hashtable_insert(payload->mem, &payload->alive_values, (kefir_hashtable_key_t) iter_key, (kefir_hashtable_value_t) seq_index));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t build_hotness(struct hotness_payload *payload) {
    REQUIRE_OK(kefir_codegen_target_ir_numbering_build(payload->mem, &payload->numbering, payload->control_flow->code));
    REQUIRE_OK(kefir_list_insert_after(payload->mem, &payload->queue, kefir_list_tail(&payload->queue), (void *) (kefir_uptr_t) payload->control_flow->code->entry_block));
    for (struct kefir_list_entry *head = kefir_list_head(&payload->queue);
        head != NULL;
        head = kefir_list_head(&payload->queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(payload->mem, &payload->queue, head));

        REQUIRE_OK(kefir_hashtable_clear(payload->mem, &payload->alive_values));

        REQUIRE_OK(update_lifetimes(payload, block_ref, KEFIR_ID_NONE));

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(payload->control_flow->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(payload->control_flow->code, instr_ref)) {
            REQUIRE_OK(update_lifetimes(payload, block_ref, instr_ref));
        }

        kefir_size_t block_length;
        REQUIRE_OK(kefir_codegen_target_ir_numbering_block_length(&payload->numbering, block_ref, &block_length));

        kefir_result_t res;
        struct kefir_hashtable_iterator alive_iter;
        kefir_hashtable_key_t alive_iter_key;
        kefir_hashtable_value_t alive_iter_value;
        for (res = kefir_hashtable_iter(&payload->alive_values, &alive_iter, &alive_iter_key, &alive_iter_value); res == KEFIR_OK;
            res = kefir_hashtable_next(&alive_iter, &alive_iter_key, &alive_iter_value)) {
            REQUIRE_OK(update_value_score(payload, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(alive_iter_key), block_length - alive_iter_value));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(payload->control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(payload->mem, &payload->queue, kefir_list_tail(&payload->queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_hotness_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_hotness *hotness,
    const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hotness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR hotness"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    
    struct hotness_payload payload = {
        .mem = mem,
        .hotness = hotness,
        .control_flow = control_flow,
        .liveness = liveness
    };
    REQUIRE_OK(kefir_hashtable_init(&payload.alive_values, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&payload.queue));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_init(&payload.numbering));

    kefir_result_t res = build_hotness(&payload);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_numbering_free(mem, &payload.numbering);
        kefir_list_free(mem, &payload.queue);
        kefir_hashtable_free(mem, &payload.alive_values);
        return res;
    });
    res = kefir_codegen_target_ir_numbering_free(mem, &payload.numbering);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &payload.queue);
        kefir_hashtable_free(mem, &payload.alive_values);
        return res;
    });
    res = kefir_list_free(mem, &payload.queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &payload.alive_values);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &payload.alive_values));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_hotness_get_global(const struct kefir_codegen_target_ir_hotness *hotness,
    kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_codegen_target_ir_value_hotness_fragment *hotness_fragment) {
    REQUIRE(hotness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR hotness"));
    REQUIRE(hotness_fragment != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR hotness fragment"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&hotness->global_hotness, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR global hotness fragment");
    }
    REQUIRE_OK(res);
    
    hotness_fragment->hotness = (kefir_uint64_t) table_value;
    return KEFIR_OK;
}
