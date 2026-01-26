/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/codegen/target-ir/update.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct replace_state {
    struct kefir_mem *mem;
    struct kefir_codegen_target_ir_code *code;
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    kefir_codegen_target_ir_value_ref_t old_value_ref;
    struct kefir_list queue;
    struct kefir_hashset inserted_phis;
};

static kefir_result_t do_replace(struct replace_state *state) {
    for (struct kefir_list_entry *head = kefir_list_head(&state->queue);
        head != NULL;
        head = kefir_list_head(&state->queue)) {
        kefir_codegen_target_ir_value_ref_t new_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM((kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, head));

        const struct kefir_codegen_target_ir_instruction *new_instr, *old_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, new_value_ref.instr_ref, &new_instr));
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, state->old_value_ref.instr_ref, &old_instr));
        kefir_codegen_target_ir_block_ref_t new_instr_block_ref = new_instr->block_ref;
        kefir_codegen_target_ir_block_ref_t old_instr_block_ref = old_instr->block_ref;

        const struct kefir_codegen_target_ir_value_type *old_value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, state->old_value_ref, &old_value_type));
        struct kefir_codegen_target_ir_value_type old_value_type_copy = *old_value_type;
        struct kefir_codegen_target_ir_instruction_metadata old_instr_metadata = old_instr->metadata;

        kefir_result_t res;
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t use_value_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(state->code, &use_iter, state->old_value_ref.instr_ref, &use_instr_ref, &use_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &use_value_ref)) {
            if (use_value_ref.aspect != state->old_value_ref.aspect ||
                use_instr_ref == new_value_ref.instr_ref) {
                continue;
            }
            
            const struct kefir_codegen_target_ir_instruction *use_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, use_instr_ref, &use_instr));

            if (use_instr->operation.opcode == state->code->klass->phi_opcode) {
                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(state->code, &iter, use_instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;) {
                    kefir_bool_t replace = false;
                    if (link_value_ref.instr_ref == state->old_value_ref.instr_ref && link_value_ref.aspect == state->old_value_ref.aspect) {
                        REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(state->control_flow, link_block_ref, new_instr->block_ref, &replace));
                    }

                    if (replace) {
                        REQUIRE_OK(kefir_codegen_target_ir_code_phi_drop(state->mem, state->code, use_instr_ref, link_block_ref));
                        REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(state->mem, state->code, use_instr_ref, link_block_ref, new_value_ref));
                        res = kefir_codegen_target_ir_code_phi_link_iter(state->code, &iter, use_instr_ref, &link_block_ref, &link_value_ref);
                    } else {
                        res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref);
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            } else {
                kefir_bool_t dominated_by_new_value = false;
                if (use_instr->block_ref != new_instr->block_ref) {
                    REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(state->control_flow, use_instr->block_ref, new_instr->block_ref, &dominated_by_new_value));
                } else {
                    for (kefir_codegen_target_ir_instruction_ref_t iter_ref = new_value_ref.instr_ref;
                        iter_ref != KEFIR_ID_NONE && !dominated_by_new_value;
                        iter_ref = kefir_codegen_target_ir_code_control_next(state->code, iter_ref)) {
                        if (iter_ref == use_instr_ref) {
                            dominated_by_new_value = true;
                        }
                    }
                }

                if (dominated_by_new_value) {
                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_value_in(state->mem, state->code, use_instr_ref, new_value_ref, state->old_value_ref));
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&state->control_flow->blocks[new_instr_block_ref].dominance_frontier, &iter, &entry); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, frontier_block_ref, entry);
            if (frontier_block_ref == old_instr_block_ref ||
                kefir_hashset_has(&state->inserted_phis, (kefir_hashset_key_t) frontier_block_ref)) {
                continue;
            }
            REQUIRE_OK(kefir_hashset_add(state->mem, &state->inserted_phis, (kefir_hashset_key_t) frontier_block_ref));

            kefir_bool_t dominated_by_old_value = false;
            REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(state->control_flow, frontier_block_ref, old_instr_block_ref, &dominated_by_old_value));
            if (!dominated_by_old_value) {
                continue;
            }

            kefir_codegen_target_ir_instruction_ref_t frontier_phi_instr_ref;
            struct kefir_codegen_target_ir_operation operation = {
                .opcode = state->code->klass->phi_opcode,
                .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
                .parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
                .parameters[2].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
                .parameters[3].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, frontier_block_ref,
                KEFIR_ID_NONE,
                &operation, &old_instr_metadata, &frontier_phi_instr_ref));
            kefir_codegen_target_ir_value_ref_t frontier_phi_value_ref = {
                .instr_ref = frontier_phi_instr_ref,
                .aspect = state->old_value_ref.aspect
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(state->mem, state->code, frontier_phi_value_ref, &old_value_type_copy));

            kefir_result_t res;
            struct kefir_hashset_iterator iter;
            kefir_hashset_key_t key;
            for (res = kefir_hashset_iter(&state->control_flow->blocks[frontier_block_ref].predecessors, &iter, &key); res == KEFIR_OK;
                res = kefir_hashset_next(&iter, &key)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref, (kefir_uptr_t) key);
                
                kefir_bool_t dominated_by_new_value = false;
                REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(state->control_flow, predecessor_block_ref, new_instr_block_ref, &dominated_by_new_value));
                if (dominated_by_new_value) {
                    REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(state->mem, state->code, frontier_phi_instr_ref, predecessor_block_ref, new_value_ref));
                } else {
                    REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(state->mem, state->code, frontier_phi_instr_ref, predecessor_block_ref, state->old_value_ref));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&frontier_phi_value_ref)));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_partial_replace_value(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_value_ref_t new_value_ref, kefir_codegen_target_ir_value_ref_t old_value_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    struct replace_state state = {
        .mem = mem,
        .code = code,
        .control_flow = control_flow,
        .old_value_ref = old_value_ref
    };
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashset_init(&state.inserted_phis, &kefir_hashtable_uint_ops));
    kefir_result_t res = kefir_list_insert_after(mem, &state.queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&new_value_ref));
    REQUIRE_CHAIN(&res, do_replace(&state));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.inserted_phis);
        kefir_list_free(mem, &state.queue);
        return res;
    });
    res = kefir_hashset_free(mem, &state.inserted_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &state.queue));
    return KEFIR_OK;
}
