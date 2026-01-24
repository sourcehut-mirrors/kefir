/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t do_rematerialize(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t insert_before_ref,
    kefir_codegen_target_ir_value_ref_t *rematerialized_value_ref) {
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));
    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
    struct kefir_codegen_target_ir_operation operation = instr->operation;

    kefir_codegen_target_ir_instruction_ref_t rematerialized_instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
        kefir_codegen_target_ir_code_control_prev(code, insert_before_ref),
        &operation,  &metadata, &rematerialized_instr_ref));

    const struct kefir_codegen_target_ir_value_type *value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_iterator instr_value_iter;
    kefir_codegen_target_ir_value_ref_t instr_value_ref;
    for (res = kefir_codegen_target_ir_code_value_iter(code, &instr_value_iter, value_ref.instr_ref, &instr_value_ref, &value_type);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&instr_value_iter, &instr_value_ref, &value_type)) {
        struct kefir_codegen_target_ir_value_type value_type_copy = *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = rematerialized_instr_ref,
            .aspect = instr_value_ref.aspect
        }, &value_type_copy));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, value_ref.instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_add_instruction_attribute(mem, code, rematerialized_instr_ref, attribute));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    *rematerialized_value_ref = (kefir_codegen_target_ir_value_ref_t) {
        .instr_ref = rematerialized_instr_ref,
        .aspect = value_ref.aspect
    };
    return KEFIR_OK;
}

static kefir_result_t is_rematerializable_value(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, kefir_bool_t *is_rematerializable) {
    *is_rematerializable = false;
    REQUIRE(!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(value_ref.aspect), KEFIR_OK);
    REQUIRE(!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
    REQUIRE(value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(add):
        case KEFIR_TARGET_IR_AMD64_OPCODE(xadd):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sub):
        case KEFIR_TARGET_IR_AMD64_OPCODE(shl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(shr):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sar):
        case KEFIR_TARGET_IR_AMD64_OPCODE(and):
        case KEFIR_TARGET_IR_AMD64_OPCODE(or):
        case KEFIR_TARGET_IR_AMD64_OPCODE(xor):
        case KEFIR_TARGET_IR_AMD64_OPCODE(not):
        case KEFIR_TARGET_IR_AMD64_OPCODE(neg):
        case KEFIR_TARGET_IR_AMD64_OPCODE(dec):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movzx):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
        case KEFIR_TARGET_IR_AMD64_OPCODE(lea):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul3):
        case KEFIR_TARGET_IR_AMD64_OPCODE(bsf):
        case KEFIR_TARGET_IR_AMD64_OPCODE(bsr):
        case KEFIR_TARGET_IR_AMD64_OPCODE(rol):
        case KEFIR_TARGET_IR_AMD64_OPCODE(bswap):
        case KEFIR_TARGET_IR_AMD64_OPCODE(xorps):
        case KEFIR_TARGET_IR_AMD64_OPCODE(xorpd):
        case KEFIR_TARGET_IR_AMD64_OPCODE(andps):
        case KEFIR_TARGET_IR_AMD64_OPCODE(andnps):
        case KEFIR_TARGET_IR_AMD64_OPCODE(andpd):
        case KEFIR_TARGET_IR_AMD64_OPCODE(andnpd):
        case KEFIR_TARGET_IR_AMD64_OPCODE(orps):
        case KEFIR_TARGET_IR_AMD64_OPCODE(orpd):
            *is_rematerializable = true;
            break;

        default:
            // Intentionally left blank
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t liveness_interval_includes(const struct kefir_codegen_target_ir_liveness *liveness,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_size_t seq_idx,
    kefir_bool_t *includes) {
    kefir_codegen_target_ir_instruction_ref_t begin_ref, end_ref;
    kefir_result_t res = kefir_codegen_target_ir_liveness_value_at(liveness, value_ref, block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, &begin_ref, &end_ref);
    if (res == KEFIR_NOT_FOUND) {
        *includes = false;
        return KEFIR_OK;
    }
    REQUIRE_OK(res);

    kefir_size_t begin_seq_idx = 0, end_seq_idx = ~0ull;
    if (begin_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, begin_ref, &begin_seq_idx));
    }
    if (end_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, end_ref, &end_seq_idx));
    }

    *includes = begin_seq_idx <= seq_idx && seq_idx < end_seq_idx;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_is_rematerializable(const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_bool_t *is_rematerializable) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(is_rematerializable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    *is_rematerializable = false;

    kefir_bool_t is_rematerializable_basic = false;
    REQUIRE_OK(is_rematerializable_value(code, value_ref, &is_rematerializable_basic));
    REQUIRE(is_rematerializable_basic, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

    kefir_size_t seq_idx;
    REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, value_ref.instr_ref, &seq_idx));

    kefir_bool_t all_parameters_alive = true;
    for (kefir_size_t i = 0; all_parameters_alive && i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        switch (instr->operation.parameters[i].type) {
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
                REQUIRE_OK(liveness_interval_includes(liveness, instr->operation.parameters[i].direct.value_ref, block_ref, seq_idx, &all_parameters_alive));
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
                if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(lea)) {
                    if (instr->operation.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS) {
                        REQUIRE_OK(liveness_interval_includes(liveness, instr->operation.parameters[i].indirect.base.value_ref, block_ref, seq_idx, &all_parameters_alive));
                    }
                    if (all_parameters_alive && instr->operation.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF) {
                        REQUIRE_OK(liveness_interval_includes(liveness, instr->operation.parameters[i].indirect.index.value_ref, block_ref, seq_idx, &all_parameters_alive));
                    }
                } else {
                    all_parameters_alive = false;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
                // Intentionally left blank
                break;

            default:
                all_parameters_alive = false;
                break;
        }
    }
    *is_rematerializable = all_parameters_alive;
    return KEFIR_OK;
}

static kefir_result_t distance_between(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr1_ref, kefir_codegen_target_ir_instruction_ref_t instr2_ref, kefir_size_t max_distance, kefir_size_t *distance) {
    *distance = 0;
    for (kefir_codegen_target_ir_instruction_ref_t iter_ref = instr1_ref;
        iter_ref != KEFIR_ID_NONE && *distance < max_distance;
        iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
        REQUIRE(iter_ref != instr2_ref, KEFIR_OK);

        const struct kefir_codegen_target_ir_instruction *iter_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &iter_instr));
        if (iter_instr->operation.opcode != code->klass->touch_opcode &&
            iter_instr->operation.opcode != code->klass->placeholder_opcode &&
            iter_instr->operation.opcode != code->klass->phi_opcode) {
            (*distance)++;
        }
    }
    REQUIRE(*distance == max_distance, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find the distance between instructions"));
    return KEFIR_OK;
}

static kefir_result_t try_rematerialize(struct kefir_mem *mem,
    struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_regalloc *regalloc,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_instruction_ref_t insert_before_ref,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_bool_t only_reuse,
    kefir_codegen_target_ir_value_ref_t *rematerialized_value_ref,
    const struct kefir_hashset *alive_values,
    struct kefir_hashtable *local_rematerializations,
    kefir_bool_t *did_rematerialize) {
    
    kefir_bool_t is_rematerializable = false;
    REQUIRE_OK(is_rematerializable_value(code, value_ref, &is_rematerializable));
    REQUIRE(is_rematerializable, KEFIR_OK);

    kefir_codegen_target_ir_regalloc_allocation_t allocation;
    kefir_result_t res = kefir_codegen_target_ir_regalloc_get(regalloc, value_ref, &allocation);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    kefir_bool_t is_evictable;
    REQUIRE_OK(regalloc->klass->is_evictable(allocation, &is_evictable, regalloc->klass->payload));
    REQUIRE(!is_evictable, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *original_instr, *insert_before_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &original_instr));
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, insert_before_ref, &insert_before_instr));

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        switch (original_instr->operation.parameters[i].type) {
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
                REQUIRE(kefir_hashset_has(alive_values, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&original_instr->operation.parameters[i].direct.value_ref)), KEFIR_OK);
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
                REQUIRE(original_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(lea), KEFIR_OK);
                if (original_instr->operation.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS) {
                    REQUIRE(kefir_hashset_has(alive_values, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&original_instr->operation.parameters[i].indirect.base.value_ref)), KEFIR_OK);
                }
                if (original_instr->operation.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF) {
                    REQUIRE(kefir_hashset_has(alive_values, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&original_instr->operation.parameters[i].indirect.index.value_ref)), KEFIR_OK);
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
                // Intentionally left blank
                break;

            default:
                return KEFIR_OK;
        }
    }

    kefir_hashtable_value_t *table_value = NULL;
    res = kefir_hashtable_at_mut(local_rematerializations, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, rematerialized_instr_ref,
            (*table_value));
        kefir_size_t distance = 0;
        REQUIRE_OK(distance_between(code, rematerialized_instr_ref, insert_before_ref, regalloc->klass->transforms->rematerialization_locality, &distance));
        if (only_reuse || distance < regalloc->klass->transforms->rematerialization_locality) {
            rematerialized_value_ref->instr_ref = rematerialized_instr_ref;
            rematerialized_value_ref->aspect = value_ref.aspect;
            *did_rematerialize = true;
            return KEFIR_OK;
        }
    }
    REQUIRE(!only_reuse, KEFIR_OK);

    if (original_instr->block_ref == insert_before_instr->block_ref) {
        kefir_size_t distance = 0;
        REQUIRE_OK(distance_between(code, value_ref.instr_ref, insert_before_ref, regalloc->klass->transforms->rematerialization_locality, &distance));
        REQUIRE(distance >= regalloc->klass->transforms->rematerialization_locality, KEFIR_OK);
    }

    REQUIRE_OK(do_rematerialize(mem, code, value_ref, block_ref, insert_before_ref, rematerialized_value_ref));
    if (table_value != NULL) {
        *table_value = rematerialized_value_ref->instr_ref;
    } else {
        REQUIRE_OK(kefir_hashtable_insert(mem, local_rematerializations, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref),
            (kefir_hashtable_value_t) rematerialized_value_ref->instr_ref));
    }
    *did_rematerialize = true;
    return KEFIR_OK;
}

static kefir_result_t rematerialize_block(struct kefir_mem *mem,
    struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness,
    const struct kefir_codegen_target_ir_regalloc *regalloc,
    kefir_codegen_target_ir_block_ref_t block_ref,
    struct kefir_hashset *alive_values,
    struct kefir_hashtable *local_rematerializations) {
    REQUIRE_OK(kefir_hashtable_clear(mem, local_rematerializations));
    const struct kefir_codegen_target_ir_liveness_value_block_ranges *liveness_ranges;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(mem, control_flow, liveness, block_ref, &liveness_ranges));

    REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, KEFIR_ID_NONE, liveness_ranges, alive_values));
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {

        kefir_bool_t has_alive_flags = false;
        kefir_result_t res;
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(alive_values, &iter, &entry);
            res == KEFIR_OK && !has_alive_flags;
            res = kefir_hashset_next(&iter, &entry)) {
            kefir_codegen_target_ir_value_ref_t alive_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(entry);
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(alive_value_ref.aspect) &&
                (KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect) == KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)) {
                has_alive_flags = true;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, instr_ref, liveness_ranges, alive_values));

        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
        if (instr->operation.opcode == code->klass->phi_opcode ||
            instr->operation.opcode == code->klass->inline_asm_opcode) {
            // Intentionally left blank
        } else {
            kefir_bool_t do_replace = false;
            struct kefir_codegen_target_ir_operation oper = instr->operation;
            for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
                if (instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                    REQUIRE_OK(try_rematerialize(mem, code, regalloc, block_ref, instr_ref,
                        instr->operation.parameters[i].direct.value_ref, has_alive_flags,
                        &oper.parameters[i].direct.value_ref, alive_values, local_rematerializations,
                        &do_replace));
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
                } else if (instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT) {
                    if (instr->operation.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS) {
                        REQUIRE_OK(try_rematerialize(mem, code, regalloc, block_ref, instr_ref,
                            instr->operation.parameters[i].indirect.base.value_ref, has_alive_flags,
                            &oper.parameters[i].indirect.base.value_ref, alive_values, local_rematerializations,
                            &do_replace));
                        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
                    } else if (instr->operation.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF) {
                        REQUIRE_OK(try_rematerialize(mem, code, regalloc, block_ref, instr_ref,
                            instr->operation.parameters[i].indirect.index.value_ref, has_alive_flags,
                            &oper.parameters[i].indirect.index.value_ref, alive_values, local_rematerializations,
                            &do_replace));
                        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
                    }
                }
            }

            if (do_replace) {
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
            }
        }
    }

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].successors, &iter, &key);
        res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, key);
        
        struct kefir_codegen_target_ir_value_phi_node_iterator phi_iter;
        kefir_codegen_target_ir_instruction_ref_t phi_ref;
        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_iter, successor_block_ref, &phi_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_phi_node_next(&phi_iter, &phi_ref)) {
            kefir_codegen_target_ir_value_ref_t link_value_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(code, phi_ref, block_ref, &link_value_ref));
            
            kefir_codegen_target_ir_value_ref_t rematerialized_value_ref;
            kefir_bool_t did_rematerialize = false;
            REQUIRE_OK(try_rematerialize(mem, code, regalloc, block_ref, kefir_codegen_target_ir_code_block_control_tail(code, block_ref), link_value_ref, false, &rematerialized_value_ref, alive_values, local_rematerializations, &did_rematerialize));
            if (did_rematerialize) {
                REQUIRE_OK(kefir_codegen_target_ir_code_phi_drop(mem, code, phi_ref, block_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, phi_ref, block_ref, rematerialized_value_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_rematerialize(struct kefir_mem *mem,
    struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness,
    const struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    kefir_result_t res;
    struct kefir_hashset alive_values;
    struct kefir_hashtable local_rematerializations;
    REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&local_rematerializations, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) &&
            !kefir_codegen_target_ir_code_is_gate_block(code, block_ref)) {
            res = rematerialize_block(mem, code, control_flow, liveness, regalloc, block_ref, &alive_values, &local_rematerializations);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_hashtable_free(mem, &local_rematerializations);
                kefir_hashset_free(mem, &alive_values);
                return res;
            });
        }
    }
    res = kefir_hashtable_free(mem, &local_rematerializations);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &alive_values);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &alive_values));

    return KEFIR_OK;
}
