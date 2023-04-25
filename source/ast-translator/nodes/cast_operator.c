/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct typeconv_callback_param {
    struct kefir_mem *mem;
    struct kefir_ast_translator_context *context;
    struct kefir_irbuilder_block *builder;
    const struct kefir_ast_temporary_identifier *temporary;
};

static kefir_result_t allocate_long_double_callback(void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid typeconv callback payload"));
    ASSIGN_DECL_CAST(struct typeconv_callback_param *, param, payload);

    REQUIRE(param->temporary->valid,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated temporary for cast operator long double parameter"));
    REQUIRE_OK(kefir_ast_translator_fetch_temporary(param->mem, param->context, param->builder, param->temporary));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_cast_operator_node(struct kefir_mem *mem,
                                                      struct kefir_ast_translator_context *context,
                                                      struct kefir_irbuilder_block *builder,
                                                      const struct kefir_ast_cast_operator *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST cast operator node"));

    const struct kefir_ast_type *expr_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->expr->properties.type);

    const struct kefir_ast_type *arg_init_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);
    const struct kefir_ast_type *arg_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, arg_init_normalized_type);
    REQUIRE(arg_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->expr, builder, context));

    struct typeconv_callback_param cb_param = {.mem = mem,
                                               .context = context,
                                               .builder = builder,
                                               .temporary = &node->base.properties.expression_props.temporary};
    struct kefir_ast_translate_typeconv_callbacks callbacks = {.allocate_long_double = allocate_long_double_callback,
                                                               .payload = &cb_param};
    REQUIRE_OK(kefir_ast_translate_typeconv(builder, context->ast_context->type_traits, expr_type, arg_normalized_type,
                                            &callbacks));
    return KEFIR_OK;
}
