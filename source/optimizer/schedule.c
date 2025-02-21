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

#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define UNSCHEDULED_INDEX ((kefir_size_t) - 1ll)

struct schedule_instruction_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_schedule *schedule;
    const struct kefir_opt_code_container *code;
    const struct kefir_opt_code_instruction_scheduler *scheduler;
    struct kefir_opt_code_block_schedule *block_schedule;
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

static kefir_result_t free_block_schedule(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                          kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(payload);
    UNUSED(key);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_block_schedule *, schedule, value);
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer block schedule"));

    REQUIRE_OK(kefir_list_free(mem, &schedule->instructions));
    KEFIR_FREE(mem, schedule);
    return KEFIR_OK;
}

static kefir_result_t free_instruction_schedule(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(payload);
    UNUSED(key);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_instruction_schedule *, schedule, value);
    REQUIRE(schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction schedule"));

    KEFIR_FREE(mem, schedule);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_init(struct kefir_opt_code_schedule *schedule) {
    REQUIRE(schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code schedule"));

    REQUIRE_OK(kefir_hashtree_init(&schedule->instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&schedule->instructions, free_instruction_schedule, NULL));
    REQUIRE_OK(kefir_hashtree_init(&schedule->blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&schedule->blocks, free_block_schedule, NULL));
    REQUIRE_OK(kefir_hashtree_init(&schedule->blocks_by_index, &kefir_hashtree_uint_ops));
    schedule->next_block_index = 0;
    schedule->next_instruction_index = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_free(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));

    REQUIRE_OK(kefir_hashtree_free(mem, &schedule->blocks_by_index));
    REQUIRE_OK(kefir_hashtree_free(mem, &schedule->blocks));
    REQUIRE_OK(kefir_hashtree_free(mem, &schedule->instructions));
    return KEFIR_OK;
}

static kefir_result_t trace_instruction_inputs(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct schedule_instruction_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction schedule parameter"));

    if (!kefir_hashtree_has(&param->schedule->instructions, (kefir_hashtree_key_t) instr_ref)) {
        REQUIRE_OK(kefir_list_insert_after(param->mem, &param->instr_queue, kefir_list_tail(&param->instr_queue),
                                           (void *) (kefir_uptr_t) instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t trace_instruction(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct schedule_instruction_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction schedule parameter"));

    REQUIRE(!kefir_hashtree_has(&param->schedule->instructions, (kefir_hashtree_key_t) instr_ref), KEFIR_OK);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->code, instr, true, trace_instruction_inputs, param));

    struct kefir_opt_code_instruction_schedule *instruction_schedule =
        KEFIR_MALLOC(param->mem, sizeof(struct kefir_opt_code_instruction_schedule));
    REQUIRE_ELSE(instruction_schedule != NULL,
                 KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate instruction schedule"));
    instruction_schedule->linear_index = UNSCHEDULED_INDEX;
    instruction_schedule->liveness_range.begin_index = UNSCHEDULED_INDEX;
    instruction_schedule->liveness_range.end_index = UNSCHEDULED_INDEX;
    kefir_result_t res =
        kefir_hashtree_insert(param->mem, &param->schedule->instructions, (kefir_hashtree_key_t) instr_ref,
                              (kefir_hashtree_value_t) instruction_schedule);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(param->mem, instruction_schedule);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t block_collect_control_flow(const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_code_analysis *code_analysis,
                                                 const struct kefir_opt_code_block *block,
                                                 struct schedule_instruction_param *param) {
    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;

    const struct kefir_opt_code_structure_block *block_props = &code_analysis->structure.blocks[block->id];

    for (res = kefir_opt_code_block_instr_control_head(code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (!KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(instr)) {
            REQUIRE_OK(kefir_list_insert_after(param->mem, &param->instr_queue, kefir_list_tail(&param->instr_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
        }
    }
    REQUIRE_OK(res);

    for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        const struct kefir_opt_code_block *successor_block;
        REQUIRE_OK(kefir_opt_code_container_block(code, successor_block_id, &successor_block));
        kefir_opt_phi_id_t phi_ref;
        for (res = kefir_opt_code_block_phi_head(code, successor_block, &phi_ref);
             res == KEFIR_OK && phi_ref != KEFIR_ID_NONE; kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {
            const struct kefir_opt_phi_node *phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi_node));
            if (!kefir_bucketset_has(&code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                     (kefir_bucketset_entry_t) phi_node->output_ref)) {
                continue;
            }
            kefir_opt_instruction_ref_t instr_ref2;
            REQUIRE_OK(kefir_opt_code_container_phi_link_for(code, phi_ref, block->id, &instr_ref2));
            REQUIRE_OK(kefir_list_insert_after(param->mem, &param->instr_queue, kefir_list_tail(&param->instr_queue),
                                               (void *) (kefir_uptr_t) instr_ref2));
        }
        REQUIRE_OK(res);

        struct kefir_bucketset_iterator alive_instr_iter;
        kefir_bucketset_entry_t alive_instr_entry;
        for (res = kefir_bucketset_iter(&code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                        &alive_instr_iter, &alive_instr_entry);
             res == KEFIR_OK; res = kefir_bucketset_next(&alive_instr_iter, &alive_instr_entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, alive_instr_ref, alive_instr_entry);
            const struct kefir_opt_instruction *alive_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(code, alive_instr_ref, &alive_instr));
            if (alive_instr->block_id != block->id) {
                continue;
            }
            REQUIRE_OK(kefir_list_insert_after(param->mem, &param->instr_queue, kefir_list_tail(&param->instr_queue),
                                               (void *) (kefir_uptr_t) alive_instr_ref));
        }
    }

    return KEFIR_OK;
}

static kefir_result_t trace_block(struct schedule_instruction_param *param) {
    kefir_opt_instruction_ref_t instr_ref;
    for (struct kefir_list_entry *iter = kefir_list_head(&param->instr_queue); iter != NULL;
         iter = kefir_list_head(&param->instr_queue)) {
        instr_ref = (kefir_opt_instruction_ref_t) (kefir_uptr_t) iter->value;
        REQUIRE_OK(kefir_list_pop(param->mem, &param->instr_queue, iter));
        REQUIRE_OK(trace_instruction(instr_ref, param));
    }

    return KEFIR_OK;
}

static kefir_result_t update_liveness(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct update_liveness_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction schedule parameter"));

    struct kefir_hashtree_node *node;
    kefir_result_t res =
        kefir_hashtree_at(&param->param->schedule->instructions, (kefir_hashtree_key_t) instr_ref, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_instruction_schedule *, instruction_schedule, node->value);

    if (instruction_schedule->liveness_range.begin_index != UNSCHEDULED_INDEX) {
        instruction_schedule->liveness_range.begin_index =
            MIN(param->linear_index, instruction_schedule->liveness_range.begin_index);
    } else {
        instruction_schedule->liveness_range.begin_index = param->linear_index;
    }
    if (instruction_schedule->liveness_range.end_index != UNSCHEDULED_INDEX) {
        instruction_schedule->liveness_range.end_index =
            MAX(param->linear_index + 1, instruction_schedule->liveness_range.end_index);
    } else {
        instruction_schedule->liveness_range.end_index = param->linear_index + 1;
    }
    return KEFIR_OK;
}

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

static kefir_result_t schedule_collect_control_flow(struct kefir_opt_code_schedule *schedule,
                                                    const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_code_analysis *code_analysis,
                                                    const struct kefir_opt_code_block *block,
                                                    struct schedule_instruction_param *param) {
    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    kefir_opt_instruction_ref_t tail_control_ref;

    const struct kefir_opt_code_structure_block *block_props = &code_analysis->structure.blocks[block->id];

    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
            kefir_hashtree_has(&schedule->instructions, (kefir_hashtree_key_t) instr_ref)) {
            REQUIRE_OK(schedule_stack_push(param, instr_ref, true, kefir_list_tail(&param->instr_queue)));
        }
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
            kefir_hashtree_has(&schedule->instructions, (kefir_hashtree_key_t) instr_ref)) {
            REQUIRE_OK(schedule_stack_push(param, instr_ref, true, kefir_list_tail(&param->instr_queue)));
        }
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &tail_control_ref));
    for (res = kefir_opt_code_block_instr_control_head(code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(code, instr_ref, &instr_ref)) {

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(instr) &&
            !kefir_hashtree_has(&schedule->instructions, (kefir_hashtree_key_t) instr_ref)) {
            continue;
        }

        if (instr_ref == tail_control_ref) {
            for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->successors); iter != NULL;
                 kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
                const struct kefir_opt_code_block *successor_block;
                REQUIRE_OK(kefir_opt_code_container_block(code, successor_block_id, &successor_block));
                kefir_opt_phi_id_t phi_ref;
                for (res = kefir_opt_code_block_phi_head(code, successor_block, &phi_ref);
                     res == KEFIR_OK && phi_ref != KEFIR_ID_NONE; kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {
                    const struct kefir_opt_phi_node *phi_node;
                    REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi_node));
                    if (!kefir_bucketset_has(&code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                             (kefir_bucketset_entry_t) phi_node->output_ref)) {
                        continue;
                    }
                    kefir_opt_instruction_ref_t instr_ref2;
                    REQUIRE_OK(kefir_opt_code_container_phi_link_for(code, phi_ref, block->id, &instr_ref2));
                    REQUIRE_OK(schedule_stack_push(param, instr_ref2, true, kefir_list_tail(&param->instr_queue)));
                }
                REQUIRE_OK(res);

                if (successor_block_id != block->id) {
                    struct kefir_bucketset_iterator alive_instr_iter;
                    kefir_bucketset_entry_t alive_instr_entry;
                    for (res = kefir_bucketset_iter(&code_analysis->liveness.blocks[successor_block_id].alive_instr,
                                                    &alive_instr_iter, &alive_instr_entry);
                         res == KEFIR_OK; res = kefir_bucketset_next(&alive_instr_iter, &alive_instr_entry)) {
                        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, alive_instr_ref, alive_instr_entry);
                        const struct kefir_opt_instruction *alive_instr;
                        REQUIRE_OK(kefir_opt_code_container_instr(code, alive_instr_ref, &alive_instr));
                        if (alive_instr->block_id != block->id) {
                            continue;
                        }
                        REQUIRE_OK(
                            schedule_stack_push(param, alive_instr_ref, true, kefir_list_tail(&param->instr_queue)));
                    }
                }
            }
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

        struct kefir_hashtree_node *node;
        REQUIRE_OK(kefir_hashtree_at(&param->schedule->instructions, (kefir_hashtree_key_t) instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_opt_code_instruction_schedule *, instruction_schedule, node->value);
        if (instruction_schedule->linear_index != UNSCHEDULED_INDEX) {
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
            REQUIRE_OK(param->scheduler->try_schedule(instr_ref, push_instruction_input, &push_param, &schedule_instr,
                                                      param->scheduler->payload));
            if (schedule_instr) {
                REQUIRE_OK(schedule_stack_push(param, instr_ref, false, push_param.insert_position));
            }
        } else {
            const kefir_size_t linear_index = param->schedule->next_instruction_index++;
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(
                param->code, instr, true, update_liveness,
                &(struct update_liveness_param) {.param = param, .linear_index = linear_index}));

            instruction_schedule->linear_index = linear_index;
            if (instruction_schedule->liveness_range.begin_index != UNSCHEDULED_INDEX) {
                instruction_schedule->liveness_range.begin_index =
                    MIN(instruction_schedule->liveness_range.begin_index, linear_index);
            } else {
                instruction_schedule->liveness_range.begin_index = linear_index;
            }
            if (instruction_schedule->liveness_range.end_index != UNSCHEDULED_INDEX) {
                instruction_schedule->liveness_range.end_index =
                    MAX(instruction_schedule->liveness_range.end_index, linear_index + 1);
            } else {
                instruction_schedule->liveness_range.end_index = linear_index + 1;
            }
            REQUIRE_OK(kefir_list_insert_after(param->mem, &param->block_schedule->instructions,
                                               kefir_list_tail(&param->block_schedule->instructions),
                                               (void *) (kefir_uptr_t) instr_ref));
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

static kefir_result_t schedule_block(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule,
                                     const struct kefir_opt_code_container *code,
                                     const struct kefir_opt_code_analysis *code_analysis, kefir_opt_block_id_t block_id,
                                     const struct kefir_opt_code_instruction_scheduler *scheduler) {
    REQUIRE(!kefir_hashtree_has(&schedule->blocks, (kefir_hashtree_key_t) block_id), KEFIR_OK);
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    struct kefir_opt_code_block_schedule *block_schedule =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_block_schedule));
    REQUIRE(block_schedule != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate block schedule"));

    block_schedule->linear_index = schedule->next_block_index++;
    kefir_result_t res = kefir_list_init(&block_schedule->instructions);
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &schedule->blocks, (kefir_hashtree_key_t) block_id,
                                              (kefir_hashtree_value_t) block_schedule));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, block_schedule);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_insert(mem, &schedule->blocks_by_index,
                                     (kefir_hashtree_key_t) block_schedule->linear_index,
                                     (kefir_hashtree_value_t) block_id));

    struct schedule_instruction_param param = {
        .mem = mem, .schedule = schedule, .code = code, .block_schedule = block_schedule, .scheduler = scheduler};

    REQUIRE_OK(kefir_list_init(&param.instr_queue));

    res = KEFIR_OK;
    REQUIRE_CHAIN(&res, block_collect_control_flow(code, code_analysis, block, &param));
    REQUIRE_CHAIN(&res, trace_block(&param));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&param.instr_queue, free_schedule_stack_entry, NULL));
    REQUIRE_CHAIN(&res, schedule_collect_control_flow(schedule, code, code_analysis, block, &param));
    REQUIRE_CHAIN(&res, schedule_instructions(&param, block_id));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &param.instr_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &param.instr_queue));

    const struct kefir_opt_code_structure_block *block_props = &code_analysis->structure.blocks[block->id];
    for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, successor_block_id, scheduler));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_run(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule,
                                           const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_code_analysis *code_analysis,
                                           const struct kefir_opt_code_instruction_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(code_analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(scheduler != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code scheduler"));

    REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, code->entry_point, scheduler));
    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&code_analysis->structure.indirect_jump_target_blocks, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, iter.entry);
        REQUIRE_OK(schedule_block(mem, schedule, code, code_analysis, block_id, scheduler));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_of_block(const struct kefir_opt_code_schedule *schedule,
                                                kefir_opt_block_id_t block_id,
                                                const struct kefir_opt_code_block_schedule **block_schedule) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(block_schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer block schedule"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&schedule->blocks, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer block schedule");
    }
    REQUIRE_OK(res);

    *block_schedule = (const struct kefir_opt_code_block_schedule *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_of(const struct kefir_opt_code_schedule *schedule,
                                          kefir_opt_instruction_ref_t instr_ref,
                                          const struct kefir_opt_code_instruction_schedule **instr_schedule) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(instr_schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction schedule"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&schedule->instructions, (kefir_hashtree_key_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction schedule");
    }
    REQUIRE_OK(res);

    *instr_schedule = (const struct kefir_opt_code_instruction_schedule *) node->value;
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_code_schedule_has(const struct kefir_opt_code_schedule *schedule,
                                         kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(schedule != NULL, false);
    return kefir_hashtree_has(&schedule->instructions, (kefir_hashtree_key_t) instr_ref);
}

kefir_bool_t kefir_opt_code_schedule_has_block(const struct kefir_opt_code_schedule *schedule,
                                               kefir_opt_block_id_t block_id) {
    REQUIRE(schedule != NULL, false);
    return kefir_hashtree_has(&schedule->blocks, (kefir_hashtree_key_t) block_id);
}

kefir_size_t kefir_opt_code_schedule_num_of_blocks(const struct kefir_opt_code_schedule *schedule) {
    REQUIRE(schedule != NULL, 0);
    return schedule->next_block_index;
}

kefir_result_t kefir_opt_code_schedule_block_by_index(const struct kefir_opt_code_schedule *schedule,
                                                      kefir_size_t index, kefir_opt_block_id_t *block_id_ptr) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&schedule->blocks_by_index, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer schedule block");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_schedule_iter(const struct kefir_opt_code_schedule *schedule,
                                                  kefir_opt_block_id_t block_id,
                                                  struct kefir_opt_code_block_schedule_iterator *iter) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid pointer to optimizer code block schedule iterator"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&schedule->blocks, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer code block schedule");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_opt_code_block_schedule *, block_schedule, node->value);
    iter->block_schedule = block_schedule;
    iter->iter = kefir_list_head(&iter->block_schedule->instructions);
    if (iter->iter != NULL) {
        iter->instr_ref = (kefir_opt_instruction_ref_t) (kefir_uptr_t) iter->iter->value;
        return KEFIR_OK;
    } else {
        iter->instr_ref = KEFIR_ID_NONE;
        return KEFIR_ITERATOR_END;
    }
}

kefir_result_t kefir_opt_code_block_schedule_next(struct kefir_opt_code_block_schedule_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block schedule iterator"));

    kefir_list_next(&iter->iter);
    if (iter->iter != NULL) {
        iter->instr_ref = (kefir_opt_instruction_ref_t) (kefir_uptr_t) iter->iter->value;
        return KEFIR_OK;
    } else {
        iter->instr_ref = KEFIR_ID_NONE;
        return KEFIR_ITERATOR_END;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_scheduler_default_schedule(
    kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_code_instruction_scheduler_dependency_callback_t dependency_callback, void *dependency_callback_payload,
    kefir_bool_t *schedule_instruction, void *payload) {
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
