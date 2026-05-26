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
#include "kefir/optimizer/configuration.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/trace.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/inline.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct inline_candidate_instr_tracer {
    const struct kefir_opt_module *module;
    const struct kefir_opt_code_container *code;
    kefir_size_t instructions;
    kefir_bool_t leaf_function;
    kefir_bool_t noinline;
};

#define TRACER_INSTR_LIMIT 128

static kefir_result_t trace_instruction(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    UNUSED(instr_ref);
    ASSIGN_DECL_CAST(struct inline_candidate_instr_tracer *, tracer, payload);
    REQUIRE(tracer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code tracer payload"));

    tracer->instructions++;
    REQUIRE(tracer->instructions < TRACER_INSTR_LIMIT, KEFIR_YIELD);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(tracer->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INVOKE: {
            const struct kefir_opt_call_node *call_node;
            REQUIRE_OK(kefir_opt_code_container_call(tracer->code, instr->operation.parameters.function_call.call_ref,
                                                     &call_node));

            const struct kefir_ir_function_decl *ir_func_decl =
                kefir_ir_module_get_declaration(tracer->module->ir_module, call_node->function_declaration_id);
            REQUIRE(ir_func_decl != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));

            static const char BUILTIN_PREFIX[] = "__kefir_builtin";
            if (strncmp(BUILTIN_PREFIX, ir_func_decl->name, sizeof(BUILTIN_PREFIX) - 1) != 0) {
                tracer->leaf_function = false;
            } else if (strcmp(ir_func_decl->name, "__kefir_builtin_frame_address") == 0 ||
                       strcmp(ir_func_decl->name, "__kefir_builtin_return_address") == 0) {
                tracer->noinline = true;
                return KEFIR_YIELD;
            }
        } break;

        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL:
            tracer->leaf_function = false;
            break;

        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL:
            tracer->noinline = true;
            return KEFIR_YIELD;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t is_inline_candidate(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                          struct kefir_opt_function *func, kefir_bool_t *candidate) {
    *candidate = false;

    if (func->ir_func->flags.inline_function_hint) {
        *candidate = true;
        return KEFIR_OK;
    }

    struct inline_candidate_instr_tracer instr_trace_payloer = {
        .module = module, .code = &func->code, .instructions = 0, .leaf_function = true, .noinline = false};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = trace_instruction,
                                                     .payload = &instr_trace_payloer};
    kefir_result_t res = kefir_opt_code_container_trace(mem, &func->code, &tracer);
    if (res == KEFIR_YIELD) {
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    if (!instr_trace_payloer.noinline &&
        (instr_trace_payloer.instructions <= 16 ||
         (instr_trace_payloer.instructions <= 24 && instr_trace_payloer.leaf_function))) {
        *candidate = true;
        return KEFIR_OK;
    }

    return KEFIR_OK;
}

static kefir_result_t inline_func_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                       struct kefir_opt_function *func,
                                       struct kefir_opt_code_control_flow *control_flow,
                                       struct kefir_opt_code_sequencing *sequencing,
                                       const struct kefir_optimizer_configuration *config,
                                       kefir_bool_t *fixpoint_reached) {
    for (kefir_opt_block_id_t block_id = 0; block_id < control_flow->num_of_blocks; block_id++) {
        kefir_bool_t reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, block_id, &reachable));
        if (!reachable) {
            continue;
        }

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(&func->code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
            kefir_bool_t inlined = false;
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE) {
                const struct kefir_opt_call_node *call_node;
                REQUIRE_OK(kefir_opt_code_container_call(
                    &func->code, instr->operation.parameters.function_call.call_ref, &call_node));

                const struct kefir_ir_function_decl *ir_func_decl =
                    kefir_ir_module_get_declaration(module->ir_module, call_node->function_declaration_id);
                REQUIRE(ir_func_decl != NULL,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));
                const struct kefir_ir_function *ir_func =
                    kefir_ir_module_get_function(module->ir_module, ir_func_decl->name);
                if (ir_func == NULL) {
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref));
                    continue;
                }
                struct kefir_opt_function *called_func;
                res = kefir_opt_module_get_function(module, ir_func->declaration->id, &called_func);
                if (res == KEFIR_NOT_FOUND) {
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref));
                    continue;
                }

                kefir_bool_t candidate = false;
                REQUIRE_OK(is_inline_candidate(mem, module, called_func, &candidate));
                if (candidate) {
                    REQUIRE_OK(kefir_opt_try_inline_function_call(
                        mem, module, func, control_flow, sequencing,
                        &(struct kefir_opt_try_inline_function_call_parameters) {
                            .max_inline_depth = config->max_inline_depth,
                            .max_inlines_per_function = config->max_inlines_per_function},
                        instr_ref, &inlined));
                }
            }
            if (inlined) {
                REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));
                REQUIRE_OK(kefir_opt_code_block_instr_head(&func->code, block_id, &instr_ref));
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

    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_sequencing sequencing;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_opt_code_sequencing_init(&sequencing));
    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &control_flow, &func->code);
    kefir_bool_t fixpoint_reached = false;
    while (!fixpoint_reached && res == KEFIR_OK) {
        fixpoint_reached = true;
        REQUIRE_CHAIN(&res, inline_func_impl(mem, module, func, &control_flow, &sequencing, config, &fixpoint_reached));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_sequencing_free(mem, &sequencing);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_sequencing_free(mem, &sequencing);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassInlineFunc = {
    .name = "inline-func", .apply = inline_func_apply, .payload = NULL};
