/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

static kefir_result_t simplify_get_global(struct kefir_mem *mem, struct kefir_opt_function *func,
                                          const struct kefir_opt_instruction *instr,
                                          kefir_opt_instruction_ref_t *replacement_ref) {
    UNUSED(mem);
    if (instr->operation.parameters.variable.offset != 0) {
        kefir_opt_block_id_t block_id = instr->block_id;
        kefir_opt_instruction_ref_t offset_ref, global_ref;
        struct kefir_opt_operation oper = instr->operation;
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, oper.parameters.variable.offset,
                                                       &offset_ref));
        if (oper.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
            REQUIRE_OK(kefir_opt_code_builder_get_global(mem, &func->code, block_id,
                                                         oper.parameters.variable.global_ref, 0, &global_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_get_thread_local(mem, &func->code, block_id,
                                                               oper.parameters.variable.global_ref, 0, &global_ref));
        }
        REQUIRE_OK(
            kefir_opt_code_builder_int64_add(mem, &func->code, block_id, global_ref, offset_ref, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t canonicalize_impl(struct kefir_mem *mem, struct kefir_opt_function *func) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;
        kefir_bool_t fixpoint_reached = false;
        while (!fixpoint_reached) {
            fixpoint_reached = true;
            for (kefir_opt_code_block_instr_head(&func->code, block->id, &instr_id); instr_id != KEFIR_ID_NONE;) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference_of(&func->debug_info, instr_id));
                kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
                kefir_opt_instruction_ref_t next_instr_ref;
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &next_instr_ref));
                kefir_bool_t drop_instr = false;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_GET_GLOBAL:
                    case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
                        REQUIRE_OK(simplify_get_global(mem, func, instr, &replacement_ref));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }

                if (replacement_ref != KEFIR_ID_NONE) {
                    fixpoint_reached = false;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                    REQUIRE_OK(
                        kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr_id));
                    kefir_bool_t is_control_flow, is_replacement_control_flow;
                    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_id, &is_control_flow));
                    if (is_control_flow) {
                        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, replacement_ref,
                                                                              &is_replacement_control_flow));
                        if (!is_replacement_control_flow) {
                            REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block->id, instr_id,
                                                                               replacement_ref));
                        }
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                    }
                    kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
                    REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference(
                        &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE));
                } else if (drop_instr) {
                    kefir_bool_t is_control_flow;
                    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_id, &is_control_flow));
                    if (is_control_flow) {
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                    }
                    kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
                    REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference(
                        &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE));
                } else {
                    instr_id = next_instr_ref;
                }
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t canonicalize_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                         struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                         const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    REQUIRE_OK(canonicalize_impl(mem, func));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassCanonicalize = {
    .name = "canonicalize", .apply = canonicalize_apply, .payload = NULL};
