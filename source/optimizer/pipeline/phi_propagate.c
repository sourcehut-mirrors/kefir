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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t phi_propagate_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                         kefir_opt_block_id_t block_id,
                                        kefir_bool_t *fixpoint_reached) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

    kefir_opt_phi_id_t phi_ref;
    const struct kefir_opt_phi_node *phi_node;
    kefir_result_t res;
    for (res = kefir_opt_code_block_phi_head(&func->code, block, &phi_ref); res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;) {

        REQUIRE_OK(kefir_opt_code_container_phi(&func->code, phi_ref, &phi_node));
        struct kefir_hashtree_node_iterator iter;
        struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter);
        if (node == NULL) {
            res = kefir_opt_phi_next_sibling(&func->code, phi_ref, &phi_ref);
            continue;
        }
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);

        if (instr_ref == phi_node->output_ref) {
            res = kefir_opt_phi_next_sibling(&func->code, phi_ref, &phi_ref);
            continue;
        }

        kefir_bool_t can_propagate = true;
        for (; can_propagate && node != NULL; node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref2, node->value);
            if (instr_ref2 != instr_ref && instr_ref2 != phi_node->output_ref) {
                can_propagate = false;
            }
        }

        if (can_propagate) {
            res = kefir_opt_phi_next_sibling(&func->code, phi_ref, &phi_ref);
            kefir_opt_instruction_ref_t output_ref = phi_node->output_ref;
            REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, instr_ref, output_ref));
            REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, output_ref));
            *fixpoint_reached = false;
        } else {
            res = kefir_opt_phi_next_sibling(&func->code, phi_ref, &phi_ref);
        }
    }

    return KEFIR_OK;
}

static kefir_result_t phi_propagate_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                          struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &total_blocks));

    kefir_bool_t fixpoint_reached = false;
    while (!fixpoint_reached) {
        fixpoint_reached = true;
        for (kefir_opt_block_id_t block_id = 0; block_id < total_blocks; block_id++) {
            REQUIRE_OK(phi_propagate_impl(mem, func, block_id, &fixpoint_reached));
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassPhiPropagate = {
    .name = "phi-propagate", .apply = phi_propagate_apply, .payload = NULL};
