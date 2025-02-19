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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t branch_removal_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &instr_id));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
        if (instr->operation.opcode != KEFIR_OPT_OPCODE_BRANCH) {
            continue;
        }

        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&func->debug_info, instr_id));

        const kefir_opt_block_id_t block_id = instr->block_id;
        kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;

        const struct kefir_opt_instruction *arg1;
        REQUIRE_OK(
            kefir_opt_code_container_instr(&func->code, instr->operation.parameters.branch.condition_ref, &arg1));
        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
            if (arg1->operation.parameters.imm.integer != 0) {
                REQUIRE_OK(kefir_opt_code_builder_finalize_jump(
                    mem, &func->code, block_id, instr->operation.parameters.branch.target_block, &replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id,
                                                                instr->operation.parameters.branch.alternative_block,
                                                                &replacement_ref));
            }

            REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr_id));
            REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_id));
        }

        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
            &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassBranchRemoval = {
    .name = "branch-removal", .apply = branch_removal_apply, .payload = NULL};
