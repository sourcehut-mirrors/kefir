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

static kefir_result_t record_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *alive_values) {
    UNUSED(mem);
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    kefir_result_t res;
    kefir_graph_vertex_id_t vertex1 = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref);
    REQUIRE_OK(kefir_graph_ensure(mem, &interference->interference_graph, vertex1, kefir_hashset_size(alive_values) * 2));
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

static kefir_result_t build_block_interference_impl(struct kefir_mem *mem, const struct kefir_codegen_target_ir_liveness *liveness, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *alive_values) {
    const struct kefir_hashtree *liveness_ranges;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(mem, control_flow, liveness, block_ref, &liveness_ranges));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, KEFIR_ID_NONE, liveness_ranges, alive_values));
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
        REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, instr_ref, liveness_ranges, alive_values));

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

static kefir_result_t build_block_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref) {
    struct kefir_hashset alive_values;
    REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));
    kefir_result_t res = build_block_interference_impl(mem, liveness, interference, control_flow, block_ref, &alive_values);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &alive_values);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &alive_values));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    
    REQUIRE_OK(kefir_graph_clear(mem, &interference->interference_graph));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            REQUIRE_OK(build_block_interference(mem, interference, control_flow, liveness, block_ref));
        }
    }
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

kefir_result_t kefir_codegen_target_ir_interference_iter(const struct kefir_codegen_target_ir_interference *interference,
    struct kefir_codegen_target_ir_interference_iterator *iter,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_value_ref_t *interfere_value_ref_ptr) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference iterator"));

    kefir_graph_vertex_id_t vertex;
    kefir_result_t res = kefir_graph_edge_iter(&interference->interference_graph, &iter->iter, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &vertex);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR interference iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(interfere_value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(vertex));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_next(struct kefir_codegen_target_ir_interference_iterator *iter,
    kefir_codegen_target_ir_value_ref_t *interfere_value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference iterator"));
    
    kefir_graph_vertex_id_t vertex;
    kefir_result_t res = kefir_graph_edge_next(&iter->iter, &vertex);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR interference iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(interfere_value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(vertex));
    return KEFIR_OK;
}
