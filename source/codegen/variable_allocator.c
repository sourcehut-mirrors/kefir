#include "kefir/codegen/variable_allocator.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_local_variable_allocator_init(struct kefir_codegen_local_variable_allocator *allocator) {
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to codegen variable allocator"));

    REQUIRE_OK(kefir_hashtree_init(&allocator->variable_locations, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&allocator->alive_variables, &kefir_hashtable_uint_ops));
    allocator->total_size = 0;
    allocator->total_alignment = 0;
    allocator->all_global = false;
    allocator->return_space_variable_ref = KEFIR_ID_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocator_free(struct kefir_mem *mem,
                                                           struct kefir_codegen_local_variable_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));

    REQUIRE_OK(kefir_hashset_free(mem, &allocator->alive_variables));
    REQUIRE_OK(kefir_hashtree_free(mem, &allocator->variable_locations));
    memset(allocator, 0, sizeof(struct kefir_codegen_local_variable_allocator));
    return KEFIR_OK;
}

struct allocator_state {
    struct kefir_mem *mem;
    struct kefir_codegen_local_variable_allocator *allocator;
    const struct kefir_opt_code_container *code;
    const struct kefir_codegen_local_variable_allocator_hooks *hooks;
    const struct kefir_opt_code_variable_scopes *scopes;
    struct kefir_hashtree current_allocation;
};

#define GET_OFFSET(_offset_size) (kefir_size_t)((((kefir_uint64_t) (_offset_size)) >> 32) & ((1ull << 32) - 1))
#define GET_SIZE(_offset_size) (kefir_size_t)(((kefir_uint64_t) (_offset_size)) & ((1ull << 32) - 1))
#define MERGE_OFFSET_SIZE(_offset, _size) (((kefir_uint64_t) (_offset)) << 32) | ((kefir_uint32_t) (_size))

static kefir_result_t do_allocate_var(struct allocator_state *state, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(kefir_hashset_has(&state->allocator->alive_variables, (kefir_hashset_key_t) instr_ref), KEFIR_OK);
    REQUIRE(state->allocator->return_space_variable_ref != instr_ref, KEFIR_OK);
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected local variable allocation instruction"));

    kefir_size_t size, alignment;
    REQUIRE_OK(state->hooks->type_layout(instr->operation.parameters.type.type_id,
                                         instr->operation.parameters.type.type_index, &size, &alignment,
                                         state->hooks->payload));
    size = MAX(size, 1);

    kefir_size_t candidate_offset = 0;
    struct kefir_hashtree_node *node = NULL;
    REQUIRE_OK(kefir_hashtree_min(&state->current_allocation, &node));
    for (; node != NULL; node = kefir_hashtree_next_node(&state->current_allocation, node)) {
        const kefir_size_t alloc_offset = GET_OFFSET(node->key);
        const kefir_size_t alloc_size = GET_SIZE(node->key);
        if (candidate_offset + size <= alloc_offset) {
            break;
        } else {
            candidate_offset =
                MAX(kefir_target_abi_pad_aligned(alloc_offset + alloc_size, alignment), candidate_offset);
        }
    }

    REQUIRE_OK(kefir_hashtree_insert(state->mem, &state->current_allocation, MERGE_OFFSET_SIZE(candidate_offset, size),
                                     (kefir_hashtree_value_t) instr_ref));
    REQUIRE_OK(kefir_hashtree_insert(state->mem, &state->allocator->variable_locations,
                                     (kefir_hashtree_key_t) instr_ref,
                                     (kefir_hashtree_value_t) MERGE_OFFSET_SIZE(candidate_offset, size)));
    state->allocator->total_size = MAX(state->allocator->total_size, candidate_offset + size);
    state->allocator->total_alignment = MAX(state->allocator->total_alignment, alignment);

    return KEFIR_OK;
}

static kefir_result_t fill_interfere_scope_vars(struct allocator_state *state, kefir_opt_instruction_ref_t scope_ref) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&state->scopes->scope_variables, (kefir_hashtree_key_t) scope_ref, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_scope_variables *, interfere_variables, node->value);

    kefir_hashset_key_t entry;
    struct kefir_hashset_iterator iter;
    for (res = kefir_hashset_iter(&interfere_variables->allocations, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
        if (!kefir_hashset_has(&state->allocator->alive_variables, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        struct kefir_hashtree_node *node;
        kefir_result_t res =
            kefir_hashtree_at(&state->allocator->variable_locations, (kefir_hashtree_key_t) instr_ref, &node);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        res = kefir_hashtree_insert(state->mem, &state->current_allocation, (kefir_hashtree_key_t) node->value,
                                    (kefir_hashtree_value_t) node->key);
        if (res != KEFIR_ALREADY_EXISTS) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_scope_vars(struct allocator_state *state, kefir_opt_instruction_ref_t scope_ref) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&state->scopes->scope_variables, (kefir_hashtree_key_t) scope_ref, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_scope_variables *, variables, node->value);

    kefir_hashset_key_t entry;
    struct kefir_hashset_iterator iter;
    for (res = kefir_hashset_iter(&variables->allocations, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
        REQUIRE_OK(do_allocate_var(state, instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t allocator_run_impl(struct allocator_state *state) {
    kefir_result_t res;
    if (state->allocator->all_global) {
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&state->allocator->alive_variables, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
            REQUIRE_OK(do_allocate_var(state, instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        return KEFIR_OK;
    }

    struct kefir_hashtree_node_iterator scopes_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->scopes->scope_variables, &scopes_iter);
         node != NULL; node = kefir_hashtree_next(&scopes_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref, node->key);

        REQUIRE_OK(kefir_hashtree_clean(state->mem, &state->current_allocation));
        struct kefir_graph_edge_iterator edge_iter;
        kefir_graph_vertex_id_t interfere_vertex_id;
        for (res =
                 kefir_graph_edge_iter(&state->scopes->scope_interference, &edge_iter, scope_ref, &interfere_vertex_id);
             res == KEFIR_OK; res = kefir_graph_edge_next(&edge_iter, &interfere_vertex_id)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, interfere_scope_ref, interfere_vertex_id);
            REQUIRE_OK(fill_interfere_scope_vars(state, interfere_scope_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(allocate_scope_vars(state, scope_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocator_mark_alive(
    struct kefir_mem *mem, struct kefir_codegen_local_variable_allocator *allocator,
    kefir_opt_instruction_ref_t instr_ref, kefir_id_t *id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));

    REQUIRE_OK(kefir_hashset_add(mem, &allocator->alive_variables, (kefir_hashset_key_t) instr_ref));
    ASSIGN_PTR(id_ptr, (kefir_id_t) instr_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocator_mark_all_global(
    struct kefir_codegen_local_variable_allocator *allocator) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));

    allocator->all_global = true;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocator_mark_return_space(
    struct kefir_codegen_local_variable_allocator *allocator, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));

    REQUIRE(allocator->return_space_variable_ref == KEFIR_ID_NONE || allocator->return_space_variable_ref == instr_ref,
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Local variable representing return space already exists"));
    allocator->return_space_variable_ref = instr_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocator_run(
    struct kefir_mem *mem, struct kefir_codegen_local_variable_allocator *allocator,
    const struct kefir_opt_code_container *code, const struct kefir_codegen_local_variable_allocator_hooks *hooks,
    const struct kefir_opt_code_variable_scopes *scopes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));
    REQUIRE(hooks != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator hooks"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer local variable scopes"));

    REQUIRE_OK(kefir_hashtree_clean(mem, &allocator->variable_locations));
    allocator->total_size = 0;
    allocator->total_alignment = 0;

    struct allocator_state state = {.mem = mem, .allocator = allocator, .code = code, .hooks = hooks, .scopes = scopes};
    REQUIRE_OK(kefir_hashtree_init(&state.current_allocation, &kefir_hashtree_uint_ops));

    kefir_result_t res = allocator_run_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.current_allocation);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &state.current_allocation));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_local_variable_allocation_of(
    const struct kefir_codegen_local_variable_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    kefir_int64_t *offset) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));
    REQUIRE(offset != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to local variable allocation type"));
    REQUIRE(offset != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to local variable offset"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&allocator->variable_locations, (kefir_hashtree_key_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested local variable allocation");
    }
    REQUIRE_OK(res);

    *offset = (kefir_int64_t) GET_OFFSET(node->value);
    return KEFIR_OK;
}
