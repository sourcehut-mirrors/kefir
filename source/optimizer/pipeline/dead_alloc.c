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

#include "kefir/core/basic-types.h"
#include "kefir/optimizer/pipeline.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t dead_alloc_apply_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            kefir_bool_t *fixpoint_reached) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id, next_instr_id;
        const struct kefir_opt_instruction *instr = NULL;
        kefir_result_t res;
        for (res = kefir_opt_code_block_instr_head(&func->code, block->id, &instr_id); instr_id != KEFIR_ID_NONE;
             instr_id = next_instr_id) {
            if (res == KEFIR_ITERATOR_END) {
                next_instr_id = KEFIR_ID_NONE;
            } else {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_SCOPE) {
                kefir_bool_t only_lifetime_mark_uses = true;
                struct kefir_opt_instruction_use_iterator use_iter;
                for (res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, instr_id, &use_iter);
                     res == KEFIR_OK && only_lifetime_mark_uses;
                     res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
                    const struct kefir_opt_instruction *use_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_iter.use_instr_ref, &use_instr));

                    switch (use_instr->operation.opcode) {
                        case KEFIR_OPT_OPCODE_INT8_STORE:
                        case KEFIR_OPT_OPCODE_INT16_STORE:
                        case KEFIR_OPT_OPCODE_INT32_STORE:
                        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
                        case KEFIR_OPT_OPCODE_INT64_STORE:
                        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
                        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
                        case KEFIR_OPT_OPCODE_INT128_STORE:
                        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
                        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
                        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
                        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                            if (use_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF] == instr_id ||
                                use_instr->operation.parameters.memory_access.flags.volatile_access) {
                                only_lifetime_mark_uses = false;
                            }
                            break;

                        case KEFIR_OPT_OPCODE_ZERO_MEMORY:
                            // Intentionally left blank
                            break;

                        case KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK:
                            REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_SCOPE,
                                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                    "Expected local lifetime mark to reference local scope"));
                            break;

                        case KEFIR_OPT_OPCODE_REF_LOCAL: {
                            struct kefir_opt_instruction_use_iterator use_iter2;
                            for (res = kefir_opt_code_container_instruction_use_instr_iter(
                                     &func->code, use_iter.use_instr_ref, &use_iter2);
                                 res == KEFIR_OK && only_lifetime_mark_uses;
                                 res = kefir_opt_code_container_instruction_use_next(&use_iter2)) {
                                const struct kefir_opt_instruction *use_instr2 = NULL;
                                REQUIRE_OK(
                                    kefir_opt_code_container_instr(&func->code, use_iter2.use_instr_ref, &use_instr2));
                                switch (use_instr2->operation.opcode) {
                                    case KEFIR_OPT_OPCODE_INT8_STORE:
                                    case KEFIR_OPT_OPCODE_INT16_STORE:
                                    case KEFIR_OPT_OPCODE_INT32_STORE:
                                    case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
                                    case KEFIR_OPT_OPCODE_INT64_STORE:
                                    case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
                                    case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
                                    case KEFIR_OPT_OPCODE_INT128_STORE:
                                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
                                    case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
                                    case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                                    case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
                                    case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                                    case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                                        if (use_instr2->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF] ==
                                                use_iter.use_instr_ref ||
                                            use_instr2->operation.parameters.memory_access.flags.volatile_access) {
                                            only_lifetime_mark_uses = false;
                                        }
                                        break;

                                    case KEFIR_OPT_OPCODE_ZERO_MEMORY:
                                        // Intentionally left blank
                                        break;

                                    default:
                                        only_lifetime_mark_uses = false;
                                        break;
                                }
                            }
                        } break;

                        default:
                            only_lifetime_mark_uses = false;
                            break;
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }

                if (!only_lifetime_mark_uses) {
                    res = kefir_opt_instruction_next_sibling(&func->code, instr_id, &next_instr_id);
                    continue;
                }

                for (res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, instr_id, &use_iter);
                     res == KEFIR_OK;
                     res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, instr_id, &use_iter)) {
                    const struct kefir_opt_instruction *use_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_iter.use_instr_ref, &use_instr));
                    if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
                        struct kefir_opt_instruction_use_iterator use_iter2;
                        for (res = kefir_opt_code_container_instruction_use_instr_iter(
                                 &func->code, use_iter.use_instr_ref, &use_iter2);
                             res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_instr_iter(
                                                  &func->code, use_iter.use_instr_ref, &use_iter2)) {
                            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, use_iter2.use_instr_ref));
                            REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, use_iter2.use_instr_ref));
                        }
                    }

                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, use_iter.use_instr_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, use_iter.use_instr_ref));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                res = kefir_opt_instruction_next_sibling(&func->code, instr_id, &next_instr_id);
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_id));
                *fixpoint_reached = false;
                continue;
            } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
                const struct kefir_opt_instruction *arg_instr = NULL;
                REQUIRE_OK(
                    kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg_instr));
                REQUIRE(arg_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_SCOPE,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected local lifetime mark to reference local scope"));
            }
            res = kefir_opt_instruction_next_sibling(&func->code, instr_id, &next_instr_id);
        }
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t dead_alloc_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    kefir_bool_t fixpoint_reached = false;
    for (; !fixpoint_reached;) {
        fixpoint_reached = true;
        REQUIRE_OK(dead_alloc_apply_impl(mem, func, &fixpoint_reached));
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassDeadAlloc = {
    .name = "dead-alloc", .apply = dead_alloc_apply, .payload = NULL};
