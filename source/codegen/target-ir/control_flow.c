#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_control_flow_init(struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR control flow"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    control_flow->code = code;
    control_flow->blocks = NULL;
    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_sources, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_targets, &kefir_hashtree_uint_ops));
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
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_ASMCMP_LABEL_BASIS:
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
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_ASMCMP:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_ASMCMP_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_STASH_INDEX:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INLINE_ASSEMBLY_INDEX:
            // Intentionally left blank
            break;
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
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            control_flow->blocks = NULL;
            return res;
        });

        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
    }

    for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_at(control_flow->code, i);
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

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_control_flow_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    if (control_flow->blocks != NULL) {
        for (kefir_size_t i = 0; i < control_flow->code->blocks_length; i++) {
            REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->blocks[i].successors));
        }
        KEFIR_FREE(mem, control_flow->blocks);
    }
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