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
    const struct kefir_opt_code_variable_conflicts *conflicts;
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

static kefir_result_t allocator_run_impl(struct allocator_state *state) {
    kefir_result_t res;
    kefir_hashset_key_t entry;
    struct kefir_hashset_iterator iter;
    for (res = kefir_hashset_iter(&state->conflicts->globally_alive, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
        REQUIRE_OK(do_allocate_var(state, instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_hashtree_node *max_global_node = NULL;
    REQUIRE_OK(kefir_hashtree_max(&state->current_allocation, &max_global_node));

    struct kefir_hashtree_node_iterator local_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->conflicts->locally_alive, &local_iter);
         node != NULL; node = kefir_hashtree_next(&local_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->key);
        if (state->allocator->all_global) {
            REQUIRE_OK(do_allocate_var(state, instr_ref));
            continue;
        }

        ASSIGN_DECL_CAST(const struct kefir_opt_code_local_variable_occurences *, occurences, node->value);
        if (!kefir_hashset_has(&state->allocator->alive_variables, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        REQUIRE_OK(kefir_hashtree_clean(state->mem, &state->current_allocation));

        for (res = kefir_hashset_iter(&state->conflicts->globally_alive, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, conflict_instr_ref, entry);
            struct kefir_hashtree_node *node;
            res = kefir_hashtree_at(&state->allocator->variable_locations, (kefir_hashtree_key_t) conflict_instr_ref,
                                    &node);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                res = kefir_hashtree_insert(state->mem, &state->current_allocation, (kefir_hashtree_key_t) node->value,
                                            (kefir_hashtree_value_t) node->key);
                if (res != KEFIR_ALREADY_EXISTS) {
                    REQUIRE_OK(res);
                }
            }
        }

        for (res = kefir_hashset_iter(&occurences->occurences, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, entry);
            REQUIRE_OK(kefir_hashtree_at(&state->conflicts->block_alive_vars, (kefir_hashtree_key_t) block_id, &node));
            ASSIGN_DECL_CAST(const struct kefir_opt_code_block_local_variables *, block_local_variables,
                node->value);

            struct kefir_hashset_iterator var_iter;
            for (res = kefir_hashset_iter(&block_local_variables->variables, &var_iter, &entry); res == KEFIR_OK;
                res = kefir_hashset_next(&var_iter, &entry)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, conflict_instr_ref, entry);
                if (instr_ref != conflict_instr_ref) {
                    struct kefir_hashtree_node *node;
                    res = kefir_hashtree_at(&state->allocator->variable_locations, (kefir_hashtree_key_t) conflict_instr_ref,
                                            &node);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        res = kefir_hashtree_insert(state->mem, &state->current_allocation, (kefir_hashtree_key_t) node->value,
                                                    (kefir_hashtree_value_t) node->key);
                        if (res != KEFIR_ALREADY_EXISTS) {
                            REQUIRE_OK(res);
                        }
                    }
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(do_allocate_var(state, instr_ref));
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
    const struct kefir_opt_code_variable_conflicts *conflicts) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator"));
    REQUIRE(hooks != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator hooks"));
    REQUIRE(conflicts != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid local variable conflicts"));

    REQUIRE_OK(kefir_hashtree_clean(mem, &allocator->variable_locations));
    allocator->total_size = 0;
    allocator->total_alignment = 0;

    struct allocator_state state = {
        .mem = mem, .allocator = allocator, .code = code, .hooks = hooks, .conflicts = conflicts};
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
