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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t get_phi_output(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t *value_ref_ptr, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    kefir_codegen_target_ir_value_ref_t value_ref = {
        .instr_ref = instr_ref,
        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
    };
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, value_ref, value_type_ptr);
    if (res == KEFIR_NOT_FOUND) {
        value_ref.aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS;
        res = kefir_codegen_target_ir_code_value_props(code, value_ref, value_type_ptr);
    }
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find target IR phi node output");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(value_ref_ptr, value_ref);
    return KEFIR_OK;
}


static kefir_result_t patch_operand_value(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t phi_value_ref, kefir_codegen_target_ir_value_ref_t *copy_value_ref, kefir_codegen_target_ir_block_ref_t block_ref, const struct kefir_codegen_target_ir_operand *operand) {
    kefir_bool_t patch_value = false;
    kefir_codegen_target_ir_value_ref_t patch_value_ref = {
        .instr_ref = KEFIR_ID_NONE
    };
    if (operand->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&phi_value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&operand->direct.value_ref)) {
        patch_value = true;
        patch_value_ref = operand->direct.value_ref;
    } else if (operand->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
            operand->indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
            KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&phi_value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&operand->indirect.base.value_ref)) {
        patch_value = true;
        patch_value_ref = operand->indirect.base.value_ref;
    }

    if (patch_value) {
        if (copy_value_ref->instr_ref == KEFIR_ID_NONE) {
            kefir_codegen_target_ir_instruction_ref_t copy_instr_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
                kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->assign_opcode,
                    .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .parameters[0].direct.value_ref = patch_value_ref,
                    .parameters[0].direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                }, NULL, &copy_instr_ref));
            copy_value_ref->instr_ref = copy_instr_ref;
            copy_value_ref->aspect = patch_value_ref.aspect;

            const struct kefir_codegen_target_ir_value_type *phi_value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, patch_value_ref, &phi_value_type));
            REQUIRE(phi_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd constrained phi value reference"));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, *copy_value_ref, phi_value_type));
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_value_in(mem, code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref),
            *copy_value_ref, patch_value_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t patch_terminator_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t phi_value_ref, kefir_codegen_target_ir_value_ref_t copy_value_ref, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_codegen_target_ir_instruction_ref_t tail_instr_ref = kefir_codegen_target_ir_code_block_control_tail(code, block_ref);
    const struct kefir_codegen_target_ir_instruction *tail_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, tail_instr_ref, &tail_instr));

    if (tail_instr->operation.opcode == code->klass->phi_opcode) {
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR block terminator instruction");
    } else if (tail_instr->operation.opcode == code->klass->inline_asm_opcode) {
        kefir_result_t res;
        struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
        const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
        for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(code, &iter, tail_instr_ref, &fragment);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND: {
                    REQUIRE_OK(patch_operand_value(mem, code, phi_value_ref, &copy_value_ref, block_ref, &fragment->operand));
                } break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(patch_operand_value(mem, code, phi_value_ref, &copy_value_ref, block_ref, &tail_instr->operation.parameters[i]));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t insert_upsilons_step(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashtable *phis, struct kefir_list *queue, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_result_t res;
    kefir_hashtable_key_t phis_key;
    kefir_hashtable_value_t phis_value;
    struct kefir_hashtable_iterator phis_iter;
    for (res = kefir_hashtable_iter(phis, &phis_iter, &phis_key, NULL);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&phis_iter, &phis_key, NULL)) {
        kefir_codegen_target_ir_value_ref_t phi_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_key);

        kefir_result_t res;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_bool_t used_elsewhere = false;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, phi_value_ref.instr_ref, &use_instr_ref, NULL);
            res == KEFIR_OK && !used_elsewhere;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, NULL)) {
            kefir_codegen_target_ir_value_ref_t user_direct_value_ref = {
                .instr_ref = use_instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            }, user_flag_value_ref = {
                .instr_ref = use_instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS
            };

            kefir_hashtable_value_t table_value;
            res = kefir_hashtable_at(phis, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&user_direct_value_ref), &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                if (table_value == phis_key) {
                    used_elsewhere = true;
                    continue;
                }
            }
            res = kefir_hashtable_at(phis, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&user_flag_value_ref), &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                if (table_value == phis_key) {
                    used_elsewhere = true;
                    continue;
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (!used_elsewhere) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) phis_key));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(queue);
        iter != NULL;
        kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, phis_key,
            (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_hashtable_at(phis, (kefir_hashtable_key_t) phis_key, &phis_value));
        kefir_codegen_target_ir_value_ref_t phi_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_key),
                                            linked_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_value);
                    
        REQUIRE_OK(patch_terminator_instruction(mem, code, phi_value_ref, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = KEFIR_ID_NONE
        }, block_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
            kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
            &(struct kefir_codegen_target_ir_operation) {
                .opcode = code->klass->upsilon_opcode,
                .parameters[0] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON,
                    .upsilon_ref = phi_value_ref
                },
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .direct.value_ref = linked_value_ref,
                    .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                }
            }, NULL, NULL));
        REQUIRE_OK(kefir_hashtable_delete(mem, phis, phis_key));   
    }
    if (kefir_list_length(queue) > 0) {
        REQUIRE_OK(kefir_list_clear(mem, queue));
        return KEFIR_OK;
    }

    REQUIRE_OK(kefir_hashtable_iter(phis, &phis_iter, &phis_key, &phis_value));
    kefir_codegen_target_ir_value_ref_t phi_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_key);
    kefir_codegen_target_ir_value_ref_t linked_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_value);

    kefir_codegen_target_ir_instruction_ref_t copy_instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
        kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->assign_opcode,
            .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
            .parameters[0].direct.value_ref = phi_value_ref,
            .parameters[0].direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
        }, NULL, &copy_instr_ref));
    kefir_codegen_target_ir_value_ref_t copy_value_ref = {
        .instr_ref = copy_instr_ref,
        .aspect = phi_value_ref.aspect
    };
    
    const struct kefir_codegen_target_ir_value_type *phi_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, phi_value_ref, &phi_value_type));
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, copy_value_ref, phi_value_type));

    REQUIRE_OK(patch_terminator_instruction(mem, code, phi_value_ref, copy_value_ref, block_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
        kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_block_control_tail(code, block_ref)),
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->upsilon_opcode,
            .parameters[0] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON,
                .upsilon_ref = phi_value_ref
            },
            .parameters[1] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                .direct.value_ref = linked_value_ref,
                .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }
        }, NULL, NULL));
    REQUIRE_OK(kefir_hashtable_delete(mem, phis, phis_key));

    kefir_hashtable_key_t phis2_key;
    kefir_hashtable_value_t phis2_value;
    struct kefir_hashtable_iterator phis2_iter;
    for (res = kefir_hashtable_iter(phis, &phis2_iter, &phis2_key, &phis2_value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&phis2_iter, &phis2_key, &phis2_value)) {
        if (phis2_value == phis_key) {
            kefir_hashtable_value_t *phis2_value_ptr;
            REQUIRE_OK(kefir_hashtable_at_mut(phis, phis2_key, &phis2_value_ptr));
            *phis2_value_ptr = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&copy_value_ref);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t insert_upsilons_into(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashtable *phis, kefir_codegen_target_ir_block_ref_t block_ref) {
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = KEFIR_OK;
    for (; res == KEFIR_OK && phis->occupied > 0;) {
        REQUIRE_CHAIN(&res, insert_upsilons_step(mem, code, phis, &queue, block_ref));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t insert_upsilons(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashtable *phis, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) ||
            kefir_codegen_target_ir_code_is_gate_block(control_flow->code, block_ref) ||
            kefir_hashset_size(&control_flow->blocks[block_ref].successors) != 1 /* Assuming critical edges are split */) {
            continue;
        }

        struct kefir_hashset_iterator succ_iter;
        kefir_hashset_key_t succ_key;
        REQUIRE_OK(kefir_hashset_iter(&control_flow->blocks[block_ref].successors, &succ_iter, &succ_key));
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref,
            succ_key);

        kefir_result_t res;
        struct kefir_codegen_target_ir_value_phi_node_iterator phi_iter;
        kefir_codegen_target_ir_instruction_ref_t phi_ref;
        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_iter, successor_block_ref, &phi_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_phi_node_next(&phi_iter, &phi_ref)) {
            kefir_codegen_target_ir_value_ref_t value_ref, link_value_ref;
            REQUIRE_OK(get_phi_output(code, phi_ref, &value_ref, NULL));
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(code, value_ref.instr_ref, block_ref, &link_value_ref));

            if (value_ref.instr_ref == link_value_ref.instr_ref && value_ref.aspect == link_value_ref.aspect) {
                continue;
            }
            
            const struct kefir_codegen_target_ir_instruction *linked_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, link_value_ref.instr_ref, &linked_instr));
            if (linked_instr->operation.opcode == code->klass->placeholder_opcode) {
                const struct kefir_codegen_target_ir_value_type *value_type;
                REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, link_value_ref, &value_type));
                if (value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE && value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                    continue;
                }
            }

            REQUIRE_OK(kefir_hashtable_insert(mem, phis, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&link_value_ref)));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (phis->occupied > 0) {
            REQUIRE_OK(insert_upsilons_into(mem, code, phis, block_ref));

            REQUIRE_OK(kefir_hashtable_free(mem, phis));
            REQUIRE_OK(kefir_hashtable_init(phis, &kefir_hashtable_uint_ops));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_insert_upsilons(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashtable phis;
    REQUIRE_OK(kefir_hashtable_init(&phis, &kefir_hashtable_uint_ops));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));

    kefir_result_t res = insert_upsilons(mem, code, &phis, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashtable_free(mem, &phis);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &phis);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &phis));
    return KEFIR_OK;
}
