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

#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/list.h"

struct dominator_tree_node {
    struct kefir_hashset children;
};

static kefir_result_t free_dominator_tree_node(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct dominator_tree_node *, node,
        value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dominator tree node"));

    REQUIRE_OK(kefir_hashset_free(mem, &node->children));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_init(struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR control flow"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    control_flow->code = code;
    control_flow->blocks = NULL;
    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_sources, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_targets, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&control_flow->dominator_tree, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&control_flow->dominator_tree, free_dominator_tree_node, NULL));
    REQUIRE_OK(kefir_hashtable_init(&control_flow->postdominator_tree, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&control_flow->postdominator_tree, free_dominator_tree_node, NULL));
    REQUIRE_OK(kefir_hashtable_init(&control_flow->dominator_cache, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

static kefir_result_t store_terminator_target(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t source_block_ref, kefir_codegen_target_ir_block_ref_t target_block_ref) {
    REQUIRE_OK(kefir_hashset_add(mem, &control_flow->blocks[source_block_ref].successors, (kefir_hashset_key_t) target_block_ref));
    REQUIRE_OK(kefir_hashset_add(mem, &control_flow->blocks[target_block_ref].predecessors, (kefir_hashset_key_t) source_block_ref));
    return KEFIR_OK;
}

static kefir_result_t scan_operand(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_operand *operand, kefir_codegen_target_ir_block_ref_t gate_block_ref, const struct kefir_codegen_target_ir_block_terminator_props *terminator_props) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            if (terminator_props->inline_assembly && terminator_props->undefined_target) {
                REQUIRE_OK(store_terminator_target(mem, control_flow, gate_block_ref, operand->block_ref));
            } else if (!terminator_props->block_terminator || terminator_props->undefined_target) {
                REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->block_ref));
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                    if (terminator_props->inline_assembly && terminator_props->undefined_target) {
                        REQUIRE_OK(store_terminator_target(mem, control_flow, gate_block_ref, operand->indirect.base.block_ref));
                    } else {
                        REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->indirect.base.block_ref));
                    }
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
            if (terminator_props->inline_assembly && terminator_props->undefined_target) {
                REQUIRE_OK(store_terminator_target(mem, control_flow, gate_block_ref, operand->rip_indirection.block_ref));
            } else {
                REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->rip_indirection.block_ref));
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t find_dominance_frontier(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            continue;
        }


        if (kefir_hashset_size(&control_flow->blocks[block_ref].predecessors) <= 1) {
            continue;
        }

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].predecessors, &iter, &key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, iter_block_ref, key);
            if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, iter_block_ref)) {
                continue;
            }
            for (; iter_block_ref != control_flow->blocks[block_ref].immediate_dominator; iter_block_ref = control_flow->blocks[iter_block_ref].immediate_dominator) {
                REQUIRE_OK(kefir_hashset_add(mem, &control_flow->blocks[iter_block_ref].dominance_frontier, (kefir_hashset_key_t) block_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    kefir_result_t res = kefir_list_free(mem, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &visited));

    return KEFIR_OK;
}

static kefir_result_t build_dominance_tree(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        kefir_codegen_target_ir_block_ref_t immediate_dominator_ref = control_flow->blocks[block_ref].immediate_dominator;
        if (immediate_dominator_ref == KEFIR_ID_NONE) {
            continue;
        }

        struct dominator_tree_node *node = NULL;
        kefir_hashtable_value_t table_value;
        kefir_result_t res = kefir_hashtable_at(&control_flow->dominator_tree, (kefir_hashtable_key_t) immediate_dominator_ref, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            node = KEFIR_MALLOC(mem, sizeof(struct dominator_tree_node));
            REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate dominator tree node"));
            res = kefir_hashset_init(&node->children, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &control_flow->dominator_tree, (kefir_hashtable_key_t) immediate_dominator_ref, (kefir_hashtable_value_t) node));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, node);
                return res;
            });
        } else {
            node = (struct dominator_tree_node *) table_value;
        }
        REQUIRE_OK(kefir_hashset_add(mem, &node->children, (kefir_hashset_key_t) block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t build_postdominance_tree(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        kefir_codegen_target_ir_block_ref_t immediate_postdominator_ref = control_flow->blocks[block_ref].immediate_postdominator;

        struct dominator_tree_node *node = NULL;
        kefir_hashtable_value_t table_value;
        kefir_result_t res = kefir_hashtable_at(&control_flow->postdominator_tree, (kefir_hashtable_key_t) immediate_postdominator_ref, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            node = KEFIR_MALLOC(mem, sizeof(struct dominator_tree_node));
            REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate dominator tree node"));
            res = kefir_hashset_init(&node->children, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &control_flow->postdominator_tree, (kefir_hashtable_key_t) immediate_postdominator_ref, (kefir_hashtable_value_t) node));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, node);
                return res;
            });
        } else {
            node = (struct dominator_tree_node *) table_value;
        }
        REQUIRE_OK(kefir_hashset_add(mem, &node->children, (kefir_hashset_key_t) block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t walk_dominance_tree_impl(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_list *queue) {
    kefir_size_t linear_index = 0;
    if (control_flow->code->entry_block != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) control_flow->code->entry_block));
    }

    kefir_result_t res;
    for (struct kefir_list_entry *iter = kefir_list_head(queue);
        iter != NULL;
        iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, entry,
            (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));

        kefir_codegen_target_ir_block_ref_t block_ref = (kefir_uint32_t) entry;
        if (entry & (1ull << 63)) {
            control_flow->blocks[block_ref].dominated_block_max_linear = linear_index;
        } else {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) (block_ref | (1ull << 63))));

            control_flow->blocks[block_ref].dominance_linear_index = linear_index++;
            struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator dom_iter;
            kefir_codegen_target_ir_block_ref_t dominated_block_ref;
            for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(control_flow, &dom_iter, block_ref, &dominated_block_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&dom_iter, &dominated_block_ref)) {
                REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) dominated_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    
    return KEFIR_OK;
}

static kefir_result_t walk_dominance_tree(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = walk_dominance_tree_impl(mem, control_flow, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t walk_postdominance_tree_impl(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_list *queue) {
    kefir_size_t linear_index = 0;
    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) (kefir_codegen_target_ir_code_block_count(control_flow->code) + 1)));

    kefir_result_t res;
    for (struct kefir_list_entry *iter = kefir_list_head(queue);
        iter != NULL;
        iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, entry,
            (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));

        kefir_codegen_target_ir_block_ref_t block_ref = (kefir_uint32_t) entry;
        if (block_ref == kefir_codegen_target_ir_code_block_count(control_flow->code) + 1) {
            block_ref = KEFIR_ID_NONE;
        }
        if (entry & (1ull << 63)) {
            if (block_ref != KEFIR_ID_NONE) {
                control_flow->blocks[block_ref].postdominated_block_max_linear = linear_index;
            }
        } else {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) (block_ref | (1ull << 63))));

            if (block_ref != KEFIR_ID_NONE) {
                control_flow->blocks[block_ref].postdominance_linear_index = linear_index++;
            }
            struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator dom_iter;
            kefir_codegen_target_ir_block_ref_t postdominated_block_ref;
            for (res = kefir_codegen_target_ir_control_flow_postdominator_tree_iter(control_flow, &dom_iter, block_ref, &postdominated_block_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_control_flow_postdominator_tree_next(&dom_iter, &postdominated_block_ref)) {
                REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) postdominated_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    
    return KEFIR_OK;
}

static kefir_result_t walk_postdominance_tree(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = walk_postdominance_tree_impl(mem, control_flow, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(control_flow->blocks == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Target IR control flow has already been built"));

    control_flow->blocks_length = control_flow->code->blocks_length;
    control_flow->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_block_control_flow) * control_flow->blocks_length);
    REQUIRE(control_flow->blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR control flow blocks"));
    for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
        kefir_result_t res = kefir_hashset_init(&control_flow->blocks[i].predecessors, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&control_flow->blocks[i].successors, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&control_flow->blocks[i].dominance_frontier, &kefir_hashtable_uint_ops));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            control_flow->blocks = NULL;
            return res;
        });

        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
        control_flow->blocks[i].immediate_postdominator = KEFIR_ID_NONE;
        control_flow->blocks[i].dominance_linear_index = (kefir_size_t) -1ll;
        control_flow->blocks[i].dominated_block_max_linear = (kefir_size_t) -1ll;
        control_flow->blocks[i].postdominance_linear_index = (kefir_size_t) -1ll;
        control_flow->blocks[i].postdominated_block_max_linear = (kefir_size_t) -1ll;
    }

    for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        const struct kefir_codegen_target_ir_block *block = kefir_codegen_target_ir_code_block_at(control_flow->code, block_ref);
        REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable o retrieve target IR block"));
        if (!kefir_hashtreeset_empty(&block->public_labels)) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) block_ref));
        }

        kefir_codegen_target_ir_instruction_ref_t current_block_tail_ref = kefir_codegen_target_ir_code_block_control_tail(control_flow->code, block_ref);
        if (current_block_tail_ref == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *current_block_tail = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, current_block_tail_ref, &current_block_tail));

        struct kefir_codegen_target_ir_block_terminator_props terminator_props;
        REQUIRE_OK(control_flow->code->klass->is_block_terminator(control_flow->code, current_block_tail, &terminator_props, control_flow->code->klass->payload));

        if (!terminator_props.block_terminator || terminator_props.function_terminator) {
            continue;
        }

        REQUIRE(!terminator_props.fallthrough, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Target IR code contains blocks with fallthrough terminators"));
        if (terminator_props.undefined_target) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_sources, (kefir_hashtreeset_entry_t) block_ref));
        }

        if (terminator_props.target_block_refs[0] != KEFIR_ID_NONE) {
            REQUIRE_OK(store_terminator_target(mem, control_flow, block_ref, terminator_props.target_block_refs[0]));
        }
        if (terminator_props.target_block_refs[1] != KEFIR_ID_NONE) {
            REQUIRE_OK(store_terminator_target(mem, control_flow, block_ref, terminator_props.target_block_refs[1]));
        }

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
            const struct kefir_codegen_target_ir_instruction *instr = NULL;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));
            struct kefir_codegen_target_ir_block_terminator_props instr_terminator_props;
            REQUIRE_OK(control_flow->code->klass->is_block_terminator(control_flow->code, instr, &instr_terminator_props, control_flow->code->klass->payload));
            if (instr->operation.opcode == control_flow->code->klass->phi_opcode) {
                // Intentionally left blank
            } else if (instr_terminator_props.block_terminator && instr->operation.opcode == control_flow->code->klass->inline_asm_opcode) {
                if (instr->operation.inline_asm_node.target_block_ref != KEFIR_ID_NONE) {
                    REQUIRE_OK(store_terminator_target(mem, control_flow, block_ref, instr->operation.inline_asm_node.gate_block_ref));
                    REQUIRE_OK(store_terminator_target(mem, control_flow, instr->operation.inline_asm_node.gate_block_ref, instr->operation.inline_asm_node.target_block_ref));

                    struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
                    const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
                    kefir_result_t res;
                    for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(control_flow->code, &iter, instr_ref, &fragment);
                        res == KEFIR_OK;
                        res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
                        switch (fragment->type) {
                            case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                                // Intentionally left blank
                                break;

                            case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                                REQUIRE_OK(scan_operand(mem, control_flow, &fragment->operand, instr->operation.inline_asm_node.gate_block_ref, &instr_terminator_props));
                                break;
                        }
                    }
                    if (res != KEFIR_ITERATOR_END) {
                        REQUIRE_OK(res);
                    }
                }
            } else {
                for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
                    REQUIRE_OK(scan_operand(mem, control_flow, &instr->operation.parameters[i], KEFIR_ID_NONE, &instr_terminator_props));
                }
            }
        }
    }

    struct kefir_hashtreeset_iterator source_iter, target_iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&control_flow->indirect_jump_sources, &source_iter);
        res == KEFIR_OK;
        res = kefir_hashtreeset_next(&source_iter)) {
        for (res = kefir_hashtreeset_iter(&control_flow->indirect_jump_targets, &target_iter);
            res == KEFIR_OK;
            res = kefir_hashtreeset_next(&target_iter)) {
            REQUIRE(control_flow->code->indirect_jump_gate_block != KEFIR_ID_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected target IR indirect jump gate to exist"));
            REQUIRE_OK(store_terminator_target(mem, control_flow, source_iter.entry, control_flow->code->indirect_jump_gate_block));
            REQUIRE_OK(store_terminator_target(mem, control_flow, control_flow->code->indirect_jump_gate_block, target_iter.entry));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_codegen_target_ir_control_flow_find_dominators(mem, control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_find_postdominators(mem, control_flow));
    REQUIRE_OK(find_dominance_frontier(mem, control_flow));
    REQUIRE_OK(build_dominance_tree(mem, control_flow));
    REQUIRE_OK(build_postdominance_tree(mem, control_flow));
    REQUIRE_OK(walk_dominance_tree(mem, control_flow));
    REQUIRE_OK(walk_postdominance_tree(mem, control_flow));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    if (control_flow->blocks != NULL) {
        for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].successors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].dominance_frontier));
        }
        KEFIR_FREE(mem, control_flow->blocks);
    }
    REQUIRE_OK(kefir_hashtable_free(mem, &control_flow->dominator_cache));
    REQUIRE_OK(kefir_hashtable_free(mem, &control_flow->dominator_tree));
    REQUIRE_OK(kefir_hashtable_free(mem, &control_flow->postdominator_tree));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_sources));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_targets));

    control_flow->blocks = NULL;
    control_flow->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    if (control_flow->blocks != NULL) {
        for (kefir_size_t i = 0; i < control_flow->blocks_length; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].successors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].dominance_frontier));
        }
        KEFIR_FREE(mem, control_flow->blocks);
    }
    REQUIRE_OK(kefir_hashtable_clear(mem, &control_flow->dominator_cache));
    REQUIRE_OK(kefir_hashtable_clear(mem, &control_flow->dominator_tree));
    REQUIRE_OK(kefir_hashtable_clear(mem, &control_flow->postdominator_tree));
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &control_flow->indirect_jump_sources));
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &control_flow->indirect_jump_targets));

    control_flow->blocks = NULL;
    control_flow->code = NULL;
    return KEFIR_OK;
}

kefir_bool_t kefir_codegen_target_ir_control_flow_is_reachable(const struct kefir_codegen_target_ir_control_flow *control_flow,
                                                     kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(control_flow != NULL, false);
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < kefir_codegen_target_ir_code_block_count(control_flow->code), false);

    return control_flow->blocks[block_ref].immediate_dominator != KEFIR_ID_NONE ||
        block_ref == control_flow->code->entry_block;
}


kefir_bool_t kefir_codegen_target_ir_control_flow_is_critical_edge(const struct kefir_codegen_target_ir_control_flow *control_flow,
                                                     kefir_codegen_target_ir_block_ref_t source_block_ref,
                                                    kefir_codegen_target_ir_block_ref_t target_block_ref) {
    REQUIRE(control_flow != NULL, false);
    REQUIRE(source_block_ref != KEFIR_ID_NONE && source_block_ref < kefir_codegen_target_ir_code_block_count(control_flow->code), false);
    REQUIRE(target_block_ref != KEFIR_ID_NONE && target_block_ref < kefir_codegen_target_ir_code_block_count(control_flow->code), false);
    
    return kefir_hashset_has(&control_flow->blocks[source_block_ref].successors, (kefir_hashset_key_t) target_block_ref) &&
        kefir_hashset_size(&control_flow->blocks[source_block_ref].successors) > 1 &&
        kefir_hashset_size(&control_flow->blocks[target_block_ref].predecessors) > 1;
}

kefir_result_t kefir_codegen_target_ir_control_flow_is_dominator(const struct kefir_codegen_target_ir_control_flow *structure,
                                                     kefir_codegen_target_ir_block_ref_t dominated_block,
                                                     kefir_codegen_target_ir_block_ref_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (dominated_block != KEFIR_ID_NONE && dominator_block != KEFIR_ID_NONE && structure->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        kefir_size_t linear_index = structure->blocks[dominator_block].dominance_linear_index;
        kefir_size_t dominated_block_max_linear = structure->blocks[dominator_block].dominated_block_max_linear;
        kefir_size_t dominated_linear = structure->blocks[dominated_block].dominance_linear_index;
        *result_ptr = linear_index != (kefir_size_t) -1ll &&
            dominated_block_max_linear != (kefir_size_t) -1ll &&
            linear_index < dominated_linear &&
            dominated_linear < dominated_block_max_linear;
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_is_postdominator(const struct kefir_codegen_target_ir_control_flow *structure,
                                                     kefir_codegen_target_ir_block_ref_t dominated_block,
                                                     kefir_codegen_target_ir_block_ref_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (dominated_block != KEFIR_ID_NONE && dominator_block != KEFIR_ID_NONE && structure->blocks[dominated_block].immediate_postdominator != KEFIR_ID_NONE) {
        kefir_size_t linear_index = structure->blocks[dominator_block].postdominance_linear_index;
        kefir_size_t postdominated_block_max_linear = structure->blocks[dominator_block].postdominated_block_max_linear;
        kefir_size_t postdominated_linear = structure->blocks[dominated_block].postdominance_linear_index;
        *result_ptr = linear_index != (kefir_size_t) -1ll &&
            postdominated_block_max_linear != (kefir_size_t) -1ll &&
            linear_index < postdominated_linear &&
            postdominated_linear < postdominated_block_max_linear;
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_find_closest_common_dominator(struct kefir_codegen_target_ir_control_flow *control_flow,
                                                       kefir_codegen_target_ir_block_ref_t block_ref,
                                                       kefir_codegen_target_ir_block_ref_t other_block_ref,
                                                       kefir_codegen_target_ir_block_ref_t *common_dominator_block_id) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(common_dominator_block_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR block reference"));

    if (block_ref == KEFIR_ID_NONE) {
        *common_dominator_block_id = other_block_ref;
        return KEFIR_OK;
    }
    if (other_block_ref == KEFIR_ID_NONE) {
        *common_dominator_block_id = block_ref;
        return KEFIR_OK;
    }

    kefir_bool_t is_dominator;
    kefir_codegen_target_ir_block_ref_t dominator_block_ref = other_block_ref;
    do {
        if (dominator_block_ref == KEFIR_ID_NONE) {
            break;
        }
        REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(control_flow, block_ref, dominator_block_ref, &is_dominator));
        if (is_dominator) {
            *common_dominator_block_id = dominator_block_ref;
            return KEFIR_OK;
        } else {
            dominator_block_ref = control_flow->blocks[dominator_block_ref].immediate_dominator;
        }
    } while (!is_dominator);

    *common_dominator_block_id = KEFIR_ID_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_dominator_tree_iter(const struct kefir_codegen_target_ir_control_flow *control_flow,
    struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *iter,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_block_ref_t *dominated_block_ref_ptr) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow dominator tree iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&control_flow->dominator_tree, (kefir_hashtable_key_t) block_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR dominator tree iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct dominator_tree_node *, tree_node,
        table_value);

    kefir_hashset_key_t entry;
    res = kefir_hashset_iter(&tree_node->children, &iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR dominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) entry);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_dominator_tree_next(struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *iter, kefir_codegen_target_ir_block_ref_t *dominated_block_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow dominator tree iterator"));

    kefir_hashset_key_t entry;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR dominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) entry);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_postdominator_tree_iter(const struct kefir_codegen_target_ir_control_flow *control_flow,
    struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *iter,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_block_ref_t *dominated_block_ref_ptr) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow dominator tree iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&control_flow->postdominator_tree, (kefir_hashtable_key_t) block_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR postdominator tree iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct dominator_tree_node *, tree_node,
        table_value);

    kefir_hashset_key_t entry;
    res = kefir_hashset_iter(&tree_node->children, &iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR postdominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) entry);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_postdominator_tree_next(struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *iter, kefir_codegen_target_ir_block_ref_t *dominated_block_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow dominator tree iterator"));

    kefir_hashset_key_t entry;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR postdominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) entry);
    return KEFIR_OK;
}
