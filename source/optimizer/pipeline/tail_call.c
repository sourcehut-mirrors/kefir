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

static kefir_result_t block_tail_call_apply(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            kefir_opt_block_id_t block_id) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

    kefir_opt_instruction_ref_t tail_instr_ref, tail_prev_instr_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &tail_instr_ref));
    REQUIRE(tail_instr_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *tail_instr, *tail_prev_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, tail_instr_ref, &tail_instr));
    REQUIRE(tail_instr->operation.opcode == KEFIR_OPT_OPCODE_RETURN, KEFIR_OK);
    REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, tail_instr_ref, &tail_prev_instr_ref));
    REQUIRE(tail_prev_instr_ref != KEFIR_ID_NONE && tail_prev_instr_ref == tail_instr->operation.parameters.refs[0],
            KEFIR_OK);
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, tail_prev_instr_ref, &tail_prev_instr));
    REQUIRE(tail_prev_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
                tail_prev_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL,
            KEFIR_OK);

    const kefir_opt_call_id_t call_ref = tail_prev_instr->operation.parameters.function_call.call_ref;
    const struct kefir_opt_call_node *call_node;
    REQUIRE_OK(kefir_opt_code_container_call(&func->code, call_ref, &call_node));

    kefir_opt_call_id_t tail_call_ref;
    kefir_opt_instruction_ref_t tail_call_instr_ref;
    REQUIRE_OK(kefir_opt_code_container_new_tail_call(
        mem, &func->code, block_id, call_node->function_declaration_id, call_node->argument_count,
        tail_prev_instr->operation.parameters.function_call.indirect_ref, &tail_call_ref, &tail_call_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_call(&func->code, call_ref, &call_node));

    for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
        REQUIRE_OK(
            kefir_opt_code_container_call_set_argument(mem, &func->code, tail_call_ref, i, call_node->arguments[i]));
    }

    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_prev_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_prev_instr_ref));

    REQUIRE_OK(kefir_opt_code_container_add_control(&func->code, block_id, tail_call_instr_ref));

    return KEFIR_OK;
}

static kefir_result_t tail_call_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                      struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                      const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
        REQUIRE_OK(block_tail_call_apply(mem, func, block_id));
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassTailCalls = {
    .name = "tail-calls", .apply = tail_call_apply, .payload = NULL};
