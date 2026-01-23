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
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t insert_before_ref) {
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

    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));

        if (user_instr->operation.opcode == code->klass->phi_opcode) {
            kefir_codegen_target_ir_value_ref_t linked_value_ref;
            res = kefir_codegen_target_ir_code_phi_link_for(code, use_instr_ref, block_ref, &linked_value_ref);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            if (linked_value_ref.instr_ref == value_ref.instr_ref &&
                linked_value_ref.aspect == value_ref.aspect) {
                REQUIRE_OK(kefir_codegen_target_ir_code_phi_drop(mem, code, use_instr_ref, block_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, use_instr_ref, block_ref, (kefir_codegen_target_ir_value_ref_t) {
                    .instr_ref = rematerialized_instr_ref,
                    .aspect = value_ref.aspect
                }));
            }
        } else {
            if (user_instr->block_ref != block_ref ||
                used_value_ref.aspect != value_ref.aspect) {
                continue;
            }

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value_in(mem, code, use_instr_ref,
                (kefir_codegen_target_ir_value_ref_t) {
                    .instr_ref = rematerialized_instr_ref,
                    .aspect = value_ref.aspect
                }, value_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_is_rematerializable(const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_bool_t *is_rematerializable) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(is_rematerializable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    *is_rematerializable = false;

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
        case KEFIR_TARGET_IR_AMD64_OPCODE(shufps):
        case KEFIR_TARGET_IR_AMD64_OPCODE(shufpd):
            // Intentionally left blank
            break;

        default:
            *is_rematerializable = false;
            return KEFIR_OK;
    }

    kefir_bool_t all_live_in = true;
    for (kefir_size_t j = 0; all_live_in && j < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; j++) {
        kefir_codegen_target_ir_value_ref_t value_refs[2] = {
            [0].instr_ref = KEFIR_ID_NONE,
            [1].instr_ref = KEFIR_ID_NONE
        };
        switch (instr->operation.parameters[j].type) {
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
                value_refs[0] = instr->operation.parameters[j].direct.value_ref;
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
                if (instr->operation.opcode != KEFIR_TARGET_IR_AMD64_OPCODE(lea)) {
                    all_live_in = false;
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
                all_live_in = false;
                break;
        }

        for (kefir_size_t k = 0; all_live_in && k < sizeof(value_refs) / sizeof(value_refs[0]); k++) {
            if (value_refs[k].instr_ref == KEFIR_ID_NONE) {
                continue;
            }

            kefir_bool_t live_in = false;
            for (kefir_size_t l = 0; !live_in && l < liveness->blocks[block_ref].live_in.length; l++) {
                if (liveness->blocks[block_ref].live_in.content[l].instr_ref == value_refs[k].instr_ref &&
                    liveness->blocks[block_ref].live_in.content[l].aspect == value_refs[k].aspect) {
                    live_in = true;
                }
            }
            all_live_in = all_live_in && live_in;
        }
    }
    *is_rematerializable = all_live_in;
    return KEFIR_OK;
}

static kefir_result_t rematerialize_block(struct kefir_mem *mem,
    struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness,
    const struct kefir_codegen_target_ir_regalloc *regalloc,
    kefir_codegen_target_ir_block_ref_t block_ref,
    struct kefir_hashset *alive_values) {
    UNUSED(mem);
    UNUSED(control_flow);

    const struct kefir_codegen_target_ir_liveness_value_block_ranges *liveness_ranges;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(mem, control_flow, liveness, block_ref, &liveness_ranges));
    for (kefir_size_t i = 0; i < liveness->blocks[block_ref].live_in.length; i++) {
        kefir_codegen_target_ir_value_ref_t value_ref = liveness->blocks[block_ref].live_in.content[i];
        if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(value_ref.aspect)) {
            continue;
        }

        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        kefir_result_t res = kefir_codegen_target_ir_regalloc_get(regalloc, value_ref, &allocation);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        kefir_bool_t is_evictable;
        REQUIRE_OK(regalloc->klass->is_evictable(allocation, &is_evictable, regalloc->klass->payload));
        if (is_evictable) {
            continue;
        }

        kefir_bool_t is_rematerializable = false;
        REQUIRE_OK(kefir_codegen_target_ir_amd64_is_rematerializable(code, liveness, value_ref, block_ref, &is_rematerializable));
        if (!is_rematerializable) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

        kefir_codegen_target_ir_instruction_ref_t first_use_ref = KEFIR_ID_NONE;
        kefir_size_t first_use_seq_idx = 0;

        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t used_value_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &used_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
            const struct kefir_codegen_target_ir_instruction *user_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
            if (user_instr->block_ref != block_ref ||
                used_value_ref.aspect != value_ref.aspect ||
                user_instr->operation.opcode == code->klass->phi_opcode ||
                user_instr->operation.opcode == code->klass->upsilon_opcode ||
                user_instr->operation.opcode == code->klass->touch_opcode ||
                user_instr->operation.opcode == code->klass->inline_asm_opcode) {
                continue;
            }

            kefir_size_t seq_idx;
            res = kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, use_instr_ref, &seq_idx);
            if (res == KEFIR_NOT_FOUND || res == KEFIR_OUT_OF_BOUNDS) {
                first_use_ref = KEFIR_ID_NONE;
                kefir_clear_error();
                res = KEFIR_OK;
                break;
            }
            REQUIRE_OK(res);

            if (first_use_ref == KEFIR_ID_NONE ||
                seq_idx < first_use_seq_idx) {
                first_use_ref = use_instr_ref;
                first_use_seq_idx = seq_idx;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (first_use_ref == KEFIR_ID_NONE) {
            continue;
        }

        REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, KEFIR_ID_NONE, liveness_ranges, alive_values));
        for (kefir_codegen_target_ir_instruction_ref_t iter_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
            iter_ref != first_use_ref;
            iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
            res = kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, iter_ref, liveness_ranges, alive_values);
            if (res == KEFIR_NOT_FOUND || res == KEFIR_OUT_OF_BOUNDS) {
                kefir_clear_error();
                res = KEFIR_OK;
            }
            REQUIRE_OK(res);
        }

        kefir_bool_t has_alive_flags = false;
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator value_iter;
        for (res = kefir_hashset_iter(alive_values, &value_iter, &entry); res == KEFIR_OK && !has_alive_flags;
            res = kefir_hashset_next(&value_iter, &entry)) {
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

        if (has_alive_flags) {
            continue;
        }

        REQUIRE_OK(do_rematerialize(mem, code, value_ref, block_ref, first_use_ref));
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

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) &&
            !kefir_codegen_target_ir_code_is_gate_block(code, block_ref)) {
            struct kefir_hashset alive_values;
            REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));
            kefir_result_t res = rematerialize_block(mem, code, control_flow, liveness, regalloc, block_ref, &alive_values);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_hashset_free(mem, &alive_values);
                return res;
            });
            REQUIRE_OK(kefir_hashset_free(mem, &alive_values));
        }
    }

    return KEFIR_OK;
}
