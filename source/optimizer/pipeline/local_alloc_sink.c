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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/structure.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct trace_instruction_payload {
    struct kefir_mem *mem;
    struct kefir_opt_function *func;
    struct kefir_opt_code_structure *structure;
};

#define IS_BLOCK_REACHABLE(_structure, _block_id)      \
    ((_block_id) == (_structure)->code->entry_point || \
     (_structure)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t find_common_dominator(struct kefir_opt_code_structure *structure, kefir_opt_block_id_t block_id,
                                            kefir_opt_block_id_t *common_dominator_block_id) {
    REQUIRE(IS_BLOCK_REACHABLE(structure, block_id), KEFIR_OK);
    if (*common_dominator_block_id == KEFIR_ID_NONE) {
        *common_dominator_block_id = block_id;
        return KEFIR_OK;
    }

    kefir_bool_t is_dominator;
    kefir_opt_block_id_t dominator_block_id = *common_dominator_block_id;
    do {
        if (dominator_block_id == KEFIR_ID_NONE) {
            break;
        }
        REQUIRE_OK(kefir_opt_code_structure_is_dominator(structure, block_id, dominator_block_id, &is_dominator));
        if (is_dominator) {
            *common_dominator_block_id = dominator_block_id;
            return KEFIR_OK;
        } else {
            dominator_block_id = structure->blocks[dominator_block_id].immediate_dominator;
        }
    } while (!is_dominator);

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find common dominator block");
}

static kefir_result_t trace_instruction_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct trace_instruction_payload *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid local allocation sink parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL, KEFIR_OK);

    kefir_opt_block_id_t closest_dominator = KEFIR_ID_NONE;

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(param->structure->code, use_iter.use_instr_ref, &use_instr));

        REQUIRE_OK(find_common_dominator(param->structure, use_instr->block_id, &closest_dominator));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_phi_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_phi_node *use_phi;
        REQUIRE_OK(kefir_opt_code_container_phi(param->structure->code, use_iter.use_phi_ref, &use_phi));

        struct kefir_hashtree_node_iterator iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&use_phi->links, &iter); node != NULL;
             node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
            if (src_instr_ref == instr_ref) {
                REQUIRE_OK(find_common_dominator(param->structure, src_block_id, &closest_dominator));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_call_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_call_node *use_call;
        REQUIRE_OK(kefir_opt_code_container_call(param->structure->code, use_iter.use_call_ref, &use_call));

        REQUIRE_OK(find_common_dominator(param->structure, use_call->block_id, &closest_dominator));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_inline_asm_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_inline_assembly_node *use_inline_asm;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly(param->structure->code, use_iter.use_inline_asm_ref,
                                                       &use_inline_asm));

        REQUIRE_OK(find_common_dominator(param->structure, use_inline_asm->block_id, &closest_dominator));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(closest_dominator != KEFIR_ID_NONE && closest_dominator != instr->block_id, KEFIR_OK);

    const struct kefir_opt_operation operation = instr->operation;
    kefir_opt_instruction_ref_t new_instr;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(param->mem, &param->func->code, closest_dominator, &operation,
                                                        &new_instr));
    REQUIRE_OK(kefir_opt_code_container_replace_references(param->mem, &param->func->code, new_instr, instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(param->mem, &param->func->code, instr_ref));
    return KEFIR_OK;
}

static kefir_result_t local_alloc_sink_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_structure structure;
    struct trace_instruction_payload payload = {.mem = mem, .func = func, .structure = &structure};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = trace_instruction_impl, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    REQUIRE_CHAIN(&res, kefir_opt_code_container_trace(mem, &func->code, &tracer));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassLocalAllocSink = {
    .name = "local-alloc-sink", .apply = local_alloc_sink_apply, .payload = NULL};
