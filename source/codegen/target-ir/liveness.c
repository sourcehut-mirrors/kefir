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
            KEFIR_FREE(mem, liveness->blocks[block_ref].live_in.content);
            KEFIR_FREE(mem, liveness->blocks[block_ref].live_out.content);
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_codegen_target_ir_liveness));
    }
    return KEFIR_OK;
}

static kefir_result_t add_to_entry(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness_entry *entry, kefir_codegen_target_ir_value_ref_t value_ref) {
    if (entry->length > 0 &&
        entry->content[entry->length - 1].instr_ref == value_ref.instr_ref &&
        entry->content[entry->length - 1].aspect == value_ref.aspect) {
        return KEFIR_OK;
    }
    if (entry->length >= entry->capacity) {
        kefir_size_t new_capacity = MAX(entry->capacity * 9 / 8, 128);
        kefir_codegen_target_ir_value_ref_t *new_content = KEFIR_REALLOC(mem, entry->content, sizeof(kefir_codegen_target_ir_value_ref_t) * new_capacity);
        REQUIRE(new_content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR liveness entry"));
        entry->content = new_content;
        entry->capacity = new_capacity;
    }

    entry->content[entry->length++] = value_ref;
    return KEFIR_OK;
}

static kefir_result_t propagate_instr_liveness(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_list *queue, kefir_uint8_t *visited_map) {
    REQUIRE_OK(kefir_list_clear(mem, queue));
    memset(visited_map, 0, sizeof(kefir_uint8_t) * kefir_codegen_target_ir_code_block_count(control_flow->code));
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, value_ref.instr_ref, &instr));
    if (instr->operation.opcode == control_flow->code->klass->placeholder_opcode) {
        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(control_flow->code, value_ref, &value_type));
        if (value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE && value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            return KEFIR_OK;
        }
    }

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t user_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value;
    for (res = kefir_codegen_target_ir_code_use_iter(control_flow->code, &use_iter, value_ref.instr_ref, &user_instr_ref, &used_value);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &user_instr_ref, &used_value)) {
        if (used_value.aspect != value_ref.aspect) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, user_instr_ref, &user_instr));
        if (instr->block_ref != user_instr->block_ref || user_instr->operation.opcode == control_flow->code->klass->phi_opcode) {
            if (user_instr->operation.opcode != control_flow->code->klass->phi_opcode) {
                REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) user_instr->block_ref));
                REQUIRE_OK(add_to_entry(mem, &liveness->blocks[user_instr->block_ref].live_in, value_ref));
            } else {
                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(control_flow->code, &iter, user_instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                    if (link_value_ref.instr_ref == value_ref.instr_ref && link_value_ref.aspect == value_ref.aspect) {
                        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) link_block_ref));
                        REQUIRE_OK(add_to_entry(mem, &liveness->blocks[link_block_ref].live_out, value_ref));
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

        if (block_ref == instr->block_ref || visited_map[block_ref] ||
            (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE)) {
            continue;
        }
        visited_map[block_ref] = true;
        REQUIRE_OK(add_to_entry(mem, &liveness->blocks[block_ref].live_in, value_ref));

        struct kefir_hashtreeset_iterator predecessor_iter;
        kefir_result_t res;
        for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].predecessors, &predecessor_iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&predecessor_iter)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref,
                predecessor_iter.entry);
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) predecessor_block_ref));
            REQUIRE_OK(add_to_entry(mem, &liveness->blocks[predecessor_block_ref].live_out, value_ref));
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

        liveness->blocks[block_ref].live_in.content = NULL;
        liveness->blocks[block_ref].live_in.length = 0;
        liveness->blocks[block_ref].live_in.capacity = 0;
        liveness->blocks[block_ref].live_out.content = NULL;
        liveness->blocks[block_ref].live_out.length = 0;
        liveness->blocks[block_ref].live_out.capacity = 0;
    }
    liveness->code = control_flow->code;
    
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_uint8_t *visited_map = KEFIR_MALLOC(mem, sizeof(kefir_uint8_t) * kefir_codegen_target_ir_code_block_count(liveness->code));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
        if (block_ref != control_flow->code->entry_block && control_flow->blocks[block_ref].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(liveness->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(liveness->code, instr_ref)) {
            struct kefir_codegen_target_ir_value_iterator value_iter;
            struct kefir_codegen_target_ir_value_ref value_ref;
            kefir_result_t res;
            for (res = kefir_codegen_target_ir_code_value_iter(liveness->code, &value_iter, instr_ref, &value_ref, NULL);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
                res = propagate_instr_liveness(mem, control_flow, liveness, value_ref, &queue, visited_map);
                REQUIRE_ELSE(res == KEFIR_OK, {
                    KEFIR_FREE(mem, visited_map);
                    kefir_list_free(mem, &queue);
                    return res;
                });
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    KEFIR_FREE(mem, visited_map);
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}
