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
    return KEFIR_OK;
}

static kefir_result_t store_terminator_target(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t source_block_ref, kefir_codegen_target_ir_block_ref_t target_block_ref) {
    REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->blocks[source_block_ref].successors, (kefir_hashtreeset_entry_t) target_block_ref));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->blocks[target_block_ref].predecessors, (kefir_hashtreeset_entry_t) source_block_ref));
    return KEFIR_OK;
}

static kefir_result_t scan_operand(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_operand *operand) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->block_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->indirect.base.block_ref));
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
            REQUIRE_OK(kefir_hashtreeset_add(mem, &control_flow->indirect_jump_targets, (kefir_hashtreeset_entry_t) operand->rip_indirection.block_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t find_block_dominance_frontier(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_list *queue, struct kefir_hashset *visited) {
    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref,
            (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));
        if (kefir_hashset_has(visited, (kefir_hashset_key_t) successor_block_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) successor_block_ref));

        kefir_bool_t is_dominator;
        REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(control_flow, successor_block_ref, block_ref, &is_dominator));
        if (is_dominator) {
            kefir_result_t res;
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&control_flow->blocks[successor_block_ref].successors, &iter); res == KEFIR_OK;
                res = kefir_hashtreeset_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, succ_block_ref, iter.entry);
                REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) succ_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } else {
            REQUIRE_OK(kefir_hashset_add(mem, &control_flow->blocks[block_ref].dominance_frontier, (kefir_hashset_key_t) successor_block_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t find_dominance_frontier(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        if (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }

        kefir_result_t res = kefir_list_clear(mem, &queue);
        REQUIRE_CHAIN(&res, kefir_hashset_clear(mem, &visited));
        REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &queue, kefir_list_tail(&queue), (void *) (kefir_uptr_t) block_ref));
        REQUIRE_CHAIN(&res, find_block_dominance_frontier(mem, control_flow, block_ref, &queue, &visited));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_list_free(mem, &queue);
            kefir_hashset_free(mem, &visited);
            return res;
        });
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
    for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
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

kefir_result_t kefir_codegen_target_ir_control_flow_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(control_flow->blocks == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Target IR control flow has already been built"));

    control_flow->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_block_control_flow) * control_flow->code->blocks_length);
    REQUIRE(control_flow->blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR control flow blocks"));
    for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
        kefir_result_t res = kefir_hashtreeset_init(&control_flow->blocks[i].predecessors, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtreeset_init(&control_flow->blocks[i].successors, &kefir_hashtree_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&control_flow->blocks[i].dominance_frontier, &kefir_hashtable_uint_ops));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            control_flow->blocks = NULL;
            return res;
        });

        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
    }

    for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        kefir_codegen_target_ir_instruction_ref_t current_block_tail_ref = kefir_codegen_target_ir_code_block_control_tail(control_flow->code, block_ref);
        if (current_block_tail_ref == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *current_block_tail = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, current_block_tail_ref, &current_block_tail));

        struct kefir_codegen_target_ir_block_terminator_props terminator_props;
        REQUIRE_OK(control_flow->code->klass->is_block_terminator(current_block_tail, &terminator_props, control_flow->code->klass->payload));

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
            for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
                REQUIRE_OK(scan_operand(mem, control_flow, &instr->operation.parameters[i]));
            }
        }
    }

    struct kefir_hashtreeset_iterator source_iter, target_iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&control_flow->indirect_jump_sources, &source_iter);
        res == KEFIR_OK;
        res = kefir_hashtreeset_next(&source_iter)) {
        for (res = kefir_hashtreeset_iter(&control_flow->indirect_jump_sources, &target_iter);
            res == KEFIR_OK;
            res = kefir_hashtreeset_next(&target_iter)) {
            REQUIRE_OK(store_terminator_target(mem, control_flow, source_iter.entry, target_iter.entry));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_codegen_target_ir_control_flow_find_dominators(mem, control_flow));
    REQUIRE_OK(find_dominance_frontier(mem, control_flow));
    REQUIRE_OK(build_dominance_tree(mem, control_flow));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    if (control_flow->blocks != NULL) {
        for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
            REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->blocks[i].successors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].dominance_frontier));
        }
        KEFIR_FREE(mem, control_flow->blocks);
    }
    REQUIRE_OK(kefir_hashtable_free(mem, &control_flow->dominator_tree));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_sources));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_targets));

    control_flow->blocks = NULL;
    control_flow->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_is_dominator(const struct kefir_codegen_target_ir_control_flow *structure,
                                                     kefir_codegen_target_ir_block_ref_t dominated_block,
                                                     kefir_codegen_target_ir_block_ref_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (structure->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(
            structure, structure->blocks[dominated_block].immediate_dominator, dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_find_closest_common_dominator(const struct kefir_codegen_target_ir_control_flow *control_flow,
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
