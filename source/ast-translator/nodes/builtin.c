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

#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

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
            kefir_result_t res = kefir_ast_translate_object_type(mem, type_arg->properties.type, 0,
                                                                 context->environment, &type_builder, &type_layout);
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

            struct kefir_ast_designator designator = {.type = KEFIR_AST_DESIGNATOR_MEMBER,
                                                      .member = field->properties.expression_props.identifier,
                                                      .next = NULL};

            kefir_ast_target_environment_opaque_type_t opaque_type;
            REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context->ast_context->target_env,
                                                             offset_base->properties.type, &opaque_type));

            struct kefir_ast_target_environment_object_info objinfo;
            kefir_result_t res = KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->ast_context->target_env,
                                                                          opaque_type, &designator, &objinfo);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->ast_context->target_env, opaque_type);
                return res;
            });
            REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->ast_context->target_env, opaque_type));

            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, objinfo.relative_offset));
        } break;
    }
    return KEFIR_OK;
}
