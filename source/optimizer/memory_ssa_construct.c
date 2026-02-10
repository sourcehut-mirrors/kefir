#include "kefir/optimizer/memory_ssa.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct construct_state {
    struct kefir_mem *mem;
    struct kefir_opt_code_memssa *memssa;
    const struct kefir_opt_code_container *code;
    const struct kefir_opt_code_control_flow *control_flow;
    const struct kefir_opt_code_liveness *liveness;

    struct kefir_hashset visited_blocks;
    struct kefir_list block_queue;
    struct kefir_hashtable inserted_phis;

    struct kefir_hashtable instr_nodes;
    struct kefir_list instr_queue;
    struct kefir_hashset instr_queue_index;
    kefir_opt_code_memssa_node_ref_t input_node_ref;
    kefir_size_t num_of_inputs;
    kefir_bool_t all_inputs_ready;
};

#define MEMORY_OP_NONE 0
#define MEMORY_OP_PRODUCE 1
#define MEMORY_OP_CONSUME 2

static kefir_result_t is_instr_memory(const struct kefir_opt_instruction *instr, kefir_uint32_t *op_type) {
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY:
        case KEFIR_OPT_OPCODE_INVOKE:
        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL:
        case KEFIR_OPT_OPCODE_INT128_ATOMIC_CMPXCHG:
        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_COMPARE_EXCHANGE:
        case KEFIR_OPT_OPCODE_COPY_MEMORY:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_MEMORY:
        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM:
        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO:
        case KEFIR_OPT_OPCODE_VARARG_COPY:
        case KEFIR_OPT_OPCODE_VARARG_GET:
            *op_type = MEMORY_OP_PRODUCE | MEMORY_OP_CONSUME;
            break;

        case KEFIR_OPT_OPCODE_INT8_LOAD:
        case KEFIR_OPT_OPCODE_INT16_LOAD:
        case KEFIR_OPT_OPCODE_INT32_LOAD:
        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_INT128_LOAD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD:
        case KEFIR_OPT_OPCODE_INT128_ATOMIC_LOAD:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_FENV_UPDATE:
            *op_type = MEMORY_OP_CONSUME;
            break;

        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
        case KEFIR_OPT_OPCODE_INT128_STORE:
        case KEFIR_OPT_OPCODE_INT128_ATOMIC_STORE:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
        case KEFIR_OPT_OPCODE_BITINT_STORE:
        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_STORE:
        case KEFIR_OPT_OPCODE_ZERO_MEMORY:
        case KEFIR_OPT_OPCODE_VARARG_START:
        case KEFIR_OPT_OPCODE_VARARG_END:
        case KEFIR_OPT_OPCODE_FENV_SAVE:
        case KEFIR_OPT_OPCODE_FENV_CLEAR:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE:
            *op_type = MEMORY_OP_PRODUCE;
            break;

        default:
            *op_type = MEMORY_OP_NONE;
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t collect_def_blocks(struct kefir_mem *mem, struct construct_state *state) {
    for (kefir_opt_block_id_t block_ref = 0; block_ref < kefir_opt_code_container_block_count(state->code);
         block_ref++) {
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(state->control_flow, block_ref, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        REQUIRE_OK(kefir_opt_code_memssa_create_block(mem, state->memssa, block_ref, KEFIR_ID_NONE));

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(state->code, block_ref, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(state->code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

            kefir_uint32_t op_type = MEMORY_OP_NONE;
            REQUIRE_OK(is_instr_memory(instr, &op_type));
            if (op_type != MEMORY_OP_NONE) {
                REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, NULL, (void *) (kefir_uptr_t) block_ref));
                break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t insert_phis(struct kefir_mem *mem, struct construct_state *state) {
    for (struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         iter = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->block_queue, iter));
        if (kefir_hashset_has(&state->visited_blocks, (kefir_hashset_key_t) block_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, &state->visited_blocks, (kefir_hashset_key_t) block_ref));

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&state->control_flow->blocks[block_ref].dominance_frontier, &iter, &entry);
             res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, frontier_block_ref, entry);
            if (kefir_hashtable_has(&state->inserted_phis, (kefir_hashtable_key_t) frontier_block_ref)) {
                continue;
            }

            kefir_opt_code_memssa_node_ref_t phi_ref;
            REQUIRE_OK(kefir_opt_code_memssa_new_phi_node(mem, state->memssa, &phi_ref));
            REQUIRE_OK(kefir_hashtable_insert(mem, &state->inserted_phis, (kefir_hashtable_key_t) frontier_block_ref,
                                              (kefir_hashtable_value_t) phi_ref));
            REQUIRE_OK(
                kefir_list_insert_after(mem, &state->block_queue, NULL, (void *) (kefir_uptr_t) frontier_block_ref));
        }
    }
    return KEFIR_OK;
}

struct link_frame {
    kefir_opt_block_id_t block_ref;
    kefir_bool_t unfolded;
    kefir_opt_code_memssa_node_ref_t node_ref;
    struct link_frame *parent;
};

static kefir_result_t push_link_frame(struct kefir_mem *mem, struct construct_state *state,
                                      kefir_opt_block_id_t block_ref, struct link_frame *parent) {
    struct link_frame *frame = KEFIR_MALLOC(mem, sizeof(struct link_frame));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR phi link frame"));
    frame->block_ref = block_ref;
    frame->unfolded = false;
    frame->node_ref = KEFIR_ID_NONE;
    frame->parent = parent;
    kefir_result_t res = kefir_list_insert_after(mem, &state->block_queue, NULL, frame);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, frame);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t find_link_for(struct link_frame *frame, kefir_opt_instruction_ref_t *link_ref) {
    for (; frame != NULL; frame = frame->parent) {
        if (frame->node_ref != KEFIR_ID_NONE) {
            *link_ref = frame->node_ref;
            return KEFIR_OK;
        }
    }

    *link_ref = KEFIR_ID_NONE;
    return KEFIR_OK;
}

static kefir_result_t link_successor_phis(struct kefir_mem *mem, struct construct_state *state,
                                          struct link_frame *frame, kefir_opt_block_id_t block_ref) {
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&state->control_flow->blocks[block_ref].successors, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_ref, (kefir_uptr_t) key);

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&state->inserted_phis, (kefir_hashtable_key_t) successor_block_ref, &table_value);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            kefir_opt_instruction_ref_t link_ref;
            REQUIRE_OK(find_link_for(frame, &link_ref));

            REQUIRE_OK(kefir_opt_code_memssa_phi_attach(
                mem, state->memssa, (kefir_opt_code_memssa_node_ref_t) table_value, block_ref, link_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t handle_input(struct construct_state *state, kefir_opt_instruction_ref_t input_instr_ref) {
    const struct kefir_opt_instruction *input_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(state->code, input_instr_ref, &input_instr));
    REQUIRE(input_instr->block_id == ((struct link_frame *) kefir_list_head(&state->block_queue)->value)->block_ref, KEFIR_OK);

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->instr_nodes, (kefir_hashtable_key_t) input_instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        if (!kefir_hashset_has(&state->instr_queue_index, (kefir_hashset_key_t) input_instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->instr_queue, kefir_list_tail(&state->instr_queue),
                                               (void *) (kefir_uptr_t) input_instr_ref));
            REQUIRE_OK(kefir_hashset_add(state->mem, &state->instr_queue_index, (kefir_hashset_key_t) input_instr_ref));
        }
        state->all_inputs_ready = false;
        return KEFIR_OK;
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(kefir_opt_code_memssa_node_ref_t, input_node_ref, table_value);
    REQUIRE(input_node_ref != KEFIR_ID_NONE, KEFIR_OK);

    if (state->num_of_inputs == 0) {
        state->input_node_ref = input_node_ref;
    } else if (state->num_of_inputs == 1) {
        kefir_opt_code_memssa_node_ref_t join_ref;
        REQUIRE_OK(kefir_opt_code_memssa_new_join_node(state->mem, state->memssa, &join_ref));
        REQUIRE_OK(kefir_opt_code_memssa_join_attach(state->mem, state->memssa, join_ref, state->input_node_ref));
        REQUIRE_OK(kefir_opt_code_memssa_join_attach(state->mem, state->memssa, join_ref, input_node_ref));
        state->input_node_ref = join_ref;
    } else {
        REQUIRE_OK(kefir_opt_code_memssa_join_attach(state->mem, state->memssa, state->input_node_ref, input_node_ref));
    }
    state->num_of_inputs++;
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_callback(kefir_opt_instruction_ref_t input_instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct construct_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory ssa constructor payload"));

    REQUIRE_OK(handle_input(state, input_instr_ref));
    return KEFIR_OK;
}

static kefir_result_t do_assign(struct kefir_mem *mem, struct construct_state *state, struct link_frame *frame) {
    for (struct kefir_list_entry *iter = kefir_list_head(&state->instr_queue); iter != NULL;
         iter = kefir_list_head(&state->instr_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->instr_queue, iter));
        REQUIRE_OK(kefir_hashset_delete(&state->instr_queue_index, (kefir_hashset_key_t) instr_ref));
        if (instr_ref == KEFIR_ID_NONE || kefir_hashtable_has(&state->instr_nodes, (kefir_hashtable_key_t) instr_ref)) {
            continue;
        }
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));
        if (instr->block_id != frame->block_ref) {
            continue;
        }

        state->all_inputs_ready = true;
        state->input_node_ref = KEFIR_ID_NONE;
        state->num_of_inputs = 0;

        REQUIRE_OK(kefir_opt_instruction_extract_inputs(state->code, instr, true, extract_inputs_callback, state));

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(state->code, instr_ref, &is_control_flow));
        if (is_control_flow) {
            kefir_opt_instruction_ref_t pred_instr_ref;
            REQUIRE_OK(kefir_opt_instruction_prev_control(state->code, instr_ref, &pred_instr_ref));
            if (pred_instr_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(handle_input(state, pred_instr_ref));
            }
        }
        
        if (!state->all_inputs_ready) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->instr_queue, kefir_list_tail(&state->instr_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashset_add(state->mem, &state->instr_queue_index, (kefir_hashset_key_t) instr_ref));
            continue;
        }

        kefir_uint32_t op_type = MEMORY_OP_NONE;
        REQUIRE_OK(is_instr_memory(instr, &op_type));

        kefir_opt_code_memssa_node_ref_t node_ref = KEFIR_ID_NONE;
        if (op_type & MEMORY_OP_PRODUCE) {
            if (node_ref == KEFIR_ID_NONE) {
                REQUIRE_OK(find_link_for(frame, &node_ref));
            }
            REQUIRE_OK(kefir_opt_code_memssa_new_produce_node(mem, state->memssa, state->input_node_ref, instr_ref,
                                                              &node_ref));
        } else if (op_type & MEMORY_OP_CONSUME) {
            if (node_ref == KEFIR_ID_NONE) {
                REQUIRE_OK(find_link_for(frame, &node_ref));
            }
            REQUIRE_OK(kefir_opt_code_memssa_new_consume_node(mem, state->memssa, state->input_node_ref, instr_ref,
                                                              &node_ref));
            node_ref = state->input_node_ref;
        }
        REQUIRE_OK(kefir_hashtable_insert(mem, &state->instr_nodes, (kefir_hashtable_key_t) instr_ref,
                                          (kefir_hashtable_value_t) node_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t assign(struct kefir_mem *mem, struct construct_state *state, struct link_frame *frame) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(state->code, frame->block_ref, &block));

    REQUIRE_OK(kefir_list_clear(mem, &state->instr_queue));
    REQUIRE_OK(kefir_hashtable_clear(mem, &state->instr_nodes));

    kefir_opt_instruction_ref_t block_tail_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(state->code, frame->block_ref, &block_tail_ref));
    if (block_tail_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_list_insert_after(mem, &state->instr_queue, kefir_list_tail(&state->instr_queue),
                                           (void *) (kefir_uptr_t) block_tail_ref));
        REQUIRE_OK(kefir_hashset_add(mem, &state->instr_queue_index, (kefir_hashset_key_t) block_tail_ref));
    }
    REQUIRE_OK(do_assign(mem, state, frame));

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(&state->liveness->blocks[frame->block_ref].alive_instr, &iter, &entry);
         res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
        if (!kefir_hashtable_has(&state->instr_nodes, (kefir_hashtable_key_t) entry)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &state->instr_queue, kefir_list_tail(&state->instr_queue),
                                               (void *) (kefir_uptr_t) entry));
            REQUIRE_OK(kefir_hashset_add(mem, &state->instr_queue_index, (kefir_hashset_key_t) entry));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(do_assign(mem, state, frame));

    return KEFIR_OK;
}

static kefir_result_t free_link_phi_frame(struct kefir_mem *mem, struct kefir_list *list,
                                          struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct link_frame *, frame, entry->value);
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR phi link frame"));

    KEFIR_FREE(mem, frame);
    return KEFIR_OK;
}

static kefir_result_t link(struct kefir_mem *mem, struct construct_state *state) {
    REQUIRE_OK(kefir_list_clear(mem, &state->block_queue));
    REQUIRE_OK(kefir_list_on_remove(&state->block_queue, free_link_phi_frame, NULL));

    REQUIRE_OK(push_link_frame(mem, state, state->code->entry_point, NULL));
    kefir_opt_code_memssa_node_ref_t root_ref;
    REQUIRE_OK(kefir_opt_code_memssa_new_root_node(mem, state->memssa, &root_ref));
    ((struct link_frame *) kefir_list_head(&state->block_queue)->value)->node_ref = root_ref;

    for (struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         iter = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(struct link_frame *, frame, iter->value);

        if (!frame->unfolded) {
            kefir_result_t res;
            kefir_hashtable_value_t table_value;
            res = kefir_hashtable_at(&state->inserted_phis, frame->block_ref, &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                frame->node_ref = (kefir_opt_code_memssa_node_ref_t) table_value;
            }

            REQUIRE_OK(assign(mem, state, frame));
            REQUIRE_OK(link_successor_phis(mem, state, frame, frame->block_ref));

            struct kefir_opt_control_flow_dominator_tree_iterator iter;
            kefir_opt_block_id_t dominated_block_ref;
            for (res = kefir_opt_control_flow_dominator_tree_iter(state->control_flow, &iter, frame->block_ref,
                                                                  &dominated_block_ref);
                 res == KEFIR_OK; res = kefir_opt_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
                REQUIRE_OK(push_link_frame(mem, state, dominated_block_ref, frame));
            }

            frame->unfolded = true;
        } else {
            REQUIRE_OK(kefir_list_pop(mem, &state->block_queue, iter));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t construct_impl(struct kefir_mem *mem, struct construct_state *state) {
    REQUIRE_OK(collect_def_blocks(mem, state));
    REQUIRE_OK(insert_phis(mem, state));
    REQUIRE_OK(link(mem, state));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_construct(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                               const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_code_control_flow *control_flow,
                                               const struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    struct construct_state state = {
        .mem = mem, .memssa = memssa, .code = code, .control_flow = control_flow, .liveness = liveness};
    REQUIRE_OK(kefir_hashset_init(&state.visited_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.inserted_phis, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_hashtable_init(&state.instr_nodes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.instr_queue));
    REQUIRE_OK(kefir_hashset_init(&state.instr_queue_index, &kefir_hashtable_uint_ops));

    kefir_result_t res = construct_impl(mem, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.instr_queue_index);
        kefir_list_free(mem, &state.instr_queue);
        kefir_hashtable_free(mem, &state.instr_nodes);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_hashset_free(mem, &state.instr_queue_index);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.instr_queue);
        kefir_hashtable_free(mem, &state.instr_nodes);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_list_free(mem, &state.instr_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.instr_nodes);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.instr_nodes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.inserted_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &state.visited_blocks));

    return KEFIR_OK;
}
