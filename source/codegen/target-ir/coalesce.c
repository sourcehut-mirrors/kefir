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

#include "kefir/codegen/target-ir/coalesce.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static  kefir_result_t free_coalesce_group(struct kefir_mem *mem, struct kefir_list *list,
                                                          struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_coalesce_group *, group,
        entry->value);
    REQUIRE(group != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce group"));

    REQUIRE_OK(kefir_hashset_free(mem, &group->members));
    KEFIR_FREE(mem, group);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_init(struct kefir_codegen_target_ir_coalesce *coalesce) {
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR coalescing"));

    REQUIRE_OK(kefir_hashtable_init(&coalesce->groups, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&coalesce->group_list));
    REQUIRE_OK(kefir_list_on_remove(&coalesce->group_list, free_coalesce_group, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalescing"));

    REQUIRE_OK(kefir_hashtable_free(mem, &coalesce->groups));
    REQUIRE_OK(kefir_list_free(mem, &coalesce->group_list));
    return KEFIR_OK;
}

static kefir_result_t try_coalesce(struct kefir_mem *mem, const struct kefir_codegen_target_ir_interference *interference, struct kefir_codegen_target_ir_coalesce *coalesce,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t other_value_ref) {
    REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref) != KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref), KEFIR_OK);

    if (kefir_hashtable_has(&coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref))) {
        REQUIRE(!kefir_hashtable_has(&coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)), KEFIR_OK);
        REQUIRE_OK(try_coalesce(mem, interference, coalesce, other_value_ref, value_ref));
        return KEFIR_OK;
    }

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);   
    if (res == KEFIR_NOT_FOUND) {
        struct kefir_codegen_target_ir_coalesce_group *group = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_coalesce_group));
        REQUIRE(group != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR coalesce group"));

        res = kefir_hashset_init(&group->members, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &coalesce->group_list, NULL, group));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, group);
            return res;
        });

        REQUIRE_OK(kefir_hashset_add(mem, &group->members, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
        REQUIRE_OK(kefir_hashset_add(mem, &group->members, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref)));
        REQUIRE_OK(kefir_hashtable_insert(mem, &coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) group));
        REQUIRE_OK(kefir_hashtable_insert(mem, &coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref), (kefir_hashtable_value_t) group));
    } else {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_coalesce_group *, group,
            table_value);
        
        kefir_bool_t interferes = false;
        kefir_hashset_key_t key;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&group->members, &iter, &key);
            res == KEFIR_OK && !interferes;
            res = kefir_hashset_next(&iter, &key)) {
            REQUIRE_OK(kefir_codegen_target_ir_interference_has(interference, other_value_ref, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(key), &interferes));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        
        if (!interferes) {
            REQUIRE_OK(kefir_hashset_add(mem, &group->members, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref)));
            REQUIRE_OK(kefir_hashtable_insert(mem, &coalesce->groups, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&other_value_ref), (kefir_hashtable_value_t) group));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t do_coalesce_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_interference *interference, struct kefir_list *queue) {
    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) control_flow->code->entry_block));
    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));
        
        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));

            if (instr->operation.opcode == control_flow->code->klass->phi_opcode) {
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(control_flow->code, instr_ref, 0, &value_ref, NULL);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);

                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(control_flow->code, &iter, instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                    REQUIRE_OK(try_coalesce(mem, interference, coalesce, value_ref, link_value_ref));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            } else if (instr->operation.opcode == control_flow->code->klass->assign_opcode &&
                instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(control_flow->code, instr_ref, 0, &value_ref, NULL);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);

                REQUIRE_OK(try_coalesce(mem, interference, coalesce, instr->operation.parameters[0].direct.value_ref, value_ref));
            } else if (instr->operation.opcode == control_flow->code->klass->upsilon_opcode &&
                instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                REQUIRE_OK(try_coalesce(mem, interference, coalesce, instr->operation.parameters[0].upsilon_ref, instr->operation.parameters[1].direct.value_ref));
            }
        }

        kefir_result_t res;
        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_coalesce_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(coalesce != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalescing"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = do_coalesce_build(mem, coalesce, control_flow, interference, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}
