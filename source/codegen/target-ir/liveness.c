#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_target_ir_liveness_init(struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR liveness"));

    liveness->code = NULL;
    liveness->blocks = NULL;
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_liveness_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    if (liveness->blocks != NULL) {
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
            kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].live_in));
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].live_out));
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_codegen_target_ir_liveness));
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_instr_liveness(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_instruction_ref_t instr_ref, struct kefir_list *queue, struct kefir_hashset *visited) {
    REQUIRE_OK(kefir_list_clear(mem, queue));
    REQUIRE_OK(kefir_hashset_clear(mem, visited));
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t user_instr_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(control_flow->code, &use_iter, instr_ref, &user_instr_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &user_instr_ref)) {
        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, user_instr_ref, &user_instr));
        if (instr->block_ref != user_instr->block_ref) {
            if (user_instr->operation.opcode != control_flow->code->klass->phi_opcode) {
                REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) user_instr->block_ref));
                REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[user_instr->block_ref].live_in, (kefir_hashset_key_t) instr_ref));
            } else {
                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(control_flow->code, &iter, user_instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                    if (link_value_ref.instr_ref == instr_ref) {
                        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) link_block_ref));
                        REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[link_block_ref].live_out, (kefir_hashset_key_t) instr_ref));
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref,
            (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));

        if (block_ref == instr->block_ref ||
            (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE) ||
            kefir_hashset_has(visited, (kefir_hashset_key_t) block_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) block_ref));
        REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[block_ref].live_in, (kefir_hashset_key_t) instr_ref));

        struct kefir_hashtreeset_iterator predecessor_iter;
        kefir_result_t res;
        for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].predecessors, &predecessor_iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&predecessor_iter)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref,
                predecessor_iter.entry);
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) predecessor_block_ref));
            REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[predecessor_block_ref].live_out, (kefir_hashset_key_t) instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_build(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(liveness->blocks == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Target IR liveness has already been built"));

    liveness->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_block_liveness) * kefir_codegen_target_ir_code_block_count(control_flow->code));
    REQUIRE(liveness->blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR liveness information"));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);

        kefir_result_t res = KEFIR_OK;
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].live_in, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].live_out, &kefir_hashtable_uint_ops));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, liveness->blocks);
            return res;
        });
    }
    liveness->code = control_flow->code;
    
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
        if (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(liveness->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(liveness->code, instr_ref)) {
            kefir_result_t res = propagate_instr_liveness(mem, control_flow, liveness, instr_ref, &queue, &visited);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_list_free(mem, &queue);
                kefir_hashset_clear(mem, &visited);
                return res;
            });
        }
    }
    kefir_result_t res = kefir_hashset_clear(mem, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_clear(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}
