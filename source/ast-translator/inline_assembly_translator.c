/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/ast-translator/translator.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/source_error.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdio.h>

kefir_result_t kefir_ast_translate_inline_assembly(struct kefir_mem *mem, const struct kefir_ast_node_base *node,
                                                   struct kefir_irbuilder_block *builder,
                                                   struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    kefir_result_t res;
    struct kefir_ast_inline_assembly *inline_asm = NULL;
    REQUIRE_MATCH_OK(&res, kefir_ast_downcast_inline_assembly(node, &inline_asm, false),
                     KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly"));

    kefir_id_t ir_inline_asm_id;
    struct kefir_ir_inline_assembly *ir_inline_asm =
        kefir_ir_module_new_inline_assembly(mem, context->module, inline_asm->asm_template, &ir_inline_asm_id);
    REQUIRE(ir_inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR inline assembly"));

    if (builder != NULL) {
        kefir_id_t next_parameter_id = 0;
        char buffer[512];
        kefir_size_t stack_slot_counter = 0;

        // Calculate amount of stack slots required for parameters
        stack_slot_counter = kefir_list_length(&inline_asm->outputs);
        for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->inputs); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, param, iter->value);
            const struct kefir_ast_type *param_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
                mem, context->ast_context->type_bundle, param->parameter->properties.type);
            REQUIRE(param_type != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to retrieve inline assembly parameter type"));

            if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(param_type)) {
                stack_slot_counter += 2;
            } else {
                stack_slot_counter++;
            }
        }

        // Translate outputs
        for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->outputs); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, param, iter->value);

            kefir_id_t parameter_id = next_parameter_id++;
            snprintf(buffer, sizeof(buffer) - 1, KEFIR_ID_FMT, parameter_id);

            const char *name = buffer;
            kefir_ir_inline_assembly_parameter_class_t klass;
            kefir_ir_inline_assembly_parameter_constraint_t constraint;
            struct kefir_ir_type *ir_type = kefir_ir_module_new_type(mem, context->module, 0, NULL);
            kefir_size_t stack_slot = 0;
            const char *alias = param->parameter_name;

            if (strncmp(param->constraint, "+", 1) == 0) {
                klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE;
            } else if (strncmp(param->constraint, "=", 1) == 0) {
                klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE;
            } else {
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                               "Unsupported inline assembly constraint '%s'", param->constraint);
            }

            if (strncmp(param->constraint + 1, "rm", 2) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY;
            } else if (strncmp(param->constraint + 1, "r", 1) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER;
            } else if (strncmp(param->constraint + 1, "m", 1) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY;
            } else {
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                               "Unsupported inline assembly constraint '%s'", param->constraint);
            }

            REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            struct kefir_irbuilder_type ir_type_builder;
            REQUIRE_OK(kefir_irbuilder_type_init(mem, &ir_type_builder, ir_type));
            res = kefir_ast_translate_object_type(mem, param->parameter->properties.type, 0, context->environment,
                                                  &ir_type_builder, NULL);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder);
                return res;
            });
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder));

            const struct kefir_ast_type *param_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
                mem, context->ast_context->type_bundle, param->parameter->properties.type);
            REQUIRE(param_type != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to retrieve inline assembly parameter type"));
            stack_slot = --stack_slot_counter;

            REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, param->parameter));

            struct kefir_ir_inline_assembly_parameter *ir_inline_asm_param = NULL;
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(mem, context->ast_context->symbols, ir_inline_asm, name,
                                                              klass, constraint, ir_type, 0, stack_slot,
                                                              &ir_inline_asm_param));

            if (alias != NULL) {
                snprintf(buffer, sizeof(buffer) - 1, "[%s]", alias);
                REQUIRE_OK(kefir_ir_inline_assembly_add_parameter_alias(mem, context->ast_context->symbols,
                                                                        ir_inline_asm, ir_inline_asm_param, buffer));
            }
        }

        // Translate inputs
        for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->inputs); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, param, iter->value);

            REQUIRE(param->parameter_name == NULL,
                    KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                    "Named parameters in AST inline assembly directives are not supported yet"));

            kefir_id_t parameter_id = next_parameter_id++;
            snprintf(buffer, sizeof(buffer) - 1, KEFIR_ID_FMT, parameter_id);

            const char *name = buffer;
            kefir_ir_inline_assembly_parameter_class_t klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ;
            kefir_ir_inline_assembly_parameter_constraint_t constraint;
            struct kefir_ir_type *ir_type = kefir_ir_module_new_type(mem, context->module, 0, NULL);
            kefir_size_t stack_slot = 0;
            const char *alias = param->parameter_name;

            if (strncmp(param->constraint, "rm", 2) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY;
            } else if (strncmp(param->constraint, "r", 1) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER;
            } else if (strncmp(param->constraint, "m", 1) == 0) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY;
            } else {
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                               "Unsupported inline assembly constraint '%s'", param->constraint);
            }

            REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            struct kefir_irbuilder_type ir_type_builder;
            REQUIRE_OK(kefir_irbuilder_type_init(mem, &ir_type_builder, ir_type));
            res = kefir_ast_translate_object_type(mem, param->parameter->properties.type, 0, context->environment,
                                                  &ir_type_builder, NULL);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder);
                return res;
            });
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder));

            const struct kefir_ast_type *param_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
                mem, context->ast_context->type_bundle, param->parameter->properties.type);
            REQUIRE(param_type != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to retrieve inline assembly parameter type"));
            if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(param_type)) {
                stack_slot_counter -= 2;
                stack_slot = stack_slot_counter;
            } else {
                stack_slot = --stack_slot_counter;
            }

            REQUIRE_OK(kefir_ast_translate_expression(mem, param->parameter, builder, context));

            struct kefir_ir_inline_assembly_parameter *ir_inline_asm_param = NULL;
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(mem, context->ast_context->symbols, ir_inline_asm, name,
                                                              klass, constraint, ir_type, 0, stack_slot,
                                                              &ir_inline_asm_param));
            if (alias != NULL) {
                snprintf(buffer, sizeof(buffer) - 1, "[%s]", alias);
                REQUIRE_OK(kefir_ir_inline_assembly_add_parameter_alias(mem, context->ast_context->symbols,
                                                                        ir_inline_asm, ir_inline_asm_param, buffer));
            }
        }

        // Translate clobbers
        for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->clobbers); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const char *, clobber, iter->value);

            REQUIRE_OK(
                kefir_ir_inline_assembly_add_clobber(mem, context->ast_context->symbols, ir_inline_asm, clobber));
        }

        REQUIRE(kefir_list_length(&inline_asm->jump_labels) == 0,
                KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                "Jump labels in AST inline assembly directives are not supported yet"));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_INLINEASM, ir_inline_asm_id));
    } else {
        REQUIRE_OK(kefir_ir_module_inline_assembly_global(mem, context->module, ir_inline_asm_id));
    }
    return KEFIR_OK;
}
