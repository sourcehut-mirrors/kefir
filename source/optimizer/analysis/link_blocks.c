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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t link_block(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                 kefir_opt_block_id_t block_id) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(structure->code, block_id, &block));

    struct kefir_list *successors = &structure->blocks[block_id].successors;

    kefir_opt_instruction_ref_t tail_instr_ref;
    const struct kefir_opt_instruction *tail_instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(structure->code, block, &tail_instr_ref));
    REQUIRE(tail_instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, tail_instr_ref, &tail_instr));
    switch (tail_instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(
                kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                        (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
            REQUIRE_OK(
                kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                        (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.target_block));
            REQUIRE_OK(kefir_list_insert_after(
                mem, successors, kefir_list_tail(successors),
                (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.alternative_block));
            break;

        case KEFIR_OPT_OPCODE_IJUMP: {
            kefir_result_t res;
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&structure->indirect_jump_target_blocks, &iter); res == KEFIR_OK;
                 res = kefir_hashtreeset_next(&iter)) {
                REQUIRE_OK(kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                                   (void *) (kefir_uptr_t) iter.entry));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                structure->code, tail_instr->operation.parameters.inline_asm_ref, &inline_asm));
            if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                REQUIRE_OK(kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                                   (void *) (kefir_uptr_t) inline_asm->default_jump_target));

                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                                       (void *) (kefir_uptr_t) target_block));
                }
            }
        } break;

        case KEFIR_OPT_OPCODE_RETURN:
        case KEFIR_OPT_OPCODE_UNREACHABLE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                   "Unexpected terminating instruction of optimizer code block");
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(successors); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block, (kefir_uptr_t) iter->value);

        REQUIRE_OK(kefir_list_insert_after(mem, &structure->blocks[successor_block].predecessors,
                                           kefir_list_tail(&structure->blocks[successor_block].predecessors),
                                           (void *) (kefir_uptr_t) block_id));
    }
    return KEFIR_OK;
}

struct link_blocks_trace_payload {
    struct kefir_mem *mem;
    struct kefir_opt_code_structure *structure;
};

static kefir_result_t link_blocks_trace(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct link_blocks_trace_payload *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer block link parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->structure->code, instr_ref, &instr));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BLOCK_LABEL) {
        REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->structure->indirect_jump_target_blocks,
                                         (kefir_hashtreeset_entry_t) instr->operation.parameters.imm.block_ref));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_link_blocks(struct kefir_mem *mem, struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    struct link_blocks_trace_payload payload = {.mem = mem, .structure = structure};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = link_blocks_trace, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_container_trace(mem, structure->code, &tracer));

    kefir_size_t total_block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &total_block_count));
    for (kefir_opt_block_id_t block_id = 0; block_id < total_block_count; block_id++) {
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(structure->code, block_id, &block));
        if (!kefir_hashtreeset_empty(&block->public_labels)) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, &structure->indirect_jump_target_blocks,
                                             (kefir_hashtreeset_entry_t) block_id));
        }
    }

    for (kefir_opt_block_id_t block_id = 0; block_id < total_block_count; block_id++) {
        REQUIRE_OK(link_block(mem, structure, block_id));
    }

    return KEFIR_OK;
}
