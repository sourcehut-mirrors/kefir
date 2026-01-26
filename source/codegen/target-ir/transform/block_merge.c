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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t merge_block(struct kefir_mem *mem,
                                  const struct kefir_codegen_target_ir_control_flow *control_flow,
                                  struct kefir_codegen_target_ir_code *code,
                                  kefir_codegen_target_ir_block_ref_t source_block_ref,
                                  kefir_codegen_target_ir_block_ref_t target_block_ref) {
    if (!kefir_codegen_target_ir_code_is_gate_block(code, source_block_ref)) {
        kefir_codegen_target_ir_instruction_ref_t source_block_tail_ref =
            kefir_codegen_target_ir_code_block_control_tail(code, source_block_ref);
        REQUIRE(source_block_tail_ref != KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid target IR block tail instruction"));
        const struct kefir_codegen_target_ir_instruction *source_block_tail;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, source_block_tail_ref, &source_block_tail));

        struct kefir_codegen_target_ir_block_terminator_props terminator_props;
        REQUIRE_OK(code->klass->is_block_terminator(code, source_block_tail, &terminator_props, code->klass->payload));
        REQUIRE(terminator_props.block_terminator && !terminator_props.function_terminator &&
                    !terminator_props.branch && !terminator_props.undefined_target && !terminator_props.inline_assembly,
                KEFIR_OK);
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, source_block_tail_ref));
    }

    for (kefir_codegen_target_ir_instruction_ref_t instr_ref =
             kefir_codegen_target_ir_code_block_control_head(code, target_block_ref);
         instr_ref != KEFIR_ID_NONE; instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
        kefir_codegen_target_ir_instruction_ref_t copied_instr_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_copy_instruction(
            mem, code, source_block_ref, kefir_codegen_target_ir_code_block_control_tail(code, source_block_ref),
            instr_ref, &copied_instr_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, copied_instr_ref, instr_ref));
    }

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&control_flow->blocks[target_block_ref].successors, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, key);

        struct kefir_codegen_target_ir_value_phi_node_iterator phi_iter;
        kefir_codegen_target_ir_instruction_ref_t phi_ref;
        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_iter, successor_block_ref, &phi_ref);
             res == KEFIR_OK; res = kefir_codegen_target_ir_code_phi_node_next(&phi_iter, &phi_ref)) {
            kefir_codegen_target_ir_value_ref_t link_value_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(code, phi_ref, target_block_ref, &link_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, phi_ref, source_block_ref, link_value_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t merge_blocks(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                   struct kefir_hashtree *elim_edges,
                                   struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));

    kefir_result_t res;
    kefir_hashset_key_t key;
    struct kefir_hashset_iterator iter;
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) ||
            kefir_hashset_size(&control_flow->blocks[block_ref].successors) != 1) {
            continue;
        }

        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].successors, &iter, &key); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, key);
            if (successor_block_ref == block_ref) {
                continue;
            }

            const struct kefir_codegen_target_ir_block *successor_block =
                kefir_codegen_target_ir_code_block_at(code, successor_block_ref);
            REQUIRE(successor_block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected target IR block to exist"));

            if (kefir_hashset_size(&control_flow->blocks[successor_block_ref].predecessors) == 1 &&
                kefir_hashtreeset_empty(&successor_block->public_labels) &&
                successor_block_ref != control_flow->code->entry_block) {
                REQUIRE_OK(kefir_hashtree_insert(mem, elim_edges, (kefir_hashtree_key_t) block_ref,
                                                 (kefir_hashtree_value_t) successor_block_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    struct kefir_hashtree_node_iterator node_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(elim_edges, &node_iter); node != NULL;
         node = kefir_hashtree_iter(elim_edges, &node_iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, node->key);
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, node->value);
        REQUIRE_OK(merge_block(mem, control_flow, code, block_ref, successor_block_ref));
        REQUIRE_OK(kefir_hashtree_delete(mem, elim_edges, (kefir_hashtree_key_t) block_ref));

        struct kefir_hashtree_node *chain_node;
        res = kefir_hashtree_at(elim_edges, (kefir_hashtree_key_t) successor_block_ref, &chain_node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_hashtree_insert(mem, elim_edges, (kefir_hashtable_key_t) block_ref, chain_node->value));
            REQUIRE_OK(kefir_hashtree_delete(mem, elim_edges, (kefir_hashtable_key_t) successor_block_ref));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_block_merge(struct kefir_mem *mem,
                                                             struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashtree elim_edges;
    REQUIRE_OK(kefir_hashtree_init(&elim_edges, &kefir_hashtree_uint_ops));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));

    kefir_result_t res = merge_blocks(mem, code, &elim_edges, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashtree_free(mem, &elim_edges);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &elim_edges);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &elim_edges));
    return KEFIR_OK;
}
