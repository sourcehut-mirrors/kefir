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

        if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            const struct kefir_opt_phi_node *use_phi;
            REQUIRE_OK(kefir_opt_code_container_phi(param->structure->code, use_instr->operation.parameters.phi_ref,
                                                    &use_phi));

            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&use_phi->links, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
                if (src_instr_ref == instr_ref && IS_BLOCK_REACHABLE(param->structure, src_block_id)) {
                    REQUIRE_OK(kefir_opt_find_closest_common_dominator(param->structure, src_block_id,
                                                                       closest_dominator, &closest_dominator));
                }
            }
        } else if (IS_BLOCK_REACHABLE(param->structure, use_instr->block_id)) {
            REQUIRE_OK(kefir_opt_find_closest_common_dominator(param->structure, use_instr->block_id, closest_dominator,
                                                               &closest_dominator));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(closest_dominator != KEFIR_ID_NONE && closest_dominator != instr->block_id, KEFIR_OK);

    const struct kefir_opt_operation operation = instr->operation;
    kefir_opt_instruction_ref_t new_instr;
    REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&param->func->debug_info, instr_ref));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(param->mem, &param->func->code, closest_dominator, &operation,
                                                        &new_instr));
    REQUIRE_OK(kefir_opt_code_container_replace_references(param->mem, &param->func->code, new_instr, instr_ref));
    REQUIRE_OK(
        kefir_opt_code_debug_info_replace_local_variable(param->mem, &param->func->debug_info, instr_ref, new_instr));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(param->mem, &param->func->code, instr_ref));
    REQUIRE_OK(
        kefir_opt_code_debug_info_replace_local_variable(param->mem, &param->func->debug_info, instr_ref, new_instr));
    return KEFIR_OK;
}

static kefir_result_t local_alloc_sink_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                             struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                             const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
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
