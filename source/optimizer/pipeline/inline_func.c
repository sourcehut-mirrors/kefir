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
#include "kefir/optimizer/configuration.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/inline.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t inline_func_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, struct kefir_opt_code_structure *structure,
                                       const struct kefir_optimizer_configuration *config,
                                       kefir_bool_t *fixpoint_reached) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &num_of_blocks));

    for (kefir_opt_block_id_t block_id = 0; block_id < structure->num_of_blocks; block_id++) {
        kefir_bool_t reachable;
        REQUIRE_OK(kefir_opt_code_structure_is_reachable_from_entry(structure, block_id, &reachable));
        if (!reachable) {
            continue;
        }

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(&func->code, block, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
            kefir_bool_t inlined = false;
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE) {
                REQUIRE_OK(kefir_opt_try_inline_function_call(
                    mem, module, func, structure,
                    &(struct kefir_opt_try_inline_function_call_parameters) {
                        .max_inline_depth = config->max_inline_depth,
                        .max_inlines_per_function = config->max_inlines_per_function},
                    instr_ref, &inlined));
            }
            if (inlined) {
                REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));
                REQUIRE_OK(kefir_opt_code_block_instr_head(&func->code, block, &instr_ref));
                *fixpoint_reached = false;
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t inline_func_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                        const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer configuration"));

    struct kefir_opt_code_structure structure;
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    kefir_bool_t fixpoint_reached = false;
    while (!fixpoint_reached && res == KEFIR_OK) {
        fixpoint_reached = true;
        REQUIRE_CHAIN(&res, inline_func_impl(mem, module, func, &structure, config, &fixpoint_reached));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassInlineFunc = {
    .name = "inline-func", .apply = inline_func_apply, .payload = NULL};
