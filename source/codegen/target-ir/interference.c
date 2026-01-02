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

#include "kefir/codegen/target-ir/interference.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_interference_init(struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference"));

    REQUIRE_OK(kefir_graph_init(&interference->interference_graph));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));

    REQUIRE_OK(kefir_graph_free(mem, &interference->interference_graph));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));

    REQUIRE_OK(kefir_graph_clear(mem, &interference->interference_graph));
    return KEFIR_OK;
}

static kefir_result_t get_liveness_index_for(struct kefir_mem *mem,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, struct kefir_hashtree *per_block_ranges, struct kefir_codegen_target_ir_interference_liveness_index **liveness_index_ptr) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(per_block_ranges, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *liveness_index_ptr = (struct kefir_codegen_target_ir_interference_liveness_index *) node->value;
    } else {
        struct kefir_codegen_target_ir_interference_liveness_index *liveness_index = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_interference_liveness_index));
        REQUIRE(liveness_index != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR per block liveness index"));
        res = kefir_hashset_init(&liveness_index->begin_liveness, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness_index->end_liveness, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, per_block_ranges, (kefir_hashtree_key_t) instr_ref, (kefir_hashtree_value_t) liveness_index));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, liveness_index);
            return res;
        });
        *liveness_index_ptr = liveness_index;
    }

    return KEFIR_OK;
}

static kefir_result_t register_liveness_at(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashtree *per_block_ranges) {

    kefir_result_t res;
    kefir_codegen_target_ir_instruction_ref_t begin_ref, end_ref;
    struct kefir_codegen_target_ir_value_liveness_block_iterator iter;
    for (res = kefir_codegen_target_ir_value_liveness_at(liveness, &iter, value_ref, block_ref, &begin_ref, &end_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_value_liveness_at_next(&iter, &begin_ref, &end_ref)) {
            struct kefir_codegen_target_ir_interference_liveness_index *begin_liveness, *end_liveness;
            REQUIRE_OK(get_liveness_index_for(mem, begin_ref, per_block_ranges, &begin_liveness));
            REQUIRE_OK(get_liveness_index_for(mem, end_ref, per_block_ranges, &end_liveness));

            REQUIRE_OK(kefir_hashset_add(mem, &begin_liveness->begin_liveness, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
            REQUIRE_OK(kefir_hashset_add(mem, &end_liveness->end_liveness, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_build_per_block_liveness(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashtree *per_block_ranges) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(per_block_ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));

    REQUIRE_OK(kefir_hashtree_clean(mem, per_block_ranges));
    for (kefir_size_t i = 0; i < liveness->blocks[block_ref].live_in.length; i++) {
        REQUIRE_OK(register_liveness_at(mem, liveness, liveness->blocks[block_ref].live_in.content[i], block_ref, per_block_ranges));
    }
    for (kefir_size_t i = 0; i < liveness->blocks[block_ref].live_out.length; i++) {
        REQUIRE_OK(register_liveness_at(mem, liveness, liveness->blocks[block_ref].live_out.content[i], block_ref, per_block_ranges));
    }
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(liveness->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            REQUIRE_OK(register_liveness_at(mem, liveness, value_ref, block_ref, per_block_ranges));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }   
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_build_update_alive_set(struct kefir_mem *mem, kefir_codegen_target_ir_instruction_ref_t instr_ref, struct kefir_hashtree *per_block_ranges, struct kefir_hashset *alive_values) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(per_block_ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));
    REQUIRE(alive_values != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR alive value set"));

    if (instr_ref == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_trim(mem, alive_values));
    }

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(per_block_ranges, (kefir_hashtree_key_t) instr_ref, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_interference_liveness_index *, liveness_index,
        node->value);
    if (instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_subtract(alive_values, &liveness_index->end_liveness));
    }
    REQUIRE_OK(kefir_hashset_merge(mem, alive_values, &liveness_index->begin_liveness));
    return KEFIR_OK;
}

static kefir_result_t record_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *alive_values) {
    UNUSED(mem);
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    kefir_result_t res;
    kefir_graph_vertex_id_t vertex1 = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref);
    for (res = kefir_hashset_iter(alive_values, &iter, &iter_key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &iter_key)) {
        kefir_codegen_target_ir_value_ref_t conflict_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(iter_key);
        if (value_ref.instr_ref != conflict_value_ref.instr_ref || value_ref.aspect != conflict_value_ref.aspect) {
            REQUIRE_OK(kefir_graph_add_edge(mem, &interference->interference_graph, vertex1, (kefir_graph_vertex_id_t) iter_key));
            REQUIRE_OK(kefir_graph_add_edge(mem, &interference->interference_graph, (kefir_graph_vertex_id_t) iter_key, vertex1));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t build_block_interference_impl(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashtree *per_block_ranges, struct kefir_hashset *alive_values) {
    REQUIRE_OK(kefir_codegen_target_ir_interference_build_update_alive_set(mem, KEFIR_ID_NONE, per_block_ranges, alive_values));
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
        REQUIRE_OK(kefir_codegen_target_ir_interference_build_update_alive_set(mem, instr_ref, per_block_ranges, alive_values));

        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(control_flow->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            if (!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect)) {
                REQUIRE_OK(record_interference(mem, interference, value_ref, alive_values));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));
        if (instr->operation.opcode == control_flow->code->klass->upsilon_opcode) {
            REQUIRE_OK(record_interference(mem, interference, instr->operation.parameters[0].upsilon_ref, alive_values));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t build_block_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashtree *per_block_ranges) {
    UNUSED(interference);
    REQUIRE_OK(kefir_codegen_target_ir_interference_build_per_block_liveness(mem, control_flow, liveness, block_ref, per_block_ranges));

    struct kefir_hashset alive_values;
    REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));
    kefir_result_t res = build_block_interference_impl(mem, interference, control_flow, block_ref, per_block_ranges, &alive_values);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &alive_values);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &alive_values));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_free_liveness_index(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_interference_liveness_index *, liveness_index,
        value);
    REQUIRE(liveness_index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness index"));

    REQUIRE_OK(kefir_hashset_free(mem, &liveness_index->begin_liveness));
    REQUIRE_OK(kefir_hashset_free(mem, &liveness_index->end_liveness));
    KEFIR_FREE(mem, liveness_index);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    
    REQUIRE_OK(kefir_graph_clear(mem, &interference->interference_graph));

    struct kefir_hashtree per_block_ranges;
    REQUIRE_OK(kefir_hashtree_init(&per_block_ranges, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&per_block_ranges, kefir_codegen_target_ir_interference_free_liveness_index, NULL));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            kefir_result_t res;
            res = build_block_interference(mem, interference, control_flow, liveness, block_ref, &per_block_ranges);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_hashtree_free(mem, &per_block_ranges);
                return res;
            });
        }
    }
    REQUIRE_OK(kefir_hashtree_free(mem, &per_block_ranges));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_has(const struct kefir_codegen_target_ir_interference *interference,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t other_value_ref,
    kefir_bool_t *has_interference) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(has_interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference flag"));
    
    *has_interference = kefir_graph_has_edge(&interference->interference_graph,
        (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref),
        (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref));
    return KEFIR_OK;
}
