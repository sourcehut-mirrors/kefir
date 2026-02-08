#include "kefir/optimizer/sequencing.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_sequencing_init(struct kefir_opt_code_sequencing *sequencing) {
    REQUIRE(sequencing != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code sequencing"));

    REQUIRE_OK(kefir_hashset_init(&sequencing->sequenced_before, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&sequencing->sequence_numbering, &kefir_hashtree_uint_ops));
    sequencing->next_seq_number = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_sequencing_free(struct kefir_mem *mem, struct kefir_opt_code_sequencing *sequencing) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sequencing != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code sequencing"));

    REQUIRE_OK(kefir_hashtree_free(mem, &sequencing->sequence_numbering));
    REQUIRE_OK(kefir_hashset_free(mem, &sequencing->sequenced_before));
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *, const struct kefir_opt_code_control_flow *,
                                            struct kefir_opt_code_sequencing *, kefir_opt_instruction_ref_t,
                                            kefir_size_t *, struct kefir_hashtreeset *);

struct instr_input_sequence_number_param {
    struct kefir_mem *mem;
    const struct kefir_opt_code_control_flow *control_flow;
    struct kefir_opt_code_sequencing *sequencing;
    kefir_opt_block_id_t block_id;
    struct kefir_hashtreeset *visited;
    kefir_size_t max_seq_number;
};

static kefir_result_t instr_input_sequence_number(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct instr_input_sequence_number_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction sequence number parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->control_flow->code, instr_ref, &instr));
    REQUIRE(instr->block_id == param->block_id, KEFIR_OK);

    kefir_size_t seq_num;
    REQUIRE_OK(
        instr_sequence_number(param->mem, param->control_flow, param->sequencing, instr_ref, &seq_num, param->visited));
    param->max_seq_number = MAX(param->max_seq_number, seq_num);
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *mem,
                                            const struct kefir_opt_code_control_flow *control_flow,
                                            struct kefir_opt_code_sequencing *sequencing,
                                            kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number,
                                            struct kefir_hashtreeset *visited) {
    REQUIRE(!kefir_hashtreeset_has(visited, (kefir_hashtreeset_entry_t) instr_ref),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Detected a loop in optimizer instruction dependencies"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&sequencing->sequence_numbering, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(seq_number, (kefir_size_t) node->value);
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, instr->block_id, &block));

    kefir_bool_t instr_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(control_flow->code, instr_ref, &instr_control_flow));

    if (instr_control_flow) {
#define CONTROL_FLOW_SEQ_STEP (1ull << 32)
        kefir_size_t seq_num = CONTROL_FLOW_SEQ_STEP;
        kefir_opt_instruction_ref_t control_flow_iter;
        for (res = kefir_opt_code_block_instr_control_head(control_flow->code, instr->block_id, &control_flow_iter);
             res == KEFIR_OK && control_flow_iter != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(control_flow->code, control_flow_iter, &control_flow_iter),
            seq_num += CONTROL_FLOW_SEQ_STEP) {
            res = kefir_hashtree_at(&sequencing->sequence_numbering, (kefir_hashtree_key_t) control_flow_iter, &node);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE((kefir_size_t) node->value == seq_num,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Control flow sequence number mismatch"));
                continue;
            }
            REQUIRE_OK(kefir_hashtree_insert(mem, &sequencing->sequence_numbering,
                                             (kefir_hashtree_key_t) control_flow_iter,
                                             (kefir_hashtree_value_t) seq_num));
            if (instr_ref == control_flow_iter) {
                ASSIGN_PTR(seq_number, seq_num);
                return KEFIR_OK;
            }
        }
#undef CONTROL_FLOW_SEQ_STEP
        REQUIRE_OK(res);
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find control flow element in block control flow");
    }

    REQUIRE_OK(kefir_hashtreeset_add(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    struct instr_input_sequence_number_param param = {.mem = mem,
                                                      .control_flow = control_flow,
                                                      .sequencing = sequencing,
                                                      .block_id = block->id,
                                                      .visited = visited,
                                                      .max_seq_number = 0};
    REQUIRE_OK(
        kefir_opt_instruction_extract_inputs(control_flow->code, instr, true, instr_input_sequence_number, &param));

    REQUIRE_OK(kefir_hashtreeset_delete(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    const kefir_size_t instr_seq = param.max_seq_number + 1;
    REQUIRE_OK(kefir_hashtree_insert(mem, &sequencing->sequence_numbering, (kefir_hashtree_key_t) instr_ref,
                                     (kefir_hashtree_value_t) instr_seq));
    ASSIGN_PTR(seq_number, instr_seq);

    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number_impl(struct kefir_mem *mem,
                                                 const struct kefir_opt_code_control_flow *control_flow,
                                                 struct kefir_opt_code_sequencing *sequencing,
                                                 kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number) {
    struct kefir_hashtreeset visited;
    REQUIRE_OK(kefir_hashtreeset_init(&visited, &kefir_hashtree_uint_ops));
    kefir_result_t res = instr_sequence_number(mem, control_flow, sequencing, instr_ref, seq_number, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));
    return KEFIR_OK;
}

static kefir_result_t is_locally_sequenced_before(struct kefir_mem *mem,
                                                  const struct kefir_opt_code_control_flow *control_flow,
                                                  struct kefir_opt_code_sequencing *sequencing,
                                                  kefir_opt_instruction_ref_t instr_ref1,
                                                  kefir_opt_instruction_ref_t instr_ref2, kefir_bool_t *result_ptr) {
    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref2, &instr2));
    REQUIRE(instr1->block_id == instr2->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instructions belong to different optimizer code blocks"));

    kefir_size_t seq1, seq2;
    REQUIRE_OK(instr_sequence_number_impl(mem, control_flow, sequencing, instr_ref1, &seq1));
    REQUIRE_OK(instr_sequence_number_impl(mem, control_flow, sequencing, instr_ref2, &seq2));
    *result_ptr = seq1 < seq2;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_is_sequenced_before(struct kefir_mem *mem,
                                                  const struct kefir_opt_code_control_flow *control_flow,
                                                  struct kefir_opt_code_sequencing *sequencing,
                                                  kefir_opt_instruction_ref_t instr_ref1,
                                                  kefir_opt_instruction_ref_t instr_ref2, kefir_bool_t *result_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(sequencing != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code sequencing"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (instr_ref1 == instr_ref2) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_hashset_key_t entry =
        (((kefir_uint64_t) instr_ref1) << 32) | (((kefir_uint64_t) instr_ref2) & ((1ull << 32) - 1));
    if (kefir_hashset_has(&sequencing->sequenced_before, entry)) {
        *result_ptr = true;
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref2, &instr2));

    if (instr1->block_id == instr2->block_id) {
        REQUIRE_OK(is_locally_sequenced_before(mem, control_flow, sequencing, instr_ref1, instr_ref2, result_ptr));
    } else {
        REQUIRE_OK(
            kefir_opt_code_control_flow_is_dominator(control_flow, instr2->block_id, instr1->block_id, result_ptr));
    }

    if (*result_ptr) {
        REQUIRE_OK(kefir_hashset_add(mem, &sequencing->sequenced_before, entry));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_sequencing_drop_cache(struct kefir_mem *mem,
                                                    struct kefir_opt_code_sequencing *sequencing) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sequencing != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code sequencing"));

    sequencing->next_seq_number = 0;
    REQUIRE_OK(kefir_hashset_clear(mem, &sequencing->sequenced_before));
    REQUIRE_OK(kefir_hashtree_clean(mem, &sequencing->sequence_numbering));
    return KEFIR_OK;
}
