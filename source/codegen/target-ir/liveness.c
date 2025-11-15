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
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].use));
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].def));
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].live_in));
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].live_out));
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_codegen_target_ir_liveness));
    }
    return KEFIR_OK;
}

static kefir_result_t update_use_def_operand(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, const struct kefir_codegen_target_ir_operand *operand) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[block_ref].use, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&operand->direct.value_ref)));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[block_ref].use, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&operand->indirect.base.value_ref)));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
    }
    return KEFIR_OK;
}

static kefir_result_t update_use_def(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, const struct kefir_codegen_target_ir_instruction *instr) {
    kefir_result_t res;
    kefir_codegen_target_ir_value_ref_t out_value_ref;
    struct kefir_codegen_target_ir_value_iterator out_iter;
    for (res = kefir_codegen_target_ir_code_value_iter(liveness->code, &out_iter, instr->instr_ref, &out_value_ref, NULL);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&out_iter, &out_value_ref, NULL)) {
        kefir_uint64_t value_encoded = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&out_value_ref);
        REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[block_ref].def, (kefir_hashset_key_t) value_encoded));
        REQUIRE_OK(kefir_hashset_delete(&liveness->blocks[block_ref].use, (kefir_hashset_key_t) value_encoded));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (instr->operation.opcode == liveness->code->klass->phi_opcode) {
        // Intentionally left blank
    } else if (instr->operation.opcode == liveness->code->klass->inline_asm_opcode) {
        struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
        const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
        for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(liveness->code, &iter, instr->instr_ref, &fragment);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                    REQUIRE_OK(update_use_def_operand(mem, liveness, block_ref, &fragment->operand));
                    break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(update_use_def_operand(mem, liveness, block_ref, &instr->operation.parameters[i]));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compute_use_def(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_tail(liveness->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_prev(liveness->code, instr_ref)) {
            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(liveness->code, instr_ref, &instr));
            REQUIRE_OK(update_use_def(mem, liveness, block_ref, instr));
        }
    }
    
    return KEFIR_OK;
}

static kefir_result_t compute_live_in(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *tmp_set, kefir_bool_t *fixpoint_reached) {
    struct kefir_codegen_target_ir_block_liveness *block = &liveness->blocks[block_ref];
    REQUIRE_OK(kefir_hashset_clear(mem, tmp_set));
    REQUIRE_OK(kefir_hashset_merge(mem, tmp_set, &block->live_in));
    REQUIRE_OK(kefir_hashset_clear(mem, &block->live_in));

    REQUIRE_OK(kefir_hashset_merge(mem, &block->live_in, &block->live_out));
    REQUIRE_OK(kefir_hashset_subtract(&block->live_in, &block->def));
    REQUIRE_OK(kefir_hashset_merge(mem, &block->live_in, &block->use));

    if (kefir_hashset_has_difference(&block->live_in, tmp_set)) {
        *fixpoint_reached = false;
    }
    return KEFIR_OK;
}

static kefir_result_t merge_live_out(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_block_ref_t successor_block_ref) {
    struct kefir_codegen_target_ir_block_liveness *block = &liveness->blocks[block_ref];
    struct kefir_codegen_target_ir_block_liveness *successor_block = &liveness->blocks[successor_block_ref];
    REQUIRE_OK(kefir_hashset_merge(mem, &block->live_out, &successor_block->live_in));

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(liveness->code, &phi_node_iter, successor_block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {        
        kefir_codegen_target_ir_value_ref_t linked_value_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(liveness->code, phi_ref, block_ref, &linked_value_ref));
        REQUIRE_OK(kefir_hashset_add(mem, &block->live_out, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&linked_value_ref)));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t compute_live_out(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *tmp_set, kefir_bool_t *fixpoint_reached) {
    struct kefir_codegen_target_ir_block_liveness *block = &liveness->blocks[block_ref];
    REQUIRE_OK(kefir_hashset_clear(mem, tmp_set));
    REQUIRE_OK(kefir_hashset_merge(mem, tmp_set, &block->live_out));
    REQUIRE_OK(kefir_hashset_clear(mem, &block->live_out));

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].successors, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, iter.entry);
        REQUIRE_OK(merge_live_out(mem, liveness, block_ref, successor_block_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (kefir_hashset_has_difference(&block->live_out, tmp_set)) {
        *fixpoint_reached = false;
    }
    return KEFIR_OK;
}

static kefir_result_t compute_fixpoint_impl(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, struct kefir_list *queue, struct kefir_hashset *tmp_set) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
        if (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }

        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) block_ref));
    }

    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref,
            (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));

        kefir_bool_t fixpoint_reached = true;
        REQUIRE_OK(compute_live_in(mem, liveness, block_ref, tmp_set, &fixpoint_reached));
        REQUIRE_OK(compute_live_out(mem, control_flow, liveness, block_ref, tmp_set, &fixpoint_reached));
        if (!fixpoint_reached) {
            kefir_result_t res;
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].predecessors, &iter); res == KEFIR_OK;
                    res = kefir_hashtreeset_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref, iter.entry);
                if (predecessor_block_ref != control_flow->code->entry_block && control_flow->blocks[predecessor_block_ref].immediate_dominator == KEFIR_ID_NONE) {
                    continue;
                }
                REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) predecessor_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compute_fixpoint(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness) {
    struct kefir_list queue;
    struct kefir_hashset tmp_set;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&tmp_set, &kefir_hashtable_uint_ops));

    kefir_result_t res = compute_fixpoint_impl(mem, control_flow, liveness, &queue, &tmp_set);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        kefir_hashset_free(mem, &tmp_set);
        return res;
    });
    res = kefir_list_free(mem, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &tmp_set);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &tmp_set));
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
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].use, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].def, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].live_in, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&liveness->blocks[block_ref].live_out, &kefir_hashtable_uint_ops));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, liveness->blocks);
            return res;
        });
    }
    liveness->code = control_flow->code;

    REQUIRE_OK(compute_use_def(mem, liveness));
    REQUIRE_OK(compute_fixpoint(mem, control_flow, liveness));
    return KEFIR_OK;
}
