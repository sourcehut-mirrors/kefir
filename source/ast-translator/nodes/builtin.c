/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t resolve_vararg(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                     struct kefir_irbuilder_block *builder, const struct kefir_ast_node_base *vararg) {
    if (vararg->properties.type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        REQUIRE_OK(kefir_ast_translate_expression(mem, vararg, builder, context));
    } else {
        REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, vararg));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_builtin_node(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                struct kefir_irbuilder_block *builder,
                                                const struct kefir_ast_builtin *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST builtin node"));

    const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
    switch (node->builtin) {
        case KEFIR_AST_BUILTIN_VA_START: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_VARARG_START, 0));
        } break;

        case KEFIR_AST_BUILTIN_VA_END: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_VARARG_END, 0));
        } break;

        case KEFIR_AST_BUILTIN_VA_ARG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));

            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type_arg, iter->value);

            kefir_id_t va_type_id;
            struct kefir_irbuilder_type type_builder;
            struct kefir_ast_type_layout *type_layout;

            struct kefir_ir_type *va_type = kefir_ir_module_new_type(mem, context->module, 0, &va_type_id);
            REQUIRE_OK(kefir_irbuilder_type_init(mem, &type_builder, va_type));
            kefir_result_t res = kefir_ast_translate_object_type(mem, context->ast_context, type_arg->properties.type,
                                                                 0, context->environment, &type_builder, &type_layout,
                                                                 &node->base.source_location);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&type_builder);
                return res;
            });

            const struct kefir_ast_type *layout_type = type_layout->type;
            kefir_size_t type_layout_idx = type_layout->value;

            res = kefir_ast_type_layout_free(mem, type_layout);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&type_builder);
                return res;
            });
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&type_builder));

            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_VARARG_GET, va_type_id, type_layout_idx));
            if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(layout_type)) {
                REQUIRE_OK(
                    kefir_ast_translate_typeconv_normalize(builder, context->ast_context->type_traits, layout_type));
            }
        } break;

        case KEFIR_AST_BUILTIN_VA_COPY: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, dst_vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, dst_vararg));

            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, src_vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, src_vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_VARARG_COPY, 0));
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, size, builder, context));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PUSHI64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ALLOCA, 1));
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, size, builder, context));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, alignment, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, alignment, builder, context));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ALLOCA, 1));
        } break;

        case KEFIR_AST_BUILTIN_OFFSETOF: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, offset_base, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, field, iter->value);

            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0));
            REQUIRE_OK(
                kefir_ast_translate_member_designator(mem, field, offset_base->properties.type, builder, context));
        } break;

        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type2_node, iter->value);

            const struct kefir_ast_type *type1 = kefir_ast_unqualified_type(type1_node->properties.type);
            const struct kefir_ast_type *type2 = kefir_ast_unqualified_type(type2_node->properties.type);

            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                builder, KEFIR_IROPCODE_PUSHU64,
                KEFIR_AST_TYPE_COMPATIBLE(context->ast_context->type_traits, type1, type2) ? 1 : 0));
        } break;

        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, cond_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr2_node, iter->value);

            struct kefir_ast_constant_expression_value cond_value;
            REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context->ast_context, cond_node, &cond_value));

            REQUIRE(cond_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cond_node->source_location,
                                           "Expected a constant expression evaluating to an integer"));

            if (cond_value.integer != 0) {
                REQUIRE_OK(kefir_ast_translate_expression(mem, expr1_node, builder, context));
            } else {
                REQUIRE_OK(kefir_ast_translate_expression(mem, expr2_node, builder, context));
            }
        } break;

        case KEFIR_AST_BUILTIN_CONSTANT: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            struct kefir_ast_constant_expression_value node_value;
            kefir_result_t res =
                kefir_ast_constant_expression_value_evaluate(mem, context->ast_context, node, &node_value);

            if (res == KEFIR_NOT_CONSTANT) {
                kefir_clear_error();
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0));
            } else {
                REQUIRE_OK(res);
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                    builder, KEFIR_IROPCODE_PUSHU64,
                    (node_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER ||
                     node_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT ||
                     node_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT ||
                     (node_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS &&
                      node_value.pointer.type != KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER))
                        ? 1
                        : 0));
            }
        } break;

        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            kefir_int_t klass;
            REQUIRE_OK(kefir_ast_type_classify(node->properties.type, &klass));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHI64, klass));
        } break;
    }
    return KEFIR_OK;
}
