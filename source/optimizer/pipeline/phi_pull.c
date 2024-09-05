/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/hashtreeset.h"
#include <string.h>

struct phi_pull_state {
    struct kefir_mem *mem;
    const struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    struct kefir_list block_queue;
    struct kefir_hashtreeset visited_blocks;
};

static kefir_result_t update_queue(struct phi_pull_state *state, const struct kefir_opt_code_block *block) {
    kefir_opt_instruction_ref_t tail_instr_ref;
    const struct kefir_opt_instruction *tail_instr;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->func->code, block, &tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, tail_instr_ref, &tail_instr));

    kefir_opt_block_id_t target_block_id, alternative_block_id;
    switch (tail_instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            target_block_id = tail_instr->operation.parameters.branch.target_block;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                               (void *) (kefir_uptr_t) target_block_id));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
            target_block_id = tail_instr->operation.parameters.branch.target_block;
            alternative_block_id = tail_instr->operation.parameters.branch.alternative_block;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                               (void *) (kefir_uptr_t) target_block_id));
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                               (void *) (kefir_uptr_t) alternative_block_id));
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *inline_asm;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                &state->func->code, tail_instr->operation.parameters.inline_asm_ref, &inline_asm));

            if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue,
                                                   kefir_list_tail(&state->block_queue),
                                                   (void *) (kefir_uptr_t) inline_asm->default_jump_target));

                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {

                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue,
                                                       kefir_list_tail(&state->block_queue),
                                                       (void *) (kefir_uptr_t) target_block));
                }
            }
        } break;

        case KEFIR_OPT_OPCODE_RETURN:
        case KEFIR_OPT_OPCODE_IJUMP:
            // Intentionally left blank
            break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t phi_pull_impl(struct phi_pull_state *state) {
    REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                       (void *) (kefir_uptr_t) state->func->code.entry_point));
    for (const struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         kefir_list_pop(state->mem, &state->block_queue, (struct kefir_list_entry *) iter),
                                       iter = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);

        if (kefir_hashtreeset_has(&state->visited_blocks, (kefir_hashtreeset_entry_t) block_id)) {
            continue;
        }
        REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->visited_blocks, (kefir_hashtreeset_entry_t) block_id));

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&state->func->code, block_id, &block));
        REQUIRE_OK(update_queue(state, block));

        kefir_opt_phi_id_t phi_ref;
        const struct kefir_opt_phi_node *phi_node;
        kefir_result_t res;
        for (res = kefir_opt_code_block_phi_head(&state->func->code, block, &phi_ref);
             res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;) {

            REQUIRE_OK(kefir_opt_code_container_phi(&state->func->code, phi_ref, &phi_node));
            struct kefir_hashtree_node_iterator iter;
            const struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter);
            if (node == NULL) {
                continue;
            }

            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

            REQUIRE_OK(
                kefir_opt_code_debug_info_set_instruction_location_cursor_of(&state->func->debug_info, instr_ref));

            kefir_bool_t pull = true;
            switch (instr->operation.opcode) {
#define COMPARE_INSTR(_cond)                                                                               \
    do {                                                                                                   \
        for (node = kefir_hashtree_next(&iter); node != NULL; node = kefir_hashtree_next(&iter)) {         \
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, other_instr_ref, node->value);                   \
            const struct kefir_opt_instruction *other_instr;                                               \
            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, other_instr_ref, &other_instr)); \
            if (_cond) {                                                                                   \
                pull = false;                                                                              \
                break;                                                                                     \
            }                                                                                              \
        }                                                                                                  \
    } while (false)

                case KEFIR_OPT_OPCODE_INT_CONST:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST ||
                                  other_instr->operation.parameters.imm.integer !=
                                      instr->operation.parameters.imm.integer);
                    break;

                case KEFIR_OPT_OPCODE_UINT_CONST:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST ||
                                  other_instr->operation.parameters.imm.uinteger !=
                                      instr->operation.parameters.imm.uinteger);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_CONST:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_FLOAT32_CONST ||
                                  memcmp(&instr->operation.parameters.imm.float32,
                                         &other_instr->operation.parameters.imm.float32, sizeof(kefir_float32_t)) != 0);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_CONST:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_FLOAT64_CONST ||
                                  memcmp(&instr->operation.parameters.imm.float64,
                                         &other_instr->operation.parameters.imm.float64, sizeof(kefir_float64_t)) != 0);
                    break;

                case KEFIR_OPT_OPCODE_GET_LOCAL:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_GET_LOCAL ||
                                  instr->operation.parameters.variable.local_index !=
                                      other_instr->operation.parameters.variable.local_index ||
                                  instr->operation.parameters.variable.offset !=
                                      other_instr->operation.parameters.variable.offset);
                    break;

                case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
                                  instr->operation.parameters.ir_ref != other_instr->operation.parameters.ir_ref);
                    break;

                case KEFIR_OPT_OPCODE_GET_GLOBAL:
                    COMPARE_INSTR(other_instr->operation.opcode != KEFIR_OPT_OPCODE_GET_GLOBAL ||
                                  instr->operation.parameters.ir_ref != other_instr->operation.parameters.ir_ref);
                    break;

                default:
                    pull = false;
                    break;
            }

            if (pull) {
                kefir_opt_instruction_ref_t replacement_ref;
                const struct kefir_opt_operation op = instr->operation;
                REQUIRE_OK(kefir_opt_code_container_new_instruction(state->mem, &state->func->code, block_id, &op,
                                                                    &replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&state->func->code, phi_node->output_ref,
                                                                           replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_replace_references(state->mem, &state->func->code, replacement_ref,
                                                                       phi_node->output_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(&state->func->code, phi_node->output_ref));

                kefir_opt_phi_id_t prev_phi_ref = phi_ref;
                REQUIRE_OK(kefir_opt_phi_next_sibling(&state->func->code, phi_ref, &phi_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_phi(&state->func->code, prev_phi_ref));

                REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                    &state->func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
            } else {
                REQUIRE_OK(kefir_opt_phi_next_sibling(&state->func->code, phi_ref, &phi_ref));
            }
        }
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t phi_pull_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct phi_pull_state state = {.mem = mem, .module = module, .func = func};
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_hashtreeset_init(&state.visited_blocks, &kefir_hashtree_uint_ops));

    kefir_result_t res = phi_pull_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.visited_blocks);
        kefir_list_free(mem, &state.block_queue);
        return res;
    });

    res = kefir_hashtreeset_free(mem, &state.visited_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &state.block_queue));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassPhiPull = {
    .name = "phi-pull", .apply = phi_pull_apply, .payload = NULL};
