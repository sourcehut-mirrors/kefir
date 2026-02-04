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

#include "kefir/optimizer/topological_schedule.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define UNSCHEDULED_INDEX ((kefir_size_t) - 1ll)

struct schedule_instruction_param {
    struct kefir_mem *mem;
    const struct kefir_opt_code_schedule *schedule;
    const struct kefir_opt_code_container *code;
    const struct kefir_opt_code_analysis *code_analysis;
    const struct kefir_opt_code_schedule_builder *schedule_builder;
    const struct kefir_opt_code_topological_scheduler *scheduler;
    kefir_opt_block_id_t block_id;
    struct kefir_list instr_queue;
};

struct update_liveness_param {
    struct schedule_instruction_param *param;
    kefir_size_t linear_index;
};

struct schedule_stack_entry {
    kefir_opt_instruction_ref_t instr_ref;
    kefir_bool_t dependencies_pending;
};

struct push_instruction_input_param {
    struct schedule_instruction_param *param;
    struct kefir_list_entry *insert_position;
};

static kefir_result_t schedule_stack_push(struct schedule_instruction_param *param,
                                          kefir_opt_instruction_ref_t instr_ref, kefir_bool_t dependencies_pending,
                                          struct kefir_list_entry *position) {
    struct schedule_stack_entry *entry = KEFIR_MALLOC(param->mem, sizeof(struct schedule_stack_entry));
    REQUIRE(entry != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code schedule stack entry"));

    entry->instr_ref = instr_ref;
    entry->dependencies_pending = dependencies_pending;
    kefir_result_t res = kefir_list_insert_after(param->mem, &param->instr_queue, position, entry);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(param->mem, entry);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t schedule_live_out(struct schedule_instruction_param *param,
                                        const struct kefir_opt_code_block *block) {
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    const struct kefir_opt_code_control_flow_block *block_props = &param->code_analysis->control_flow.blocks[block->id];
    for (res = kefir_hashset_iter(&block_props->successors, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, entry);
        const struct kefir_opt_code_block *successor_block;
        REQUIRE_OK(kefir_opt_code_container_block(param->code, successor_block_id, &successor_block));
        kefir_opt_phi_id_t phi_ref;
        for (res = kefir_opt_code_block_phi_head(param->code, successor_block, &phi_ref);
             res == KEFIR_OK && phi_ref != KEFIR_ID_NONE; kefir_opt_phi_next_sibling(param->code, phi_ref, &phi_ref)) {
            const struct kefir_opt_phi_node *phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(param->code, phi_ref, &phi_node));
            if (!kefir_hashset_has(&param->code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                   (kefir_hashset_key_t) phi_node->output_ref)) {
                continue;
            }
            kefir_opt_instruction_ref_t instr_ref2;
            REQUIRE_OK(kefir_opt_code_container_phi_link_for(param->code, phi_ref, block->id, &instr_ref2));
            REQUIRE_OK(schedule_stack_push(param, instr_ref2, true, kefir_list_tail(&param->instr_queue)));
        }
        REQUIRE_OK(res);

        if (successor_block_id != block->id) {
            struct kefir_hashset_iterator alive_instr_iter;
            kefir_hashset_key_t alive_instr_entry;
            for (res = kefir_hashset_iter(&param->code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                          &alive_instr_iter, &alive_instr_entry);
                 res == KEFIR_OK; res = kefir_hashset_next(&alive_instr_iter, &alive_instr_entry)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, alive_instr_ref, alive_instr_entry);
                const struct kefir_opt_instruction *alive_instr;
                REQUIRE_OK(kefir_opt_code_container_instr(param->code, alive_instr_ref, &alive_instr));
                if (alive_instr->block_id != block->id) {
                    continue;
                }
                REQUIRE_OK(schedule_stack_push(param, alive_instr_ref, true, kefir_list_tail(&param->instr_queue)));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t schedule_collect_control_flow(const struct kefir_opt_code_block *block,
                                                    struct schedule_instruction_param *param) {
    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    kefir_opt_instruction_ref_t tail_control_ref;

    for (res = kefir_opt_code_block_instr_head(param->code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(param->code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
            kefir_bool_t is_alive;
            REQUIRE_OK(
                kefir_opt_code_liveness_instruction_is_alive(&param->code_analysis->liveness, instr_ref, &is_alive));
            if (is_alive) {
                REQUIRE_OK(schedule_stack_push(param, instr_ref, true, kefir_list_tail(&param->instr_queue)));
            }
        }
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_instr_head(param->code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(param->code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
            kefir_bool_t is_alive;
            REQUIRE_OK(
                kefir_opt_code_liveness_instruction_is_alive(&param->code_analysis->liveness, instr_ref, &is_alive));
            if (is_alive) {
                REQUIRE_OK(schedule_stack_push(param, instr_ref, true, kefir_list_tail(&param->instr_queue)));
            }
        }
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(param->code, block, &tail_control_ref));
    res = kefir_opt_code_block_instr_control_head(param->code, block, &instr_ref);
    REQUIRE_OK(res);
    if (instr_ref == KEFIR_ID_NONE) {
        REQUIRE_OK(schedule_live_out(param, block));
    }
    for (; res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(param->code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
        if (KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(instr)) {
            kefir_bool_t is_alive;
            REQUIRE_OK(
                kefir_opt_code_liveness_instruction_is_alive(&param->code_analysis->liveness, instr_ref, &is_alive));
            if (!is_alive) {
                continue;
            }
        }

        if (instr_ref == tail_control_ref) {
            REQUIRE_OK(schedule_live_out(param, block));
        }
        REQUIRE_OK(schedule_stack_push(param, instr_ref, true, kefir_list_tail(&param->instr_queue)));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

static kefir_result_t push_instruction_input(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct push_instruction_input_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction schedule parameter"));

    REQUIRE_OK(schedule_stack_push(param->param, instr_ref, true, param->insert_position));
    if (param->insert_position == NULL) {
        param->insert_position = kefir_list_head(&param->param->instr_queue);
    } else {
        kefir_list_next((const struct kefir_list_entry **) &param->insert_position);
    }
    return KEFIR_OK;
}

static kefir_result_t schedule_instructions(struct schedule_instruction_param *param, kefir_opt_block_id_t block_id) {
    kefir_opt_instruction_ref_t instr_ref;
    for (struct kefir_list_entry *iter = kefir_list_head(&param->instr_queue); iter != NULL;
         iter = kefir_list_head(&param->instr_queue)) {
        ASSIGN_DECL_CAST(struct schedule_stack_entry *, entry, iter->value);
        instr_ref = entry->instr_ref;
        const kefir_bool_t dependencies_pending = entry->dependencies_pending;
        REQUIRE_OK(kefir_list_pop(param->mem, &param->instr_queue, iter));

        if (kefir_opt_code_schedule_has(param->schedule, instr_ref)) {
            continue;
        }
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
        if (instr->block_id != block_id) {
            continue;
        }

        if (dependencies_pending) {
            struct push_instruction_input_param push_param = {.param = param, .insert_position = NULL};
            kefir_bool_t schedule_instr = false;
            REQUIRE_OK(param->scheduler->instr_callback(instr_ref, push_instruction_input, &push_param, &schedule_instr,
                                                        param->scheduler->instr_callback_payload));
            if (schedule_instr) {
                REQUIRE_OK(schedule_stack_push(param, instr_ref, false, push_param.insert_position));
            }
        } else {
            REQUIRE_OK(param->schedule_builder->schedule_instruction(param->mem, param->block_id, instr_ref, NULL,
                                                                     param->schedule_builder->payload));
        }
    }

    return KEFIR_OK;
}

static kefir_result_t free_schedule_stack_entry(struct kefir_mem *mem, struct kefir_list *list,
                                                struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL && entry->value, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    KEFIR_FREE(mem, entry->value);
    return KEFIR_OK;
}

static kefir_result_t schedule_block(struct kefir_mem *mem, const struct kefir_opt_code_schedule *schedule,
                                     const struct kefir_opt_code_container *code,
                                     const struct kefir_opt_code_analysis *code_analysis, kefir_opt_block_id_t block_id,
                                     const struct kefir_opt_code_schedule_builder *schedule_builder,
                                     struct kefir_opt_code_topological_scheduler *scheduler) {
    REQUIRE(!kefir_opt_code_schedule_has_block(schedule, block_id), KEFIR_OK);
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    kefir_uint32_t block_linear_index;
    REQUIRE_OK(schedule_builder->schedule_block(mem, block_id, &block_linear_index, schedule_builder->payload));

    struct schedule_instruction_param param = {.mem = mem,
                                               .schedule = schedule,
                                               .code = code,
                                               .code_analysis = code_analysis,
                                               .block_id = block_id,
                                               .schedule_builder = schedule_builder,
                                               .scheduler = scheduler};

    REQUIRE_OK(kefir_list_init(&param.instr_queue));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&param.instr_queue, free_schedule_stack_entry, NULL));
    REQUIRE_CHAIN(&res, schedule_collect_control_flow(block, &param));
    REQUIRE_CHAIN(&res, schedule_instructions(&param, block_id));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &param.instr_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &param.instr_queue));

    const struct kefir_opt_code_control_flow_block *block_props = &code_analysis->control_flow.blocks[block->id];
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(&block_props->successors, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, entry);
        REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, successor_block_id, schedule_builder, scheduler));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t do_schedule(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                  const struct kefir_opt_code_analysis *code_analysis,
                                  const struct kefir_opt_code_schedule *schedule,
                                  struct kefir_opt_code_schedule_builder *schedule_builder, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(code_analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(schedule_builder != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule builder"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_topological_scheduler *, scheduler, payload);
    REQUIRE(scheduler != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code topological scheduler"));

    REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, code->entry_point, schedule_builder, scheduler));
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(&code_analysis->control_flow.indirect_jump_target_blocks, &iter, &entry);
         res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, entry);
        REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, block_id, schedule_builder, scheduler));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_topological_scheduler_default_schedule(
    kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_code_topological_scheduler_instruction_dependency_callback_t dependency_callback,
    void *dependency_callback_payload, kefir_bool_t *schedule_instruction, void *payload) {
    REQUIRE(
        dependency_callback != NULL,
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction dependency scheduler callback"));
    REQUIRE(schedule_instruction != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction scheduler flag"));
    ASSIGN_DECL_CAST(const struct kefir_opt_code_container *, code, payload);
    REQUIRE(code != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code container"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    REQUIRE_OK(
        kefir_opt_instruction_extract_inputs(code, instr, true, dependency_callback, dependency_callback_payload));
    *schedule_instruction = true;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_topological_scheduler_init(
    struct kefir_opt_code_topological_scheduler *scheduler,
    kefir_opt_code_topological_scheduler_instruction_callback_t instr_callback, void *instr_callback_payload) {
    REQUIRE(scheduler != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code topological scheduler"));
    REQUIRE(instr_callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid optimizer code topological scheduler instruction callback"));

    scheduler->scheduler.do_schedule = do_schedule;
    scheduler->scheduler.payload = scheduler;
    scheduler->instr_callback = instr_callback;
    scheduler->instr_callback_payload = instr_callback_payload;
    return KEFIR_OK;
}
