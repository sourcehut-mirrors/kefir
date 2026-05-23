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

#include "kefir/codegen/target-ir/amd64/topological_scheduler.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct scheduler_payload {
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_loop_collection *loops;
};

static kefir_result_t insert_sorted_by_loop_depth(struct kefir_mem *mem, struct kefir_list *queue,
                                                  const struct kefir_codegen_target_ir_loop_collection *loops,
                                                  kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_uint32_t depth, preheader_depth;
    REQUIRE_OK(kefir_codegen_target_ir_loop_level(loops, block_ref, &depth));
    REQUIRE_OK(kefir_codegen_target_ir_loop_preheader_level(loops, block_ref, &preheader_depth));
    depth = MAX(depth, preheader_depth);
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = iter->next) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, other_block_ref, (kefir_uptr_t) iter->value);

        kefir_uint32_t other_depth, other_preheader_depth;
        REQUIRE_OK(kefir_codegen_target_ir_loop_level(loops, other_block_ref, &other_depth));
        REQUIRE_OK(kefir_codegen_target_ir_loop_preheader_level(loops, other_block_ref, &other_preheader_depth));
        other_depth = MAX(other_depth, other_preheader_depth);

        if (other_depth <= depth) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, iter->prev, (void *) (kefir_uptr_t) block_ref));
            return KEFIR_OK;
        }
    }

    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) block_ref));
    return KEFIR_OK;
}

static kefir_result_t do_schedule_impl(struct kefir_mem *mem,
                                       const struct kefir_codegen_target_ir_code_schedule *schedule,
                                       struct kefir_codegen_target_ir_code_schedule_builder *schedule_builder,
                                       const struct kefir_codegen_target_ir_control_flow *control_flow,
                                       const struct kefir_codegen_target_ir_loop_collection *loops,
                                       struct kefir_list *queue) {

    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_codegen_target_ir_code_schedule_has_block(schedule, block_ref)) {
            continue;
        }
        if (!kefir_codegen_target_ir_code_is_gate_block(schedule->code, block_ref)) {
            REQUIRE_OK(schedule_builder->schedule_block(mem, block_ref, schedule_builder->payload));
        }

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].successors, &iter, &key); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, key);
            REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops, successor_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        kefir_codegen_target_ir_instruction_ref_t block_tail_ref =
            kefir_codegen_target_ir_code_block_control_tail(control_flow->code, block_ref);
        if (block_tail_ref != KEFIR_ID_NONE) {
            const struct kefir_codegen_target_ir_instruction *block_tail;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, block_tail_ref, &block_tail));

            struct kefir_codegen_target_ir_block_terminator_props terminator_props;
            REQUIRE_OK(control_flow->code->klass->is_block_terminator(control_flow->code, block_tail, &terminator_props,
                                                                      control_flow->code->klass->payload));

            if (terminator_props.block_terminator && terminator_props.branch) {
                kefir_codegen_target_ir_block_ref_t lhs_block_ref = terminator_props.target_block_refs[0];
                kefir_codegen_target_ir_block_ref_t rhs_block_ref = terminator_props.target_block_refs[1];
                if (kefir_hashset_has(&control_flow->blocks[lhs_block_ref].successors,
                                      (kefir_hashset_key_t) rhs_block_ref)) {
                    REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops, rhs_block_ref));
                    REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops, lhs_block_ref));
                } else {
                    REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops, lhs_block_ref));
                    REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops, rhs_block_ref));
                }
            } else if (terminator_props.block_terminator && terminator_props.inline_assembly) {
                REQUIRE_OK(insert_sorted_by_loop_depth(mem, queue, loops,
                                                       block_tail->operation.inline_asm_node.target_block_ref));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t do_schedule(struct kefir_mem *mem, const struct kefir_codegen_target_ir_code_schedule *schedule,
                                  struct kefir_codegen_target_ir_code_schedule_builder *schedule_builder,
                                  kefir_codegen_target_ir_block_ref_t entry_point_ref, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));
    REQUIRE(schedule_builder != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule builder"));
    REQUIRE(
        entry_point_ref != KEFIR_ID_NONE && entry_point_ref < kefir_codegen_target_ir_code_block_count(schedule->code),
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule entry point"));
    ASSIGN_DECL_CAST(const struct scheduler_payload *, scheduler_payload, payload);

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res =
        kefir_list_insert_after(mem, &queue, kefir_list_tail(&queue), (void *) (kefir_uptr_t) entry_point_ref);
    REQUIRE_CHAIN(&res, do_schedule_impl(mem, schedule, schedule_builder, scheduler_payload->control_flow,
                                         scheduler_payload->loops, &queue));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_topological_scheduler_init(
    struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_loop_collection *loops,
    struct kefir_codegen_target_ir_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR loop collection"));
    REQUIRE(scheduler != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR scheduler"));

    struct scheduler_payload *payload = KEFIR_MALLOC(mem, sizeof(struct scheduler_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate scheduler payload"));
    payload->control_flow = control_flow;
    payload->loops = loops;

    scheduler->do_schedule = do_schedule;
    scheduler->payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_topological_scheduler_free(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scheduler != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR scheduler"));

    KEFIR_FREE(mem, scheduler->payload);
    return KEFIR_OK;
}
