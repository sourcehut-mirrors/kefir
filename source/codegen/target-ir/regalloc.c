#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct regalloc_state {
    struct kefir_codegen_target_ir_regalloc *regalloc;
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_interference *interference;
    struct kefir_list block_queue;
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

static kefir_result_t do_regalloc(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref) {
    const struct kefir_codegen_target_ir_value_type *value_type = NULL;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->control_flow->code, value_ref, &value_type));

    REQUIRE_OK(state->regalloc_state.reset(mem, state->regalloc_state.payload));
    
    kefir_result_t res;
    struct kefir_graph_edge_iterator iter;
    kefir_graph_vertex_id_t interference_vertex;
    for (res = kefir_graph_edge_iter(&state->interference->interference_graph, &iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &interference_vertex);
        res == KEFIR_OK;
        res = kefir_graph_edge_next(&iter, &interference_vertex)) {
        kefir_codegen_target_ir_value_ref_t conflict_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(interference_vertex);
        const struct kefir_codegen_target_ir_value_type *conflict_value_type = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->control_flow->code, conflict_value_ref, &conflict_value_type));
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

    kefir_codegen_target_ir_regalloc_allocation_t allocation;
    REQUIRE_OK(state->regalloc->klass->do_allocate(mem, value_type, state->stack_frame, state->regalloc_state.payload, &allocation, state->regalloc->klass->payload));
    REQUIRE_OK(kefir_hashtable_insert(mem, &state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) allocation));
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
            REQUIRE_OK(do_regalloc(mem, state, value_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }   
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
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_stack_frame *stack_frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    struct regalloc_state state = {
        .regalloc = regalloc,
        .control_flow = control_flow,
        .interference = interference,
        .stack_frame = stack_frame
    };
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(regalloc->klass->new_state(mem, &state.regalloc_state, regalloc->klass->payload));
    kefir_result_t res = regalloc_run_impl(mem, &state, control_flow->code->entry_block);
    REQUIRE_ELSE(res == KEFIR_OK, {
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        kefir_list_free(mem, &state.block_queue);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
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
