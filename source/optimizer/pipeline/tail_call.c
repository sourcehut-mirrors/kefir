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
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/structure.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct escape_analysis_param {
    struct kefir_mem *mem;
    const struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    kefir_bool_t tail_call_possible;
    struct kefir_hashtreeset no_escapes;
    struct kefir_hashtreeset visited_instr;
};

static kefir_result_t check_escape(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    REQUIRE(instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    ASSIGN_DECL_CAST(struct escape_analysis_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid escape analysis payload"));
    REQUIRE(!kefir_hashtreeset_has(&param->visited_instr, (kefir_hashtreeset_entry_t) instr_ref) &&
                !kefir_hashtreeset_has(&param->no_escapes, (kefir_hashtreeset_entry_t) instr_ref),
            KEFIR_OK);

    REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->visited_instr, (kefir_hashtreeset_entry_t) instr_ref));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_ALLOC_LOCAL:
        case KEFIR_OPT_OPCODE_STACK_ALLOC:
        case KEFIR_OPT_OPCODE_SCOPE_PUSH:
        case KEFIR_OPT_OPCODE_SCOPE_POP:
            param->tail_call_possible = false;
            return KEFIR_YIELD;

        default:
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(&param->func->code, instr, false, check_escape, payload));
            break;
    }

    if (param->tail_call_possible) {
        REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->no_escapes, (kefir_hashtreeset_entry_t) instr_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t escape_analyze(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct escape_analysis_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid escape analysis payload"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INVOKE:
        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL: {
            const struct kefir_opt_call_node *call_node;
            REQUIRE_OK(kefir_opt_code_container_call(&param->func->code,
                                                     instr->operation.parameters.function_call.call_ref, &call_node));

            const struct kefir_ir_function_decl *ir_func_decl =
                kefir_ir_module_get_declaration(param->module->ir_module, call_node->function_declaration_id);
            REQUIRE(ir_func_decl != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

            if (ir_func_decl->returns_twice) {
                param->tail_call_possible = false;
                return KEFIR_YIELD;
            }

            for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
                const struct kefir_ir_typeentry *typeentry =
                    kefir_ir_type_at(ir_func_decl->params, kefir_ir_type_child_index(ir_func_decl->params, i));
                REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type entry"));
                switch (typeentry->typecode) {
                    case KEFIR_IR_TYPE_STRUCT:
                    case KEFIR_IR_TYPE_UNION:
                    case KEFIR_IR_TYPE_ARRAY:
                        // Intentionally left blank
                        break;

                    default:
                        REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
                        REQUIRE_OK(check_escape(call_node->arguments[i], param));
                        break;
                }
            }
            // Despite being part of call node, return space does not escape per se (i.e. for callee it is legal to use
            // only as long as the function does not return, thus no escape can happen).
        } break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *inline_asm_node;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                &param->func->code, instr->operation.parameters.inline_asm_ref, &inline_asm_node));

            for (kefir_size_t i = 0; i < inline_asm_node->parameter_count; i++) {
                REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
                REQUIRE_OK(check_escape(inline_asm_node->parameters[i].load_store_ref, param));
                REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
                REQUIRE_OK(check_escape(inline_asm_node->parameters[i].read_ref, param));
            }
        } break;

        case KEFIR_OPT_OPCODE_INT8_LOAD:
        case KEFIR_OPT_OPCODE_INT16_LOAD:
        case KEFIR_OPT_OPCODE_INT32_LOAD:
        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            if (instr->operation.parameters.memory_access.flags.volatile_access) {
                REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
                REQUIRE_OK(check_escape(instr->operation.parameters.refs[0], param));
            }
            break;

        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
            REQUIRE_OK(check_escape(instr->operation.parameters.refs[1], param));
            if (instr->operation.parameters.memory_access.flags.volatile_access) {
                REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
                REQUIRE_OK(check_escape(instr->operation.parameters.refs[0], param));
            }
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
            REQUIRE_OK(check_escape(instr->operation.parameters.refs[0], param));
            REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
            REQUIRE_OK(check_escape(instr->operation.parameters.refs[1], param));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
            REQUIRE_OK(check_escape(instr->operation.parameters.refs[0], param));
            REQUIRE_OK(kefir_hashtreeset_clean(param->mem, &param->visited_instr));
            REQUIRE_OK(check_escape(instr->operation.parameters.refs[1], param));
            break;

        default:
            // Intentionally left blank
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t block_tail_call_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_function *func, kefir_opt_block_id_t block_id) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

    kefir_opt_instruction_ref_t tail_instr_ref, call_instr_ref, prev_tail_instr_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &tail_instr_ref));
    REQUIRE(tail_instr_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *tail_instr, *call_instr, *prev_tail_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, tail_instr_ref, &tail_instr));
    REQUIRE(tail_instr->operation.opcode == KEFIR_OPT_OPCODE_RETURN, KEFIR_OK);

    call_instr_ref = KEFIR_ID_NONE;
    REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, tail_instr_ref, &prev_tail_instr_ref));
    for (; call_instr_ref == KEFIR_ID_NONE;) {
        REQUIRE(prev_tail_instr_ref != KEFIR_ID_NONE, KEFIR_OK);
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, prev_tail_instr_ref, &prev_tail_instr));
        if (prev_tail_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
            prev_tail_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL) {
            REQUIRE(prev_tail_instr_ref == tail_instr->operation.parameters.refs[0], KEFIR_OK);
            call_instr_ref = prev_tail_instr_ref;
            call_instr = prev_tail_instr;
        } else {
            REQUIRE(prev_tail_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK, KEFIR_OK);
            REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, prev_tail_instr_ref, &prev_tail_instr_ref));
        }
    }

    kefir_opt_instruction_ref_t sole_use_ref;
    REQUIRE_OK(kefir_opt_instruction_get_sole_use(&func->code, call_instr_ref, &sole_use_ref));
    REQUIRE(sole_use_ref == tail_instr_ref, KEFIR_OK);

    struct escape_analysis_param param = {.mem = mem, .module = module, .func = func, .tail_call_possible = true};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = escape_analyze, .payload = &param};
    REQUIRE_OK(kefir_hashtreeset_init(&param.no_escapes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&param.visited_instr, &kefir_hashtree_uint_ops));
    kefir_result_t res = kefir_opt_code_container_trace(mem, &func->code, &tracer);
    REQUIRE_ELSE(res == KEFIR_OK || res == KEFIR_YIELD, {
        kefir_hashtreeset_free(mem, &param.visited_instr);
        kefir_hashtreeset_free(mem, &param.no_escapes);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &param.visited_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &param.visited_instr);
        kefir_hashtreeset_free(mem, &param.no_escapes);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &param.no_escapes));
    REQUIRE(param.tail_call_possible, KEFIR_OK);

    const kefir_opt_call_id_t call_ref = call_instr->operation.parameters.function_call.call_ref;
    const struct kefir_opt_call_node *call_node;
    REQUIRE_OK(kefir_opt_code_container_call(&func->code, call_ref, &call_node));

    kefir_opt_call_id_t tail_call_ref;
    kefir_opt_instruction_ref_t tail_call_instr_ref;
    REQUIRE_OK(kefir_opt_code_container_new_tail_call(
        mem, &func->code, block_id, call_node->function_declaration_id, call_node->argument_count,
        call_instr->operation.parameters.function_call.indirect_ref, &tail_call_ref, &tail_call_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_call(&func->code, call_ref, &call_node));
    if (call_node->return_space != KEFIR_ID_NONE) {
        REQUIRE_OK(
            kefir_opt_code_container_call_set_return_space(mem, &func->code, tail_call_ref, call_node->return_space));
    }

    for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
        REQUIRE_OK(
            kefir_opt_code_container_call_set_argument(mem, &func->code, tail_call_ref, i, call_node->arguments[i]));
    }

    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, call_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, call_instr_ref));

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
        REQUIRE_OK(block_tail_call_apply(mem, module, func, block_id));
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassTailCalls = {
    .name = "tail-calls", .apply = tail_call_apply, .payload = NULL};
