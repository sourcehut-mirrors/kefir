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
#include "kefir/ast-translator/jump.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/flow_control.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/source_error.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdio.h>

static kefir_result_t match_constraints(const char *constraints, kefir_bool_t *immediate_constraint,
                                        kefir_bool_t *register_constraint, kefir_bool_t *memory_constraint,
                                        const struct kefir_source_location *source_location) {
    for (const char *iter = constraints; *iter != '\0'; ++iter) {
        switch (*iter) {
            case 'i':
                *immediate_constraint = true;
                break;

            case 'r':
                *register_constraint = true;
                break;
                ;

            case 'm':
                *memory_constraint = true;
                break;

            default:
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, source_location,
                                               "Unsupported inline assembly constraint '%s'", constraints);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_outputs(struct kefir_mem *mem, const struct kefir_ast_node_base *node,
                                        struct kefir_irbuilder_block *builder,
                                        struct kefir_ast_translator_context *context,
                                        struct kefir_ast_inline_assembly *inline_asm,
                                        struct kefir_ir_inline_assembly *ir_inline_asm, kefir_id_t *next_parameter_id,
                                        kefir_size_t *stack_slot_counter) {
    char buffer[512];
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->outputs); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, param, iter->value);

        kefir_id_t parameter_id = (*next_parameter_id)++;
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

        kefir_bool_t immediate_constraint = false;
        kefir_bool_t register_constraint = false;
        kefir_bool_t memory_constraint = false;
        REQUIRE_OK(match_constraints(param->constraint + 1, &immediate_constraint, &register_constraint,
                                     &memory_constraint, &node->source_location));
        if (register_constraint && memory_constraint) {
            constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY;
        } else if (register_constraint) {
            constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER;
        } else if (memory_constraint) {
            constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY;
        } else {
            return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                           "Unsupported inline assembly constraint '%s'", param->constraint);
        }

        const struct kefir_ast_type *param_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
            mem, context->ast_context->type_bundle, param->parameter->properties.type);
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to retrieve inline assembly parameter type"));

        REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
        struct kefir_irbuilder_type ir_type_builder;
        REQUIRE_OK(kefir_irbuilder_type_init(mem, &ir_type_builder, ir_type));
        kefir_result_t res =
            kefir_ast_translate_object_type(mem, param_type, 0, context->environment, &ir_type_builder, NULL);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder);
            return res;
        });
        REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder));

        stack_slot = --(*stack_slot_counter);

        REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, param->parameter));

        struct kefir_ir_inline_assembly_parameter *ir_inline_asm_param = NULL;
        REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(mem, context->ast_context->symbols, ir_inline_asm, name,
                                                          klass, constraint, ir_type, 0, stack_slot,
                                                          &ir_inline_asm_param));

        if (alias != NULL) {
            snprintf(buffer, sizeof(buffer) - 1, "[%s]", alias);
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter_alias(mem, context->ast_context->symbols, ir_inline_asm,
                                                                    ir_inline_asm_param, buffer));
        }
    }

    return KEFIR_OK;
}

static kefir_size_t resolve_identifier_offset(const struct kefir_ast_type_layout *layout) {
    if (layout->parent != NULL) {
        return resolve_identifier_offset(layout->parent) + layout->properties.relative_offset;
    } else {
        return layout->properties.relative_offset;
    }
}

static kefir_result_t translate_pointer_to_identifier(struct kefir_ast_constant_expression_value *value,
                                                      const char **base, kefir_int64_t *offset,
                                                      const struct kefir_source_location *location) {
    if (value->pointer.scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT) {
        switch (value->pointer.scoped_id->object.storage) {
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
                *base = value->pointer.base.literal;
                *offset = value->pointer.offset;
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 value->pointer.scoped_id->payload.ptr);
                *base = value->pointer.scoped_id->object.initializer != NULL
                            ? KEFIR_AST_TRANSLATOR_STATIC_VARIABLES_IDENTIFIER
                            : KEFIR_AST_TRANSLATOR_STATIC_UNINIT_VARIABLES_IDENTIFIER;
                *offset = resolve_identifier_offset(identifier_data->layout) + value->pointer.offset;
            } break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
                return KEFIR_SET_SOURCE_ERROR(
                    KEFIR_ANALYSIS_ERROR, location,
                    "Unexpected storage class of variable-based constant immediate inline assembly parameter");
        }
    } else {
        REQUIRE(value->pointer.scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Inline assembly immediate parameters can be initialized by pointer either to "
                                       "an object or to a function"));
        *base = value->pointer.base.literal;
        *offset = value->pointer.offset;
    }
    return KEFIR_OK;
}

static const kefir_ir_string_literal_type_t StringLiteralTypes[] = {
    [KEFIR_AST_STRING_LITERAL_MULTIBYTE] = KEFIR_IR_STRING_LITERAL_MULTIBYTE,
    [KEFIR_AST_STRING_LITERAL_UNICODE8] = KEFIR_IR_STRING_LITERAL_MULTIBYTE,
    [KEFIR_AST_STRING_LITERAL_UNICODE16] = KEFIR_IR_STRING_LITERAL_UNICODE16,
    [KEFIR_AST_STRING_LITERAL_UNICODE32] = KEFIR_IR_STRING_LITERAL_UNICODE32,
    [KEFIR_AST_STRING_LITERAL_WIDE] = KEFIR_IR_STRING_LITERAL_UNICODE32};

static kefir_result_t translate_inputs(struct kefir_mem *mem, const struct kefir_ast_node_base *node,
                                       struct kefir_irbuilder_block *builder,
                                       struct kefir_ast_translator_context *context,
                                       struct kefir_ast_inline_assembly *inline_asm,
                                       struct kefir_ir_inline_assembly *ir_inline_asm, kefir_id_t *next_parameter_id,
                                       kefir_size_t *stack_slot_counter) {
    char buffer[512];
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->inputs); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, param, iter->value);

        kefir_id_t parameter_id = (*next_parameter_id)++;
        snprintf(buffer, sizeof(buffer) - 1, KEFIR_ID_FMT, parameter_id);

        const char *name = buffer;
        kefir_ir_inline_assembly_parameter_class_t klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ;
        kefir_ir_inline_assembly_parameter_constraint_t constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_NONE;
        struct kefir_ir_type *ir_type = kefir_ir_module_new_type(mem, context->module, 0, NULL);
        const char *alias = param->parameter_name;
        kefir_ir_inline_assembly_immediate_type_t imm_type;
        const char *imm_identifier_base = NULL;
        kefir_id_t imm_literal_base = 0;
        kefir_int64_t param_value = 0;
        struct kefir_ir_inline_assembly_parameter *ir_inline_asm_param = NULL;

        kefir_bool_t register_constraint = false;
        kefir_bool_t memory_constraint = false;
        kefir_bool_t immediate_constraint = false;
        const char *constraint_str = param->constraint;
        if (*constraint_str >= '0' && *constraint_str <= '9') {
            char match_name[32] = {0};
            for (kefir_size_t i = 0; i < sizeof(match_name) - 1 && *constraint_str >= '0' && *constraint_str <= '9';
                 i++, constraint_str++) {
                match_name[i] = *constraint_str;
            }
            kefir_result_t res =
                kefir_ir_inline_assembly_resolve_parameter(ir_inline_asm, match_name, &ir_inline_asm_param);
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &inline_asm->base.source_location,
                                             "Unable to find matching parameter");
            }
            REQUIRE_OK(res);
            register_constraint = *constraint_str == '\0';
        }
        REQUIRE_OK(match_constraints(constraint_str, &immediate_constraint, &register_constraint, &memory_constraint,
                                     &node->source_location));

        if (immediate_constraint && param->parameter->properties.expression_props.constant_expression) {
            struct kefir_ast_constant_expression_value value;
            REQUIRE_OK(
                kefir_ast_constant_expression_value_evaluate(mem, context->ast_context, param->parameter, &value));
            if (value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER ||
                value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT) {
                imm_type = KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED;
                param_value = value.integer;
            } else if (value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS) {
                switch (value.pointer.type) {
                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER:
                        imm_type = KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED;
                        REQUIRE_OK(translate_pointer_to_identifier(&value, &imm_identifier_base, &param_value,
                                                                   &inline_asm->base.source_location));
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER:
                        imm_type = KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED;
                        param_value = value.pointer.base.integral + value.pointer.offset;
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL: {
                        imm_type = KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED;
                        kefir_id_t id;
                        REQUIRE_OK(kefir_ir_module_string_literal(
                            mem, context->module, StringLiteralTypes[value.pointer.base.string.type], true,
                            value.pointer.base.string.content, value.pointer.base.string.length, &id));
                        imm_literal_base = id;
                        param_value = value.pointer.offset;
                    } break;
                }
            } else {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &inline_asm->base.source_location,
                                              "Unexpected immediate inline assembly parameter type");
            }
            klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE;
        }

        if (klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            if (register_constraint && memory_constraint) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY;
            } else if (register_constraint) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER;
            } else if (memory_constraint) {
                constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY;
            } else {
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                               "Unable to satisfy inline assembly constraint '%s'", param->constraint);
            }
        }

        const struct kefir_ast_type *param_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
            mem, context->ast_context->type_bundle, param->parameter->properties.type);
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to retrieve inline assembly parameter type"));

        if (klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(param_type)) {
                (*stack_slot_counter) -= 2;
                param_value = (*stack_slot_counter);
            } else {
                param_value = --(*stack_slot_counter);
            }

            REQUIRE_OK(kefir_ast_translate_expression(mem, param->parameter, builder, context));
        }

        REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
        struct kefir_irbuilder_type ir_type_builder;
        REQUIRE_OK(kefir_irbuilder_type_init(mem, &ir_type_builder, ir_type));
        kefir_result_t res =
            kefir_ast_translate_object_type(mem, param_type, 0, context->environment, &ir_type_builder, NULL);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder);
            return res;
        });
        REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&ir_type_builder));

        if (ir_inline_asm_param == NULL && klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(mem, context->ast_context->symbols, ir_inline_asm, name,
                                                              klass, constraint, ir_type, 0, param_value,
                                                              &ir_inline_asm_param));
        } else if (ir_inline_asm_param == NULL && klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            REQUIRE_OK(kefir_ir_inline_assembly_add_immediate_parameter(
                mem, context->ast_context->symbols, ir_inline_asm, name, ir_type, 0, imm_type, imm_identifier_base,
                imm_literal_base, param_value, &ir_inline_asm_param));
        } else {
            REQUIRE(klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ &&
                        constraint == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &inline_asm->base.source_location,
                                           "Cannot assign matching constraint to the parameter"));
            REQUIRE(ir_inline_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE &&
                        ir_inline_asm_param->constraint == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &inline_asm->base.source_location,
                                           "Cannot assign matching constraint to the parameter"));
            REQUIRE_OK(kefir_ir_inline_assembly_parameter_read_from(mem, ir_inline_asm, ir_inline_asm_param, ir_type, 0,
                                                                    param_value));
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter_alias(mem, context->ast_context->symbols, ir_inline_asm,
                                                                    ir_inline_asm_param, buffer));
        }

        if (alias != NULL) {
            snprintf(buffer, sizeof(buffer) - 1, "[%s]", alias);
            REQUIRE_OK(kefir_ir_inline_assembly_add_parameter_alias(mem, context->ast_context->symbols, ir_inline_asm,
                                                                    ir_inline_asm_param, buffer));
        }
    }
    return KEFIR_OK;
}

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

            if (strchr(param->constraint, 'i') != NULL &&
                param->parameter->properties.expression_props.constant_expression &&
                KEFIR_AST_TYPE_IS_SCALAR_TYPE(param_type)) {
                continue;
            }

            if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(param_type)) {
                stack_slot_counter += 2;
            } else {
                stack_slot_counter++;
            }
        }

        // Translate outputs
        REQUIRE_OK(translate_outputs(mem, node, builder, context, inline_asm, ir_inline_asm, &next_parameter_id,
                                     &stack_slot_counter));

        // Translate inputs
        REQUIRE_OK(translate_inputs(mem, node, builder, context, inline_asm, ir_inline_asm, &next_parameter_id,
                                    &stack_slot_counter));

        // Translate clobbers
        for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->clobbers); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const char *, clobber, iter->value);

            REQUIRE_OK(
                kefir_ir_inline_assembly_add_clobber(mem, context->ast_context->symbols, ir_inline_asm, clobber));
        }

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_INLINEASM, ir_inline_asm_id));

        if (!kefir_hashtree_empty(&inline_asm->base.properties.inline_assembly.branching_point->branches)) {
            kefir_size_t patch_index = kefir_irblock_length(builder->block);
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_JMP, 0));
            struct kefir_hashtree_node_iterator iter;
            for (const struct kefir_hashtree_node *node =
                     kefir_hashtree_iter(&inline_asm->base.properties.inline_assembly.branching_point->branches, &iter);
                 node != NULL; node = kefir_hashtree_next(&iter)) {

                ASSIGN_DECL_CAST(const char *, jump_label, node->key);
                ASSIGN_DECL_CAST(struct kefir_ast_flow_control_point *, jump_target, node->value);

                kefir_size_t jump_trampoline = kefir_irblock_length(builder->block);
                REQUIRE_OK(kefir_ast_translate_jump(
                    mem, context, builder, inline_asm->base.properties.inline_assembly.origin_flow_control_point,
                    jump_target, &inline_asm->base.source_location));

                struct kefir_ir_inline_assembly_jump_target *ir_jump_target = NULL;
                kefir_id_t identifier = next_parameter_id++;
                snprintf(buffer, sizeof(buffer) - 1, KEFIR_ID_FMT, identifier);
                REQUIRE_OK(kefir_ir_inline_assembly_add_jump_target(
                    mem, context->ast_context->symbols, ir_inline_asm, buffer,
                    context->ast_context->surrounding_function_name, jump_trampoline, &ir_jump_target));
                snprintf(buffer, sizeof(buffer) - 1, "[%s]", jump_label);
                REQUIRE_OK(kefir_ir_inline_assembly_add_jump_target_alias(mem, context->ast_context->symbols,
                                                                          ir_inline_asm, ir_jump_target, buffer));
            }

            struct kefir_irinstr *instr = kefir_irblock_at(builder->block, patch_index);
            REQUIRE(instr != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR instruction from block"));
            instr->arg.u64 = kefir_irblock_length(builder->block);
        }
    } else {
        REQUIRE_OK(kefir_ir_module_inline_assembly_global(mem, context->module, ir_inline_asm_id));
    }
    return KEFIR_OK;
}
