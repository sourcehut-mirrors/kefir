#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/bitset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_block_is_dominator(const struct kefir_opt_code_analysis *analysis,
                                                 kefir_opt_block_id_t dominated_block,
                                                 kefir_opt_block_id_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (analysis->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_block_is_dominator(analysis, analysis->blocks[dominated_block].immediate_dominator,
                                                     dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_control_flow(const struct kefir_opt_code_container *code,
                                                          kefir_opt_instruction_ref_t instr_ref,
                                                          kefir_bool_t *result_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, instr->block_id, &block));

    *result_ptr = block->control_flow.head == instr_ref || instr->control_flow.prev != KEFIR_ID_NONE ||
                  instr->control_flow.next != KEFIR_ID_NONE;
    return KEFIR_OK;
}

struct is_locally_sequenced_before_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_analysis *analysis;
    kefir_opt_instruction_ref_t instr_ref;
    kefir_bool_t *result;
};

static kefir_result_t is_locally_sequenced_before_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct is_locally_sequenced_before_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction sequence parameter"));

    if (*param->result) {
        kefir_bool_t result = false;
        REQUIRE_OK(
            kefir_opt_code_instruction_is_sequenced_before(param->mem, param->analysis, instr_ref, param->instr_ref, &result));
        *param->result = *param->result && result;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_locally_sequenced_before(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                                      kefir_opt_instruction_ref_t instr_ref1,
                                                                      kefir_opt_instruction_ref_t instr_ref2,
                                                                      kefir_bool_t *result_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref2, &instr2));
    REQUIRE(instr1->block_id == instr2->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instructions belong to different optimizer code blocks"));

    kefir_bool_t result = true;

    kefir_bool_t instr1_control_flow, instr2_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(analysis->code, instr_ref1, &instr1_control_flow));
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(analysis->code, instr_ref2, &instr2_control_flow));

    if (instr1_control_flow && instr2_control_flow) {
        kefir_bool_t found = false;
        for (kefir_opt_instruction_ref_t iter = instr_ref2; !found && iter != KEFIR_ID_NONE;) {
            if (iter == instr_ref1) {
                found = true;
            } else {
                REQUIRE_OK(kefir_opt_instruction_prev_control(analysis->code, iter, &iter));
            }
        }
        if (!found) {
            result = false;
        }
    }

    if (result && instr2_control_flow) {
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(
            analysis->code, instr1, true, is_locally_sequenced_before_impl,
            &(struct is_locally_sequenced_before_param) {
                .mem = mem, .analysis = analysis, .instr_ref = instr_ref2, .result = &result}));
    }
    *result_ptr = result;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_sequenced_before(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                              kefir_opt_instruction_ref_t instr_ref1,
                                                              kefir_opt_instruction_ref_t instr_ref2,
                                                              kefir_bool_t *result_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_bucketset_entry_t entry = (((kefir_uint64_t) instr_ref1) << 32) | (((kefir_uint64_t) instr_ref2) & ((1ull << 32) - 1));
    if (kefir_bucketset_has(&analysis->sequenced_before, entry)) {
        *result_ptr = true;
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref2, &instr2));

    if (instr1->block_id == instr2->block_id) {
        REQUIRE_OK(
            kefir_opt_code_instruction_is_locally_sequenced_before(mem, analysis, instr_ref1, instr_ref2, result_ptr));
    } else {
        REQUIRE_OK(kefir_opt_code_block_is_dominator(analysis, instr2->block_id, instr1->block_id, result_ptr));
    }

    if (*result_ptr) {
        REQUIRE_OK(kefir_bucketset_add(mem, &analysis->sequenced_before, entry));
    }
    return KEFIR_OK;
}

struct verify_use_def_payload {
    struct kefir_mem *mem;
    struct kefir_opt_code_analysis *analysis;
    kefir_opt_instruction_ref_t instr_ref;
};

static kefir_result_t verify_use_def_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    kefir_bool_t sequenced_before;
    REQUIRE_OK(kefir_opt_code_instruction_is_sequenced_before(param->mem, param->analysis, instr_ref, param->instr_ref,
                                                              &sequenced_before));
    REQUIRE(sequenced_before, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Reversed use-define chain in optimizer code"));

    return KEFIR_OK;
}

static kefir_result_t verify_use_def(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->analysis->code, instr_ref, &instr));

    REQUIRE_OK(kefir_bucketset_add(param->mem, &param->analysis->blocks[instr->block_id].alive_instr,
                                   (kefir_hashtreeset_entry_t) instr_ref));

    param->instr_ref = instr_ref;
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->analysis->code, instr, true, verify_use_def_impl, payload));

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(param->analysis->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        res = kefir_opt_code_container_instr(param->analysis->code, use_iter.use_instr_ref, &use_instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->analysis->blocks[use_instr->block_id].alive_instr, (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_phi_iter(param->analysis->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_phi_node *use_phi;
        res = kefir_opt_code_container_phi(param->analysis->code, use_iter.use_phi_ref, &use_phi);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        struct kefir_hashtree_node_iterator iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&use_phi->links, &iter); node != NULL;
             node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
            if (src_instr_ref == instr_ref) {
                REQUIRE_OK(kefir_bucketset_add(param->mem, &param->analysis->blocks[src_block_id].alive_instr, (kefir_bucketset_entry_t) instr_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_call_iter(param->analysis->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_call_node *use_call;
        res = kefir_opt_code_container_call(param->analysis->code, use_iter.use_call_ref, &use_call);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->analysis->blocks[use_call->block_id].alive_instr, (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_inline_asm_iter(param->analysis->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_inline_assembly_node *use_inline_asm;
        res = kefir_opt_code_container_inline_assembly(param->analysis->code, use_iter.use_inline_asm_ref,
                                                       &use_inline_asm);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->analysis->blocks[use_inline_asm->block_id].alive_instr, (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_alive_instructions_impl(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis, kefir_opt_block_id_t block_id, struct kefir_bitset *visited_blocks, struct kefir_list *queue) {
    kefir_result_t res;
    struct kefir_bucketset_iterator iter;
    kefir_bucketset_entry_t entry;

    for (res = kefir_bucketset_iter(&analysis->blocks[block_id].alive_instr, &iter, &entry);
        res == KEFIR_OK;
        res = kefir_bucketset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref,
            entry);

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref, &instr));
        if (instr->block_id == block_id) {
            continue;
        }

        REQUIRE_OK(kefir_bitset_clear(visited_blocks));
        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) block_id));

        for (struct kefir_list_entry *iter = kefir_list_head(queue);
            iter != NULL;
            iter = kefir_list_head(queue)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, current_block_id,
                (kefir_uptr_t) iter->value);   
            REQUIRE_OK(kefir_list_pop(mem, queue, iter));

            kefir_bool_t visited;
            REQUIRE_OK(kefir_bitset_get(visited_blocks, current_block_id, &visited));
            if (visited || (block_id != current_block_id && kefir_bucketset_has(&analysis->blocks[current_block_id].alive_instr, (kefir_hashtreeset_entry_t) instr_ref))) {
                continue;
            }

            REQUIRE_OK(kefir_bitset_set(visited_blocks, current_block_id, true));
            REQUIRE_OK(
                kefir_bucketset_add(mem, &analysis->blocks[current_block_id].alive_instr, (kefir_bucketset_entry_t) instr_ref));

            if (current_block_id != instr->block_id) {
                for (const struct kefir_list_entry *iter2 = kefir_list_head(&analysis->blocks[current_block_id].predecessors);
                    iter2 != NULL; kefir_list_next(&iter2)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, pred_block_id, (kefir_uptr_t) iter2->value);
                    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) pred_block_id));
                }
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t propagate_alive_instructions(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));

    struct kefir_bitset visited_blocks;
    struct kefir_list queue;
    REQUIRE_OK(kefir_bitset_init(&visited_blocks));
    REQUIRE_OK(kefir_list_init(&queue));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_bitset_ensure(mem, &visited_blocks, num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; res == KEFIR_OK && block_id < num_of_blocks; block_id++) {
        res = propagate_alive_instructions_impl(mem, analysis, block_id, &visited_blocks, &queue);
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bitset_free(mem, &visited_blocks);
        kefir_list_free(mem, &queue);
        return res;
    });
    res = kefir_bitset_free(mem, &visited_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_trace_use_def(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    struct verify_use_def_payload payload = {.mem = mem, .analysis = analysis, .instr_ref = KEFIR_ID_NONE};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = verify_use_def, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_container_trace(mem, analysis->code, &tracer));
    REQUIRE_OK(kefir_bucketset_clean(mem, &analysis->sequenced_before));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));
    REQUIRE_OK(propagate_alive_instructions(mem, analysis));
    return KEFIR_OK;
}
