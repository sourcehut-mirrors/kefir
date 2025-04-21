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

#include "kefir/optimizer/analysis.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_code_analysis_init(struct kefir_opt_code_analysis *analysis) {
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_structure_init(&analysis->structure));
    REQUIRE_OK(kefir_opt_code_liveness_init(&analysis->liveness));
    REQUIRE_OK(kefir_opt_code_variable_conflicts_init(&analysis->variable_conflicts));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analyze(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                      struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &num_of_blocks));

    REQUIRE_OK(kefir_opt_code_structure_build(mem, &analysis->structure, code));
    REQUIRE_OK(kefir_opt_code_liveness_build(mem, &analysis->liveness, &analysis->structure));
    REQUIRE_OK(kefir_opt_code_variable_conflicts_build(mem, &analysis->variable_conflicts, &analysis->liveness));
    REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, &analysis->structure));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_free(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_variable_conflicts_free(mem, &analysis->variable_conflicts));
    REQUIRE_OK(kefir_opt_code_liveness_free(mem, &analysis->liveness));
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &analysis->structure));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_clear(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_liveness_free(mem, &analysis->liveness));
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &analysis->structure));

    REQUIRE_OK(kefir_opt_code_structure_init(&analysis->structure));
    REQUIRE_OK(kefir_opt_code_liveness_init(&analysis->liveness));
    return KEFIR_OK;
}

static kefir_result_t free_code_analysis(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                         kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_analysis *, analysis, value);
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_analysis_free(mem, analysis));
    KEFIR_FREE(mem, analysis);
    return KEFIR_OK;
}

static kefir_result_t module_analyze_impl(struct kefir_mem *mem, struct kefir_opt_module_analysis *analysis) {
    REQUIRE_OK(kefir_opt_module_liveness_trace(mem, &analysis->liveness, analysis->module));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *func = kefir_ir_module_function_iter(analysis->module->ir_module, &iter);
         func != NULL; func = kefir_ir_module_function_next(&iter)) {

        struct kefir_opt_function *opt_func = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(analysis->module, func->declaration->id, &opt_func));

        struct kefir_opt_code_analysis *code_analysis = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_analysis));
        REQUIRE(code_analysis != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code analysis data"));

        kefir_result_t res = kefir_opt_code_analysis_init(code_analysis);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, code_analysis);
            return res;
        });

        res = kefir_opt_code_analyze(mem, &opt_func->code, code_analysis);
        ;
        REQUIRE_CHAIN(&res,
                      kefir_hashtree_insert(mem, &analysis->functions, (kefir_hashtree_key_t) func->declaration->id,
                                            (kefir_hashtree_value_t) code_analysis));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_analysis_free(mem, code_analysis);
            KEFIR_FREE(mem, code_analysis);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analyze(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module analysis"));

    REQUIRE_OK(kefir_hashtree_init(&analysis->functions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&analysis->functions, free_code_analysis, NULL));
    REQUIRE_OK(kefir_opt_module_liveness_init(&analysis->liveness));
    analysis->module = module;

    kefir_result_t res = module_analyze_impl(mem, analysis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_module_liveness_free(mem, &analysis->liveness);
        kefir_hashtree_free(mem, &analysis->functions);
        analysis->module = NULL;
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analysis_free(struct kefir_mem *mem, struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));

    REQUIRE_OK(kefir_opt_module_liveness_free(mem, &analysis->liveness));
    REQUIRE_OK(kefir_hashtree_free(mem, &analysis->functions));
    analysis->module = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analysis_get_function(const struct kefir_opt_module_analysis *analysis,
                                                      kefir_id_t func_id,
                                                      const struct kefir_opt_code_analysis **code_analysis_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));
    REQUIRE(code_analysis_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&analysis->functions, (kefir_hashtree_key_t) func_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer code analysis");
    }
    REQUIRE_OK(res);

    *code_analysis_ptr = (const struct kefir_opt_code_analysis *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_liveness_init(struct kefir_opt_module_liveness *liveness) {
    REQUIRE(liveness != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module liveness"));

    REQUIRE_OK(kefir_hashtreeset_init(&liveness->symbols, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&liveness->string_literals, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_liveness_free(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module liveness"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &liveness->string_literals));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &liveness->symbols));
    return KEFIR_OK;
}

static kefir_result_t trace_inline_asm(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                       struct kefir_queue *symbol_queue,
                                       const struct kefir_ir_inline_assembly *inline_asm) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_parameter *, param, iter->value);
        if (param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            switch (param->immediate_type) {
                case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED:
                    REQUIRE_OK(
                        kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) param->immediate_identifier_base));
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED:
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &liveness->string_literals,
                                                     (kefir_hashtreeset_entry_t) param->immediate_literal_base));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);
        REQUIRE_OK(kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) jump_target->target_function));
    }

    return KEFIR_OK;
}

struct function_trace_payload {
    struct kefir_mem *mem;
    struct kefir_opt_module_liveness *liveness;
    const struct kefir_opt_module *module;
    const struct kefir_opt_function *function;
    struct kefir_queue *symbol_queue;
};

static kefir_result_t trace_instruction(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct function_trace_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module function trace payload"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->function->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INVOKE:
        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL: {
            const struct kefir_opt_call_node *call;
            REQUIRE_OK(kefir_opt_code_container_call(&param->function->code,
                                                     instr->operation.parameters.function_call.call_ref, &call));

            const struct kefir_ir_function_decl *decl =
                kefir_ir_module_get_declaration(param->module->ir_module, call->function_declaration_id);
            REQUIRE(decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));
            if (decl->name != NULL) {
                REQUIRE_OK(kefir_queue_push(param->mem, param->symbol_queue, (kefir_queue_entry_t) decl->name));
            }
        } break;

        case KEFIR_OPT_OPCODE_GET_GLOBAL:
        case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL: {
            const char *symbol = kefir_ir_module_get_named_symbol(param->module->ir_module,
                                                                  instr->operation.parameters.variable.global_ref);
            REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));
            REQUIRE_OK(kefir_queue_push(param->mem, param->symbol_queue, (kefir_queue_entry_t) symbol));
        } break;

        case KEFIR_OPT_OPCODE_STRING_REF:
            REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->liveness->string_literals,
                                             (kefir_hashtreeset_entry_t) instr->operation.parameters.imm.string_ref));
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *opt_inline_asm;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                &param->function->code, instr->operation.parameters.inline_asm_ref, &opt_inline_asm));
            const struct kefir_ir_inline_assembly *inline_asm =
                kefir_ir_module_get_inline_assembly(param->module->ir_module, opt_inline_asm->inline_asm_id);
            REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR inline assembly"));
            REQUIRE_OK(trace_inline_asm(param->mem, param->liveness, param->symbol_queue, inline_asm));
        } break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t trace_function(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                     const struct kefir_opt_module *module, const struct kefir_opt_function *function,
                                     struct kefir_queue *symbol_queue) {
    struct function_trace_payload payload = {
        .mem = mem, .liveness = liveness, .module = module, .function = function, .symbol_queue = symbol_queue};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = trace_instruction, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_container_trace(mem, &function->code, &tracer));
    return KEFIR_OK;
}

static kefir_result_t trace_data(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                 const struct kefir_ir_data *data, struct kefir_queue *symbol_queue) {
    for (kefir_size_t i = 0; i < data->total_length; i++) {
        const struct kefir_ir_data_value *value;
        REQUIRE_OK(kefir_ir_data_value_at(data, i, &value));

        if (!value->defined) {
            continue;
        }

        switch (value->type) {
            case KEFIR_IR_DATA_VALUE_UNDEFINED:
            case KEFIR_IR_DATA_VALUE_INTEGER:
            case KEFIR_IR_DATA_VALUE_FLOAT32:
            case KEFIR_IR_DATA_VALUE_FLOAT64:
            case KEFIR_IR_DATA_VALUE_LONG_DOUBLE:
            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32:
            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64:
            case KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE:
            case KEFIR_IR_DATA_VALUE_STRING:
            case KEFIR_IR_DATA_VALUE_RAW:
            case KEFIR_IR_DATA_VALUE_AGGREGATE:
            case KEFIR_IR_DATA_VALUE_BITS:
                // Intentionally left blank
                break;

            case KEFIR_IR_DATA_VALUE_STRING_POINTER:
                REQUIRE_OK(kefir_hashtreeset_add(mem, &liveness->string_literals,
                                                 (kefir_hashtreeset_entry_t) value->value.string_ptr.id));
                break;

            case KEFIR_IR_DATA_VALUE_POINTER:
                REQUIRE_OK(kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) value->value.pointer.reference));
                break;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t initialize_symbol_queue(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                              const struct kefir_opt_module *module, struct kefir_queue *symbol_queue) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->ir_module->identifiers, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_identifier *, ir_identifier, node->value);
        if (ir_identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_EXPORT ||
            ir_identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK) {
            REQUIRE_OK(kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) symbol));
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->ir_module->functions, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_function *, function, node->value);
        if (function->flags.constructor || function->flags.destructor) {
            REQUIRE_OK(kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) symbol));
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->ir_module->global_inline_asm, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly *, inline_asm, node->value);
        REQUIRE_OK(trace_inline_asm(mem, liveness, symbol_queue, inline_asm));
    }

    return KEFIR_OK;
}

static kefir_result_t liveness_trace(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                     const struct kefir_opt_module *module, struct kefir_queue *symbol_queue) {
    REQUIRE_OK(initialize_symbol_queue(mem, liveness, module, symbol_queue));

    while (!kefir_queue_is_empty(symbol_queue)) {
        kefir_queue_entry_t entry;
        REQUIRE_OK(kefir_queue_pop_first(mem, symbol_queue, &entry));
        ASSIGN_DECL_CAST(const char *, symbol, entry);
        if (kefir_hashtreeset_has(&liveness->symbols, (kefir_hashtree_key_t) symbol)) {
            continue;
        }

        struct kefir_hashtree_node *node = NULL;
        kefir_result_t res = kefir_hashtree_at(&module->ir_module->functions, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(const struct kefir_ir_function *, func, node->value);
            struct kefir_opt_function *opt_func;
            REQUIRE_OK(kefir_opt_module_get_function(module, func->declaration->id, &opt_func));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &liveness->symbols, (kefir_hashtreeset_entry_t) symbol));
            REQUIRE_OK(trace_function(mem, liveness, module, opt_func, symbol_queue));
        }

        res = kefir_hashtree_at(&module->ir_module->named_data, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_ir_data *, data, node->value);
            REQUIRE_OK(kefir_hashtreeset_add(mem, &liveness->symbols, (kefir_hashtreeset_entry_t) symbol));
            REQUIRE_OK(trace_data(mem, liveness, data, symbol_queue));
        }

        res = kefir_hashtree_at(&module->ir_module->identifiers, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_ir_identifier *, ir_identifier, node->value);
            REQUIRE_OK(kefir_hashtreeset_add(mem, &liveness->symbols, (kefir_hashtreeset_entry_t) symbol));
            if (ir_identifier->alias != NULL) {
                REQUIRE_OK(kefir_queue_push(mem, symbol_queue, (kefir_queue_entry_t) ir_identifier->alias));
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_liveness_trace(struct kefir_mem *mem, struct kefir_opt_module_liveness *liveness,
                                               const struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module liveness"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));

    struct kefir_queue symbol_queue;
    REQUIRE_OK(kefir_queue_init(&symbol_queue));
    kefir_result_t res = liveness_trace(mem, liveness, module, &symbol_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &symbol_queue);
        return res;
    });

    REQUIRE_OK(kefir_queue_free(mem, &symbol_queue));
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_module_is_symbol_alive(const struct kefir_opt_module_liveness *liveness, const char *symbol) {
    REQUIRE(liveness != NULL, false);
    return kefir_hashtreeset_has(&liveness->symbols, (kefir_hashtreeset_entry_t) symbol);
}

kefir_bool_t kefir_opt_module_is_string_literal_alive(const struct kefir_opt_module_liveness *liveness,
                                                      kefir_id_t literal_id) {
    REQUIRE(liveness != NULL, false);
    return kefir_hashtreeset_has(&liveness->string_literals, (kefir_hashtreeset_entry_t) literal_id);
}
