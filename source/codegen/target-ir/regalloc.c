#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct regalloc_state {
    struct kefir_codegen_target_ir_regalloc *regalloc;
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_liveness *liveness;
    const struct kefir_codegen_target_ir_interference *interference;
    const struct kefir_codegen_target_ir_coalesce *coalesce;
    struct kefir_list block_queue;
    struct kefir_list value_queue;
    struct kefir_hashtable value_scores;
    struct kefir_hashtree per_block_ranges;
    struct kefir_hashtable alive_values;
    struct kefir_codegen_target_ir_regalloc_state regalloc_state;
    const struct kefir_codegen_target_ir_stack_frame *stack_frame;
};

kefir_result_t kefir_codegen_target_ir_regalloc_init(struct kefir_codegen_target_ir_regalloc *regalloc, const struct kefir_codegen_target_ir_regalloc_class *klass) {
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator class"));

    REQUIRE_OK(kefir_hashtable_init(&regalloc->allocation, &kefir_hashtable_uint_ops));
    regalloc->klass = klass;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    REQUIRE_OK(kefir_hashtable_free(mem, &regalloc->allocation));
    return KEFIR_OK;
}

static kefir_result_t build_constraints_and_hints(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref) {
    REQUIRE_OK(state->regalloc_state.reset(mem, state->regalloc_state.payload));
    
    kefir_result_t res;
    struct kefir_graph_edge_iterator iter;
    kefir_graph_vertex_id_t interference_vertex;
    for (res = kefir_graph_edge_iter(&state->interference->interference_graph, &iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &interference_vertex);
        res == KEFIR_OK;
        res = kefir_graph_edge_next(&iter, &interference_vertex)) {
        kefir_codegen_target_ir_value_ref_t conflict_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(interference_vertex);
        const struct kefir_codegen_target_ir_value_type *conflict_value_type = NULL;
        res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, conflict_value_ref, &conflict_value_type);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (conflict_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            REQUIRE_OK(state->regalloc_state.reserve(mem, conflict_value_type, state->regalloc_state.payload));
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) interference_vertex, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(state->regalloc_state.add_conflict(mem, (kefir_codegen_target_ir_regalloc_allocation_t) table_value, state->regalloc_state.payload));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (state->coalesce != NULL) {
        struct kefir_codegen_target_ir_coalesce_iterator iter;
        kefir_codegen_target_ir_value_ref_t coalesce_value_ref;
        for (res = kefir_codegen_target_ir_coalesce_iter(state->coalesce, &iter, value_ref, &coalesce_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_coalesce_next(&iter, &coalesce_value_ref)) {
            const struct kefir_codegen_target_ir_value_type *coalesce_value_type = NULL;
            res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, coalesce_value_ref, &coalesce_value_type);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            
            if (coalesce_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                REQUIRE_OK(state->regalloc_state.add_register_hint(mem, coalesce_value_type->constraint.physical_register, state->regalloc_state.payload));
            }

            kefir_hashtable_value_t table_value;
            res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref), &table_value);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
            REQUIRE_OK(state->regalloc_state.add_allocation_hint(mem, (kefir_codegen_target_ir_regalloc_allocation_t) table_value, state->regalloc_state.payload));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

static kefir_result_t compute_eviction_score(struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref, kefir_uint64_t *score) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->value_scores, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *score = ((table_value >> 32) << 32) / ((kefir_uint32_t) table_value);
    } else {
        *score = ~0ull;
    }
    return KEFIR_OK;
}

static kefir_result_t try_evict_neighbor(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref) {
    kefir_uint64_t evict_score;
    REQUIRE_OK(compute_eviction_score(state, value_ref, &evict_score));
    REQUIRE(evict_score != ~0ull, KEFIR_OK);
    kefir_codegen_target_ir_value_ref_t evict_value = value_ref;

    kefir_result_t res;
    struct kefir_graph_edge_iterator iter;
    kefir_graph_vertex_id_t interference_vertex;
    for (res = kefir_graph_edge_iter(&state->interference->interference_graph, &iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &interference_vertex);
        res == KEFIR_OK;
        res = kefir_graph_edge_next(&iter, &interference_vertex)) {
        kefir_codegen_target_ir_value_ref_t conflict_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(interference_vertex);
        const struct kefir_codegen_target_ir_value_type *conflict_value_type = NULL;
        res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, conflict_value_ref, &conflict_value_type);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (conflict_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT || 
            (conflict_value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE &&
            conflict_value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT)) {
            continue;
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) interference_vertex, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        
        kefir_bool_t evictable;
        REQUIRE_OK(state->regalloc->klass->is_evictable(table_value, &evictable, state->regalloc->klass->payload));
        if (!evictable) {
            continue;
        }

        kefir_uint64_t score;
        REQUIRE_OK(compute_eviction_score(state, conflict_value_ref, &score));
        if (score < evict_score) {
            evict_score = score;
            evict_value = conflict_value_ref;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value) != KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
        REQUIRE_OK(kefir_hashtable_delete(mem, &state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value)));
        REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, kefir_list_tail(&state->value_queue), (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value)));
        REQUIRE_OK(build_constraints_and_hints(mem, state, value_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t do_regalloc(struct kefir_mem *mem, struct regalloc_state *state) {
    for (struct kefir_list_entry *head = kefir_list_head(&state->value_queue);
        head != NULL;
        head = kefir_list_head(&state->value_queue)) {
        kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM((kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->value_queue, head));

        kefir_result_t res;
        if (state->coalesce != NULL) {
            struct kefir_codegen_target_ir_coalesce_iterator iter;
            kefir_codegen_target_ir_value_ref_t coalesce_value_ref;
            for (res = kefir_codegen_target_ir_coalesce_iter(state->coalesce, &iter, value_ref, &coalesce_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_coalesce_next(&iter, &coalesce_value_ref)) {
                kefir_hashtable_value_t table_value;
                res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref), &table_value);
                if (res == KEFIR_NOT_FOUND) {
                    REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref)));
                } else {
                    REQUIRE_OK(res);
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }

        if (kefir_hashtable_has(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *value_type = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->control_flow->code, value_ref, &value_type));

        REQUIRE_OK(build_constraints_and_hints(mem, state, value_ref));

        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        res = state->regalloc->klass->do_allocate(mem, value_type, state->stack_frame, state->regalloc_state.payload, true, &allocation, state->regalloc->klass->payload);
        if (res == KEFIR_OUT_OF_SPACE) {
            REQUIRE_OK(try_evict_neighbor(mem, state, value_ref));
            REQUIRE_OK(state->regalloc->klass->do_allocate(mem, value_type, state->stack_frame, state->regalloc_state.payload, false, &allocation, state->regalloc->klass->payload));
        } else {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_hashtable_insert(mem, &state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) allocation));   
    }
    return KEFIR_OK;
}

static kefir_result_t do_regalloc_block(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(state->control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(state->control_flow->code, instr_ref)) {
        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(state->control_flow->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
            REQUIRE_OK(do_regalloc(mem, state));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }   
    }

    return KEFIR_OK;
}

static kefir_result_t update_value_score(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref, kefir_size_t local_lifetime) {
    kefir_hashtable_value_t *table_value_ptr;
    kefir_result_t res = kefir_hashtable_at_mut(&state->value_scores, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value_ptr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        kefir_uint32_t uses = (*table_value_ptr) >> 32;
        kefir_uint32_t lifetime = *table_value_ptr;
        lifetime += local_lifetime;
        *table_value_ptr = (((kefir_uint64_t) uses) << 32) | lifetime;
    } else {
        kefir_uint32_t uses = kefir_codegen_target_ir_code_num_of_uses(state->control_flow->code, value_ref);
        kefir_uint64_t value = (((kefir_uint64_t) uses) << 32) | (kefir_uint32_t) local_lifetime;
        REQUIRE_OK(kefir_hashtable_insert(mem, &state->value_scores, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) value));
    }
    return KEFIR_OK;
}

static kefir_result_t update_lifetimes(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_size_t local_index) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&state->per_block_ranges, (kefir_hashtree_key_t) instr_ref, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_interference_liveness_index *, liveness_index,
        node->value);
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    if (instr_ref != KEFIR_ID_NONE) {
        for (res = kefir_hashset_iter(&liveness_index->end_liveness, &iter, &iter_key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &iter_key)) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->alive_values, (kefir_hashtable_key_t) iter_key, &table_value));
            REQUIRE_OK(kefir_hashtable_delete(mem, &state->alive_values, (kefir_hashtable_key_t) iter_key));
            REQUIRE_OK(update_value_score(mem, state, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(iter_key), local_index - table_value));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    for (res = kefir_hashset_iter(&liveness_index->begin_liveness, &iter, &iter_key); res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &iter_key)) {
        REQUIRE_OK(kefir_hashtable_insert(mem, &state->alive_values, (kefir_hashtable_key_t) iter_key, (kefir_hashtable_value_t) local_index));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t regalloc_run_impl(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) block_ref));
    for (struct kefir_list_entry *head = kefir_list_head(&state->block_queue);
        head != NULL;
        head = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->block_queue, head));

        REQUIRE_OK(kefir_codegen_target_ir_interference_build_per_block_liveness(mem, state->control_flow, state->liveness, block_ref, &state->per_block_ranges));
        REQUIRE_OK(kefir_hashtable_clear(&state->alive_values));

        kefir_size_t local_index = 0;
        REQUIRE_OK(update_lifetimes(mem, state, KEFIR_ID_NONE, local_index));

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(state->control_flow->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(state->control_flow->code, instr_ref), local_index++) {
            REQUIRE_OK(update_lifetimes(mem, state, instr_ref, local_index));
        }

        kefir_result_t res;
        struct kefir_hashtable_iterator alive_iter;
        kefir_hashtable_key_t alive_iter_key;
        kefir_hashtable_value_t alive_iter_value;
        for (res = kefir_hashtable_iter(&state->alive_values, &alive_iter, &alive_iter_key, &alive_iter_value); res == KEFIR_OK;
            res = kefir_hashtable_next(&alive_iter, &alive_iter_key, &alive_iter_value)) {
            REQUIRE_OK(update_value_score(mem, state, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(alive_iter_key), local_index - alive_iter_value));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(state->control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) block_ref));
    for (struct kefir_list_entry *head = kefir_list_head(&state->block_queue);
        head != NULL;
        head = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->block_queue, head));
        REQUIRE_OK(do_regalloc_block(mem, state, block_ref));

        kefir_result_t res;
        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(state->control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_run(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness,
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_stack_frame *stack_frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    struct regalloc_state state = {
        .regalloc = regalloc,
        .control_flow = control_flow,
        .liveness = liveness,
        .interference = interference,
        .coalesce = coalesce,
        .stack_frame = stack_frame
    };
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_list_init(&state.value_queue));
    REQUIRE_OK(kefir_hashtable_init(&state.value_scores, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.per_block_ranges, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.alive_values, &kefir_hashtable_uint_ops));
    REQUIRE_OK(regalloc->klass->new_state(mem, &state.regalloc_state, regalloc->klass->payload));
    kefir_result_t res = regalloc_run_impl(mem, &state, control_flow->code->entry_block);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.alive_values);
        kefir_hashtree_free(mem, &state.per_block_ranges);
        kefir_hashtable_free(mem, &state.value_scores);
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.alive_values);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.per_block_ranges);
        kefir_hashtable_free(mem, &state.value_scores);
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_hashtree_free(mem, &state.per_block_ranges);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.value_scores);
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.value_scores);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_list_free(mem, &state.value_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    REQUIRE_OK(state.regalloc_state.free_state(mem, state.regalloc_state.payload));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_get(const struct kefir_codegen_target_ir_regalloc *regalloc,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr) {
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find register allocation for requested target IR value");
    }
    REQUIRE_OK(res);
    *allocation_ptr = (kefir_codegen_target_ir_regalloc_allocation_t) table_value;
    return KEFIR_OK;
}
