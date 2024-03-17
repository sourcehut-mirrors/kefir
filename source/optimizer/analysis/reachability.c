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

#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define INSERT_INTO_QUEUE(_mem, _queue, _instr_ref)                                         \
    do {                                                                                    \
        if ((_instr_ref) != KEFIR_ID_NONE) {                                                \
            REQUIRE_OK(kefir_list_insert_after((_mem), (_queue), kefir_list_tail((_queue)), \
                                               (void *) (kefir_uptr_t) (_instr_ref)));      \
        }                                                                                   \
    } while (0)

static kefir_result_t mark_reachable_code_in_block(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                   kefir_opt_block_id_t block_id, struct kefir_list *queue) {
    REQUIRE(!analysis->blocks[block_id].reachable, KEFIR_OK);
    analysis->blocks[block_id].reachable = true;
    analysis->block_linearization_length++;

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(analysis->code, block_id, &block));
    struct kefir_opt_instruction *instr = NULL;
    kefir_result_t res;
    for (res = kefir_opt_code_block_instr_control_head(analysis->code, block, &instr); res == KEFIR_OK && instr != NULL;
         res = kefir_opt_instruction_next_control(analysis->code, instr, &instr)) {
        INSERT_INTO_QUEUE(mem, queue, instr->id);
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_store_mem(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                    struct kefir_list *queue,
                                                    const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.memory_access.location);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.memory_access.value);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_load_mem(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                   struct kefir_list *queue,
                                                   const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.memory_access.location);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_stack_alloc(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                      struct kefir_list *queue,
                                                      const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.stack_allocation.alignment_ref);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.stack_allocation.size_ref);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_bitfield(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                   struct kefir_list *queue,
                                                   const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.bitfield.base_ref);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.bitfield.value_ref);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_branch(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                 struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.branch.condition_ref);
    REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, instr->operation.parameters.branch.target_block, queue));
    if (instr->operation.parameters.branch.alternative_block != KEFIR_ID_NONE) {
        REQUIRE_OK(
            mark_reachable_code_in_block(mem, analysis, instr->operation.parameters.branch.alternative_block, queue));
    }
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_cmp_branch(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                     struct kefir_list *queue,
                                                     const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.branch.comparison.refs[0]);
    switch (instr->operation.parameters.branch.comparison.type) {
        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS_OR_EQUALS:
            INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.branch.comparison.refs[1]);
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS_CONST:
            // Intentionally left blank
            break;
    }
    REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, instr->operation.parameters.branch.target_block, queue));
    REQUIRE_OK(
        mark_reachable_code_in_block(mem, analysis, instr->operation.parameters.branch.alternative_block, queue));
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_typed_ref1(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                     struct kefir_list *queue,
                                                     const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.typed_refs.ref[0]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_typed_ref2(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                     struct kefir_list *queue,
                                                     const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.typed_refs.ref[0]);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.typed_refs.ref[1]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_ref1(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.refs[0]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_ref1_imm(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                   struct kefir_list *queue,
                                                   const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.ref_imm.refs[0]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_ref2(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.refs[0]);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.refs[1]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_atomic_op(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                    struct kefir_list *queue,
                                                    const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.atomic_op.ref[0]);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.atomic_op.ref[1]);
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.atomic_op.ref[2]);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_immediate(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                    struct kefir_list *queue,
                                                    const struct kefir_opt_instruction *instr) {
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BLOCK_LABEL) {
        REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, instr->operation.parameters.imm.block_ref, queue));
        REQUIRE_OK(kefir_list_insert_after(mem, &analysis->indirect_jump_target_blocks,
                                           kefir_list_tail(&analysis->indirect_jump_target_blocks),
                                           (void *) (kefir_uptr_t) instr->operation.parameters.imm.block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_index(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_variable(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                    struct kefir_list *queue,
                                                    const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_none(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_call_ref(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                   struct kefir_list *queue,
                                                   const struct kefir_opt_instruction *instr) {
    struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(
        kefir_opt_code_container_call(analysis->code, instr->operation.parameters.function_call.call_ref, &call));
    INSERT_INTO_QUEUE(mem, queue, instr->operation.parameters.function_call.indirect_ref);
    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        INSERT_INTO_QUEUE(mem, queue, call->arguments[i]);
    }
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_phi_ref(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                  struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(analysis->code, instr->operation.parameters.phi_ref, &phi));

    kefir_result_t res;
    struct kefir_opt_phi_node_link_iterator iter;
    kefir_opt_block_id_t block_id;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_phi_node_link_iter(phi, &iter, &block_id, &instr_ref); res == KEFIR_OK;
         res = kefir_opt_phi_node_link_next(&iter, &block_id, &instr_ref)) {
        if (analysis->blocks[block_id].reachable) {
            INSERT_INTO_QUEUE(mem, queue, instr_ref);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_inline_asm(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                     struct kefir_list *queue,
                                                     const struct kefir_opt_instruction *instr) {
    struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(analysis->code, instr->operation.parameters.inline_asm_ref,
                                                        &inline_asm));
    for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
        INSERT_INTO_QUEUE(mem, queue, inline_asm->parameters[i].read_ref);
        INSERT_INTO_QUEUE(mem, queue, inline_asm->parameters[i].load_store_ref);
    }
    if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
             node != NULL; node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
            REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, target_block, queue));
        }

        REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, inline_asm->default_jump_target, queue));
    }
    return KEFIR_OK;
}

static kefir_result_t find_reachable_code_loop(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, struct kefir_list *phi_queue) {
    kefir_result_t res = KEFIR_OK;
    for (struct kefir_list_entry *queue_head = kefir_list_head(queue); res == KEFIR_OK && queue_head != NULL;
         res = kefir_list_pop(mem, queue, queue_head), queue_head = kefir_list_head(queue)) {

        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) queue_head->value);
        if (analysis->instructions[instr_ref].reachable) {
            continue;
        }

        analysis->instructions[instr_ref].reachable = true;
        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref, &instr));

        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI && phi_queue != NULL) {
            INSERT_INTO_QUEUE(mem, phi_queue, instr_ref);
            continue;
        }

        analysis->linearization_length++;

        switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                                     \
    case KEFIR_OPT_OPCODE_##_id:                                               \
        REQUIRE_OK(find_reachable_code_##_class(mem, analysis, queue, instr)); \
        break;

            KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
        }
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

#undef INSERT_INTO_QUEUE

static kefir_result_t find_reachable_code_impl(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, struct kefir_list *phi_queue) {
    REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, analysis->code->entry_point, queue));
    kefir_size_t total_block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &total_block_count));
    for (kefir_opt_block_id_t block_id = 0; block_id < total_block_count; block_id++) {
        struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(analysis->code, block_id, &block));
        if (!kefir_hashtreeset_empty(&block->public_labels)) {
            REQUIRE_OK(mark_reachable_code_in_block(mem, analysis, block_id, queue));
        }
    }
    REQUIRE_OK(find_reachable_code_loop(mem, analysis, queue, phi_queue));
    for (const struct kefir_list_entry *iter = kefir_list_head(phi_queue); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        analysis->instructions[instr_ref].reachable = false;
    }
    REQUIRE_OK(kefir_list_move_all(queue, phi_queue));
    REQUIRE_OK(find_reachable_code_loop(mem, analysis, queue, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analyze_reachability(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    struct kefir_list instr_queue, phi_queue;
    REQUIRE_OK(kefir_list_init(&instr_queue));
    REQUIRE_OK(kefir_list_init(&phi_queue));
    kefir_result_t res = find_reachable_code_impl(mem, analysis, &instr_queue, &phi_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &instr_queue);
        kefir_list_free(mem, &phi_queue);
        return res;
    });
    res = kefir_list_free(mem, &instr_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &phi_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &phi_queue));
    return res;
}
