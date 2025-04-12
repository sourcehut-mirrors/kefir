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

#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct pending_instructions {
    struct kefir_queue instr;
};

static kefir_result_t free_pending_instructions(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct pending_instructions *, pending, value);
    REQUIRE(pending != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pending instructions"));

    REQUIRE_OK(kefir_queue_free(mem, &pending->instr));
    KEFIR_FREE(mem, pending);
    return KEFIR_OK;
}

static kefir_result_t enqueue_instr(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                    struct kefir_queue *instr_queue, struct kefir_hashtreeset *traced_blocks,
                                    struct kefir_hashtree *pending_instr, kefir_opt_instruction_ref_t instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    if (kefir_hashtreeset_has(traced_blocks, (kefir_hashtreeset_entry_t) instr->block_id)) {
        REQUIRE_OK(kefir_queue_push(mem, instr_queue, (kefir_queue_entry_t) instr_ref));
    } else {
        struct pending_instructions *pending;
        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(pending_instr, (kefir_hashtree_key_t) instr->block_id, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            pending = (struct pending_instructions *) node->value;
        } else {
            pending = KEFIR_MALLOC(mem, sizeof(struct pending_instructions));
            REQUIRE(pending != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pending instructions"));
            res = kefir_queue_init(&pending->instr);
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, pending_instr, (kefir_hashtree_key_t) instr->block_id,
                                                      (kefir_hashtree_value_t) pending));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, pending);
                return res;
            });
        }

        REQUIRE_OK(kefir_queue_push(mem, &pending->instr, (kefir_queue_entry_t) instr->id));
    }
    return KEFIR_OK;
}

static kefir_result_t trace_block(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                  kefir_opt_block_id_t block_id, struct kefir_queue *instr_queue,
                                  struct kefir_hashtreeset *traced_blocks, struct kefir_hashtree *pending_instr) {
    REQUIRE(!kefir_hashtreeset_has(traced_blocks, (kefir_hashtreeset_entry_t) block_id), KEFIR_OK);
    REQUIRE_OK(kefir_hashtreeset_add(mem, traced_blocks, (kefir_hashtreeset_entry_t) block_id));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_control_head(code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(code, instr_ref, &instr_ref)) {
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (!KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(instr)) {
            REQUIRE_OK(enqueue_instr(mem, code, instr_queue, traced_blocks, pending_instr, instr_ref));
        }
    }
    REQUIRE_OK(res);

    struct kefir_hashtree_node *node;
    res = kefir_hashtree_at(pending_instr, (kefir_hashtree_key_t) block_id, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct pending_instructions *, pending, node->value);
        while (!kefir_queue_is_empty(&pending->instr)) {
            kefir_queue_entry_t entry;
            REQUIRE_OK(kefir_queue_pop_first(mem, &pending->instr, &entry));
            REQUIRE_OK(kefir_queue_push(mem, instr_queue, entry));
        }
        REQUIRE_OK(kefir_hashtree_delete(mem, pending_instr, (kefir_hashtree_key_t) block_id));
    }
    return KEFIR_OK;
}

struct add_instr_input_param {
    struct kefir_mem *mem;
    const struct kefir_opt_code_container *code;
    struct kefir_queue *instr_queue;
    struct kefir_hashtreeset *traced_blocks;
    struct kefir_hashtree *pending_instr;
};

static kefir_result_t add_instr_input(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct add_instr_input_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code instruction trace parameter"));

    REQUIRE_OK(enqueue_instr(param->mem, param->code, param->instr_queue, param->traced_blocks, param->pending_instr,
                             instr_ref));
    return KEFIR_OK;
}

static kefir_result_t trace_instr(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                  const struct kefir_opt_code_container_tracer *tracer,
                                  kefir_opt_instruction_ref_t instr_ref, struct kefir_queue *instr_queue,
                                  struct kefir_hashtreeset *traced_blocks, struct kefir_hashtreeset *traced_instr,
                                  struct kefir_hashtreeset *pending_block_labels, kefir_bool_t *has_indirect_jumps,
                                  struct kefir_hashtree *pending_instr) {
    REQUIRE(!kefir_hashtreeset_has(traced_instr, (kefir_hashtreeset_entry_t) instr_ref), KEFIR_OK);
    REQUIRE_OK(kefir_hashtreeset_add(mem, traced_instr, (kefir_hashtreeset_entry_t) instr_ref));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            if (*has_indirect_jumps) {
                REQUIRE_OK(trace_block(mem, code, instr->operation.parameters.imm.block_ref, instr_queue, traced_blocks,
                                       pending_instr));
            } else {
                REQUIRE_OK(kefir_hashtreeset_add(
                    mem, pending_block_labels, (kefir_hashtreeset_entry_t) instr->operation.parameters.imm.block_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
            REQUIRE_OK(trace_block(mem, code, instr->operation.parameters.branch.alternative_block, instr_queue,
                                   traced_blocks, pending_instr));
            REQUIRE_OK(trace_block(mem, code, instr->operation.parameters.branch.target_block, instr_queue,
                                   traced_blocks, pending_instr));
            break;

        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(trace_block(mem, code, instr->operation.parameters.branch.target_block, instr_queue,
                                   traced_blocks, pending_instr));
            break;

        case KEFIR_OPT_OPCODE_IJUMP: {
            *has_indirect_jumps = true;

            kefir_size_t total_block_count;
            REQUIRE_OK(kefir_opt_code_container_block_count(code, &total_block_count));
            for (kefir_opt_block_id_t block_id = 0; block_id < total_block_count; block_id++) {
                const struct kefir_opt_code_block *block;
                REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));
                if (!kefir_hashtreeset_empty(&block->public_labels) ||
                    kefir_hashtreeset_has(pending_block_labels, (kefir_hashtreeset_entry_t) block_id)) {
                    REQUIRE_OK(trace_block(mem, code, block_id, instr_queue, traced_blocks, pending_instr));
                }
            }
            REQUIRE_OK(kefir_hashtreeset_clean(mem, pending_block_labels));
        } break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, instr->operation.parameters.inline_asm_ref,
                                                                &inline_asm));
            if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(trace_block(mem, code, target_block, instr_queue, traced_blocks, pending_instr));
                }

                REQUIRE_OK(
                    trace_block(mem, code, inline_asm->default_jump_target, instr_queue, traced_blocks, pending_instr));
            }
        } break;

        default:
            // Intentionally left blank
            break;
    }

    REQUIRE_OK(kefir_opt_instruction_extract_inputs(code, instr, false, add_instr_input,
                                                    &(struct add_instr_input_param) {.mem = mem,
                                                                                     .code = code,
                                                                                     .instr_queue = instr_queue,
                                                                                     .traced_blocks = traced_blocks,
                                                                                     .pending_instr = pending_instr}));

    REQUIRE_OK(tracer->trace_instruction(instr_ref, tracer->payload));
    return KEFIR_OK;
}

static kefir_result_t trace_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                 const struct kefir_opt_code_container_tracer *tracer, struct kefir_queue *instr_queue,
                                 struct kefir_hashtreeset *traced_blocks, struct kefir_hashtreeset *traced_instr,
                                 struct kefir_hashtree *pending_instr, struct kefir_hashtreeset *pending_block_labels) {
    kefir_size_t total_block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &total_block_count));
    REQUIRE_OK(trace_block(mem, code, code->entry_point, instr_queue, traced_blocks, pending_instr));

    kefir_bool_t has_indirect_jumps = false;
    while (!kefir_queue_is_empty(instr_queue)) {
        kefir_queue_entry_t entry;
        REQUIRE_OK(kefir_queue_pop_first(mem, instr_queue, &entry));
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
        REQUIRE_OK(trace_instr(mem, code, tracer, instr_ref, instr_queue, traced_blocks, traced_instr,
                               pending_block_labels, &has_indirect_jumps, pending_instr));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_trace(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_container_tracer *tracer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(tracer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code tracer"));

    struct kefir_queue instr_queue;
    struct kefir_hashtreeset traced_blocks;
    struct kefir_hashtreeset traced_instr;
    struct kefir_hashtree pending_instr;
    struct kefir_hashtreeset pending_block_labels;
    REQUIRE_OK(kefir_queue_init(&instr_queue));
    REQUIRE_OK(kefir_hashtreeset_init(&traced_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&traced_instr, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&pending_block_labels, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&pending_instr, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&pending_instr, free_pending_instructions, NULL));

    kefir_result_t res = trace_impl(mem, code, tracer, &instr_queue, &traced_blocks, &traced_instr, &pending_instr,
                                    &pending_block_labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &pending_block_labels);
        kefir_queue_free(mem, &instr_queue);
        kefir_hashtreeset_free(mem, &traced_blocks);
        kefir_hashtreeset_free(mem, &traced_instr);
        kefir_hashtree_free(mem, &pending_instr);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &pending_block_labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &instr_queue);
        kefir_hashtreeset_free(mem, &traced_blocks);
        kefir_hashtreeset_free(mem, &traced_instr);
        kefir_hashtree_free(mem, &pending_instr);
        return res;
    });
    res = kefir_queue_free(mem, &instr_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &traced_blocks);
        kefir_hashtreeset_free(mem, &traced_instr);
        kefir_hashtree_free(mem, &pending_instr);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &traced_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &traced_instr);
        kefir_hashtree_free(mem, &pending_instr);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &traced_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &pending_instr);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &pending_instr));
    return KEFIR_OK;
}
