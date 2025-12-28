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

#include "kefir/codegen/target-ir/coalesce.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_coalesce_init(struct kefir_codegen_target_ir_coalesce *coalesce) {
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR coalescing"));

    REQUIRE_OK(kefir_graph_init(&coalesce->coalesce_graph));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalescing"));

    REQUIRE_OK(kefir_graph_free(mem, &coalesce->coalesce_graph));
    return KEFIR_OK;
}

static kefir_result_t record_coalesce(struct kefir_mem *mem, const struct kefir_codegen_target_ir_interference *interference, struct kefir_codegen_target_ir_coalesce *coalesce,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t other_value_ref) {
    REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref) != KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref), KEFIR_OK);

    kefir_bool_t interferes = false;
    REQUIRE_OK(kefir_codegen_target_ir_interference_has(interference, value_ref, other_value_ref, &interferes));
    if (!interferes) {
        REQUIRE_OK(kefir_graph_add_edge(mem, &coalesce->coalesce_graph,
            (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref),
            (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref)));
        REQUIRE_OK(kefir_graph_add_edge(mem, &coalesce->coalesce_graph,
            (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref),
            (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
    }
    return KEFIR_OK;
}

struct try_coalesce_payload {
    struct kefir_codegen_target_ir_coalesce *coalesce;
    const struct kefir_codegen_target_ir_interference *interference;
};

static kefir_result_t coalesce_callback(struct kefir_mem *mem, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t other_value_ref, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct try_coalesce_payload *, coalesce_payload, payload);
    REQUIRE(coalesce_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce payload"));

    REQUIRE_OK(record_coalesce(mem, coalesce_payload->interference, coalesce_payload->coalesce, value_ref, other_value_ref));
    return KEFIR_OK;
}

static kefir_result_t do_coalesce_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_coalesce_class *klass, struct kefir_list *queue) {
    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) control_flow->code->entry_block));
    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));
        
        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));

            if (instr->operation.opcode == control_flow->code->klass->phi_opcode) {
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(control_flow->code, instr_ref, 0, &value_ref, NULL);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);

                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(control_flow->code, &iter, instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                    REQUIRE_OK(record_coalesce(mem, interference, coalesce, value_ref, link_value_ref));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            } else if (instr->operation.opcode == control_flow->code->klass->assign_opcode &&
                instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(control_flow->code, instr_ref, 0, &value_ref, NULL);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);

                REQUIRE_OK(record_coalesce(mem, interference, coalesce, instr->operation.parameters[0].direct.value_ref, value_ref));
            } else if (instr->operation.opcode == control_flow->code->klass->upsilon_opcode &&
                instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                REQUIRE_OK(record_coalesce(mem, interference, coalesce, instr->operation.parameters[0].upsilon_ref, instr->operation.parameters[1].direct.value_ref));
            } else {
                struct try_coalesce_payload payload = {
                    .coalesce = coalesce,
                    .interference = interference
                };
                REQUIRE_OK(klass->extract_coalescing(mem, control_flow->code, instr, &(struct kefir_codegen_target_ir_coalesce_callback) {
                    .coalesce = coalesce_callback,
                    .payload = &payload
                }, klass->payload));
            }
        }

        kefir_result_t res;
        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_coalesce_class *klass) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalescing"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce class"));

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = do_coalesce_build(mem, coalesce, control_flow, interference, klass, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_iter(const struct kefir_codegen_target_ir_coalesce *coalesce,
    struct kefir_codegen_target_ir_coalesce_iterator *iter,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_value_ref_t *coalesce_value_ref_ptr) {
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR coalesce iterator"));

    kefir_graph_vertex_id_t vertex;
    kefir_result_t res = kefir_graph_edge_iter(&coalesce->coalesce_graph, &iter->iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref),
        &vertex);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR coalesce iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(coalesce_value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(vertex));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_next(struct kefir_codegen_target_ir_coalesce_iterator *iter,
    kefir_codegen_target_ir_value_ref_t *coalesce_value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce iterator"));

    kefir_graph_vertex_id_t vertex;
    kefir_result_t res = kefir_graph_edge_next(&iter->iter, &vertex);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR coalesce iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(coalesce_value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(vertex));
    return KEFIR_OK;
}
