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

static kefir_result_t insert_upsilons_step(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashtable *phis, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_result_t res;
    kefir_hashtable_key_t phis_key;
    kefir_hashtable_value_t phis_value;
    struct kefir_hashtable_iterator phis_iter;
    for (res = kefir_hashtable_iter(phis, &phis_iter, &phis_key, &phis_value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&phis_iter, &phis_key, &phis_value)) {
        kefir_codegen_target_ir_value_ref_t phi_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_key);
        kefir_codegen_target_ir_value_ref_t linked_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(phis_value);
        if (phi_value_ref.instr_ref == linked_value_ref.instr_ref && phi_value_ref.aspect == linked_value_ref.aspect) {
            REQUIRE_OK(kefir_hashtable_delete(mem, phis, phis_key));
            return KEFIR_OK;
        }

        kefir_bool_t used_elsewhere = false;
        kefir_hashtable_value_t phis2_value;
        struct kefir_hashtable_iterator phis2_iter;
        for (res = kefir_hashtable_iter(phis, &phis2_iter, NULL, &phis2_value);
            res == KEFIR_OK && !used_elsewhere;
            res = kefir_hashtable_next(&phis2_iter, NULL, &phis2_value)) {
            used_elsewhere = phis2_value == phis_key;
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (!used_elsewhere) {
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
            return KEFIR_OK;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
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
    for (; phis->occupied > 0;) {
        REQUIRE_OK(insert_upsilons_step(mem, code, phis, block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t insert_upsilons(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashtable *phis, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref) ||
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
            REQUIRE_OK(kefir_hashtable_insert(mem, phis, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&link_value_ref)));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(insert_upsilons_into(mem, code, phis, block_ref));
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
