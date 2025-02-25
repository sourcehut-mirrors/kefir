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

static kefir_result_t drop_dead_phi_links_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                               struct kefir_opt_code_structure *structure) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

        kefir_opt_phi_id_t phi_ref;
        kefir_result_t res;
        for (res = kefir_opt_code_block_phi_head(&func->code, block, &phi_ref);
             res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
             res = kefir_opt_phi_next_sibling(&func->code, phi_ref, &phi_ref)) {
            const struct kefir_opt_phi_node *phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(&func->code, phi_ref, &phi_node));
            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter); node != NULL;) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, link_block_id, node->key);
                node = kefir_hashtree_next(&iter);
                kefir_bool_t found_predecessor = false;
                for (const struct kefir_list_entry *iter2 = kefir_list_head(&structure->blocks[block_id].predecessors);
                     iter2 != NULL && !found_predecessor; kefir_list_next(&iter2)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, pred_block_id, (kefir_uptr_t) iter2->value);
                    if (link_block_id == pred_block_id) {
                        found_predecessor = true;
                    }
                }

                if (!found_predecessor) {
                    REQUIRE_OK(kefir_opt_code_container_phi_drop_link(mem, &func->code, phi_ref, link_block_id));
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_dead_phi_links_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                struct kefir_opt_function *func,
                                                const struct kefir_optimizer_pass *pass,
                                                const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_structure structure;
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    REQUIRE_CHAIN(&res, drop_dead_phi_links_impl(mem, func, &structure));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassDropDeadPhiLinks = {
    .name = "drop-dead-phi-links", .apply = drop_dead_phi_links_apply, .payload = NULL};
