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

static kefir_result_t ignore_use_callback(kefir_opt_instruction_ref_t instr_ref,
                                          kefir_opt_instruction_ref_t user_instr_ref, kefir_bool_t *ignore_use,
                                          void *payload) {
    UNUSED(instr_ref);
    REQUIRE(ignore_use != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t *, ignore_instr_ref, payload);
    REQUIRE(ignore_use != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    *ignore_use = *ignore_instr_ref == user_instr_ref;
    return KEFIR_OK;
}

static kefir_result_t phi_select_match(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_code_structure *structure, kefir_opt_phi_id_t phi_ref) {
    const struct kefir_opt_phi_node *phi_node;
    REQUIRE_OK(kefir_opt_code_container_phi(&func->code, phi_ref, &phi_node));

    REQUIRE(phi_node->number_of_links == 2, KEFIR_OK);

    const kefir_opt_instruction_ref_t phi_instr_ref = phi_node->output_ref;
    const struct kefir_opt_instruction *phi_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, phi_instr_ref, &phi_instr));
    const kefir_opt_block_id_t phi_instr_block_id = phi_instr->block_id;

    const kefir_opt_block_id_t immediate_dominator_block_id =
        structure->blocks[phi_instr->block_id].immediate_dominator;
    REQUIRE(immediate_dominator_block_id != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_code_block *immediate_dominator_block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, immediate_dominator_block_id, &immediate_dominator_block));

    kefir_opt_instruction_ref_t immediate_dominator_tail_ref;
    REQUIRE_OK(
        kefir_opt_code_block_instr_control_tail(&func->code, immediate_dominator_block, &immediate_dominator_tail_ref));
    REQUIRE(immediate_dominator_tail_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *immediate_dominator_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, immediate_dominator_tail_ref, &immediate_dominator_tail));
    REQUIRE(immediate_dominator_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH, KEFIR_OK);

    kefir_opt_branch_condition_variant_t condition_variant =
        immediate_dominator_tail->operation.parameters.branch.condition_variant;
    kefir_opt_instruction_ref_t condition_ref = immediate_dominator_tail->operation.parameters.branch.condition_ref;

    const kefir_opt_block_id_t immediate_dominator_target =
        immediate_dominator_tail->operation.parameters.branch.target_block;
    const kefir_opt_block_id_t immediate_dominator_alternative =
        immediate_dominator_tail->operation.parameters.branch.alternative_block;

    kefir_opt_instruction_ref_t link_ref1, link_ref2;
    kefir_bool_t move_link1 = false;
    kefir_bool_t move_link2 = false;
    if (immediate_dominator_target == phi_instr->block_id) {
        REQUIRE_OK(
            kefir_opt_code_container_phi_link_for(&func->code, phi_ref, immediate_dominator_block_id, &link_ref1));
    } else {
#define CHECK_TARGET(_dominator_branch, _link_ref, _move_link)                                                     \
    do {                                                                                                           \
        kefir_bool_t is_predecessor;                                                                               \
        REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(                                    \
            structure, immediate_dominator_block_id, (_dominator_branch), &is_predecessor));                       \
        REQUIRE(is_predecessor, KEFIR_OK);                                                                         \
        REQUIRE_OK(kefir_opt_code_structure_block_direct_predecessor(structure, (_dominator_branch),               \
                                                                     phi_instr->block_id, &is_predecessor));       \
        REQUIRE(is_predecessor, KEFIR_OK);                                                                         \
                                                                                                                   \
        const struct kefir_opt_code_block *branch_block;                                                           \
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, (_dominator_branch), &branch_block));               \
        kefir_opt_instruction_ref_t branch_block_tail, branch_block_tail_prev;                                     \
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, branch_block, &branch_block_tail));        \
        REQUIRE(branch_block_tail != KEFIR_ID_NONE, KEFIR_OK);                                                     \
        REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, branch_block_tail, &branch_block_tail_prev));   \
        REQUIRE(branch_block_tail_prev == KEFIR_ID_NONE, KEFIR_OK);                                                \
                                                                                                                   \
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&func->code, phi_ref, (_dominator_branch), (_link_ref))); \
                                                                                                                   \
        const struct kefir_opt_instruction *link_instr;                                                            \
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, *(_link_ref), &link_instr));                        \
                                                                                                                   \
        if (link_instr->block_id == (_dominator_branch)) {                                                         \
            kefir_bool_t can_move_instr;                                                                           \
            REQUIRE_OK(kefir_opt_can_move_instruction_with_local_dependencies(                                     \
                mem, structure, *(_link_ref), phi_instr->block_id,                                                 \
                &(struct kefir_opt_can_move_instruction_ignore_use) {.callback = ignore_use_callback,              \
                                                                     .payload = (void *) &phi_instr_ref},          \
                &can_move_instr));                                                                                 \
            REQUIRE(can_move_instr, KEFIR_OK);                                                                     \
            *(_move_link) = true;                                                                                  \
        }                                                                                                          \
    } while (0)
        CHECK_TARGET(immediate_dominator_target, &link_ref1, &move_link1);
    }
    if (immediate_dominator_alternative == phi_instr->block_id) {
        REQUIRE_OK(
            kefir_opt_code_container_phi_link_for(&func->code, phi_ref, immediate_dominator_block_id, &link_ref2));
    } else {
        CHECK_TARGET(immediate_dominator_alternative, &link_ref2, &move_link2);
#undef CHECK_TARGET
    }

    if (move_link1) {
        REQUIRE_OK(kefir_opt_move_instruction_with_local_dependencies(mem, &func->code, link_ref1, phi_instr_block_id,
                                                                      &link_ref1));
    }
    if (move_link2) {
        REQUIRE_OK(kefir_opt_move_instruction_with_local_dependencies(mem, &func->code, link_ref2, phi_instr_block_id,
                                                                      &link_ref2));
    }

    kefir_opt_instruction_ref_t replacement_ref;
    REQUIRE_OK(kefir_opt_code_builder_select(mem, &func->code, phi_instr_block_id, condition_variant, condition_ref,
                                             link_ref1, link_ref2, &replacement_ref));
    REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, phi_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, phi_instr_ref));

    return KEFIR_OK;
}

static kefir_result_t phi_select_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                      struct kefir_opt_code_structure *structure) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

        kefir_opt_phi_id_t phi_ref;
        kefir_result_t res;
        for (res = kefir_opt_code_block_phi_head(&func->code, block, &phi_ref);
             res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;) {
            kefir_opt_phi_id_t next_phi_ref;
            REQUIRE_OK(kefir_opt_phi_next_sibling(&func->code, phi_ref, &next_phi_ref));
            REQUIRE_OK(phi_select_match(mem, func, structure, phi_ref));
            phi_ref = next_phi_ref;
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t phi_select_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_structure structure;
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    REQUIRE_CHAIN(&res, phi_select_impl(mem, func, &structure));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassPhiSelect = {
    .name = "phi-select", .apply = phi_select_apply, .payload = NULL};
