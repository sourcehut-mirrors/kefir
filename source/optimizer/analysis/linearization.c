/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#define MARK_LIVENESS(_analysis, _target_ref, _instr_ref)                                                 \
    do {                                                                                                  \
        if ((_target_ref) != KEFIR_ID_NONE) {                                                             \
            kefir_size_t _end_index = (_analysis)->instructions[_target_ref].liveness_interval.end_index; \
            (_analysis)->instructions[_target_ref].liveness_interval.end_index =                          \
                MAX(_end_index, (_analysis)->instructions[(_instr_ref)].linear_position + 1);             \
        }                                                                                                 \
    } while (0)

static kefir_result_t mark_liveness_store_mem(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                              struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.memory_access.location, instr->id);
    MARK_LIVENESS(analysis, instr->operation.parameters.memory_access.value, instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_load_mem(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                             struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.memory_access.location, instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_stack_alloc(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.stack_allocation.alignment_ref, instr->id);
    MARK_LIVENESS(analysis, instr->operation.parameters.stack_allocation.size_ref, instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_bitfield(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                             struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.bitfield.base_ref, instr->id);
    MARK_LIVENESS(analysis, instr->operation.parameters.bitfield.value_ref, instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_branch(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                           struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    if (instr->operation.parameters.branch.alternative_block != KEFIR_ID_NONE) {
        REQUIRE_OK(
            kefir_list_insert_after(mem, queue, kefir_list_tail(queue),
                                    (void *) (kefir_uptr_t) instr->operation.parameters.branch.alternative_block));
    }
    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue),
                                       (void *) (kefir_uptr_t) instr->operation.parameters.branch.target_block));
    MARK_LIVENESS(analysis, instr->operation.parameters.branch.condition_ref, instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_typed_ref1(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.typed_refs.ref[0], instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_typed_ref2(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                               struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.typed_refs.ref[0], instr->id);
    MARK_LIVENESS(analysis, instr->operation.parameters.typed_refs.ref[1], instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_ref1(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                         struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.refs[0], instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_ref2(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                         struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    MARK_LIVENESS(analysis, instr->operation.parameters.refs[0], instr->id);
    MARK_LIVENESS(analysis, instr->operation.parameters.refs[1], instr->id);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_immediate(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                              struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(analysis);
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BLOCK_LABEL) {
        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue),
                                           (void *) (kefir_uptr_t) instr->operation.parameters.imm.block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_index(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                          struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_none(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                         struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_ir_ref(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                           struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_call_ref(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                             struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(queue);
    struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(
        kefir_opt_code_container_call(analysis->code, instr->operation.parameters.function_call.call_ref, &call));
    MARK_LIVENESS(analysis, instr->operation.parameters.function_call.indirect_ref, instr->id);
    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        MARK_LIVENESS(analysis, call->arguments[i], instr->id);
    }
    return KEFIR_OK;
}

static kefir_result_t mark_liveness_phi_ref(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                            struct kefir_list *queue, const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(analysis);
    UNUSED(queue);
    UNUSED(instr);
    return KEFIR_OK;
}

static kefir_result_t linearize_impl(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                     struct kefir_list *queue) {
    kefir_size_t linear_index = 0;
    kefir_result_t res = KEFIR_OK;
    for (struct kefir_list_entry *queue_tail = kefir_list_tail(queue); res == KEFIR_OK && queue_tail != NULL;
         res = kefir_list_pop(mem, queue, queue_tail), queue_tail = kefir_list_tail(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) queue_tail->value);

        if (analysis->blocks[block_id].linear_interval.begin_index != KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED) {
            continue;
        }

        analysis->blocks[block_id].linear_interval.begin_index = linear_index;

        struct kefir_opt_code_block *block = NULL;
        REQUIRE_OK(kefir_opt_code_container_block(analysis->code, block_id, &block));
        const struct kefir_opt_instruction *instr = NULL;
        for (res = kefir_opt_code_block_instr_head(analysis->code, block, &instr); res == KEFIR_OK && instr != NULL;
             res = kefir_opt_instruction_next_sibling(analysis->code, instr, &instr)) {

            if (!analysis->instructions[instr->id].reachable) {
                continue;
            }

            analysis->linearization[linear_index] = &analysis->instructions[instr->id];
            analysis->instructions[instr->id].linear_position = linear_index;
            analysis->instructions[instr->id].liveness_interval.begin_index = linear_index;
            analysis->instructions[instr->id].liveness_interval.end_index = ++linear_index;

            switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                               \
    case KEFIR_OPT_OPCODE_##_id:                                         \
        REQUIRE_OK(mark_liveness_##_class(mem, analysis, queue, instr)); \
        break;

                KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
            }
        }
        REQUIRE_OK(res);

        analysis->blocks[block_id].linear_interval.end_index = linear_index;
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analyze_linearize(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    analysis->linearization = KEFIR_MALLOC(
        mem, sizeof(struct kefir_opt_code_analysis_instruction_properties *) * analysis->linearization_length);
    REQUIRE(analysis->linearization != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis linearization"));

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = kefir_list_insert_after(mem, &queue, kefir_list_tail(&queue),
                                                 (void *) (kefir_uptr_t) analysis->code->entry_point);
    REQUIRE_CHAIN(&res, linearize_impl(mem, analysis, &queue));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}
