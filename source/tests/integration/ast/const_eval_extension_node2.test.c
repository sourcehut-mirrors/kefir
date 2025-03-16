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

#include "kefir/core/basic-types.h"
#include "kefir/ir/module.h"
#include "kefir/core/mem.h"
#include "kefir/ir/builder.h"
#include "kefir/ast/node.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/constant_expression.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/environment.h"
#include "kefir/ast-translator/scope/translator.h"
#include "kefir/ir/format.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/local_context.h"
#include "kefir/test/util.h"
#include "kefir/test/module_shim.h"

static kefir_result_t analyze_extension_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             struct kefir_ast_node_base *node) {
    UNUSED(mem);
    UNUSED(context);
    node->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    node->properties.type = kefir_ast_type_signed_int();
    node->properties.expression_props.addressable = true;
    return KEFIR_OK;
}

static kefir_result_t evaluate_pointer_extension_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                      const struct kefir_ast_node_base *node,
                                                      struct kefir_ast_constant_expression_pointer *pointer) {
    UNUSED(mem);
    UNUSED(node);
    UNUSED(context);

    struct kefir_ast_extension_node *ext_node;
    REQUIRE_OK(kefir_ast_downcast_extension_node(node, &ext_node, false));

    pointer->type = KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER;
    pointer->base.literal = "HELLO";
    pointer->offset = (kefir_uptr_t) ext_node->payload;
    pointer->pointer_node = node;
    pointer->scoped_id = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));
    struct kefir_ast_translator_environment env;
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_context_extensions analysis_ext = {
        .analyze_extension_node = analyze_extension_node,
        .evaluate_constant_pointer_extension_node = evaluate_pointer_extension_node};

    struct kefir_ast_global_context global_context;
    REQUIRE_OK(kefir_ast_global_context_init(mem, kefir_util_default_type_traits(),
                                             &kft_util_get_translator_environment()->target_env, &global_context,
                                             &analysis_ext));

    struct kefir_ast_extension_node_class ext_node_class = {0};

    struct kefir_ast_initializer *init1 = kefir_ast_new_expression_initializer(
        mem,
        KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
            mem, KEFIR_AST_OPERATION_ADDRESS,
            KEFIR_AST_NODE_BASE(kefir_ast_new_extension_node(mem, &ext_node_class, (void *) (kefir_uptr_t) 100)))));
    struct kefir_ast_initializer *init2 = kefir_ast_new_expression_initializer(
        mem,
        KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
            mem, KEFIR_AST_OPERATION_ADDRESS,
            KEFIR_AST_NODE_BASE(kefir_ast_new_extension_node(mem, &ext_node_class, (void *) (kefir_uptr_t) 200)))));
    REQUIRE_OK(kefir_ast_global_context_define_external(
        mem, &global_context, "x1",
        kefir_ast_type_pointer(mem, global_context.context.type_bundle, kefir_ast_type_signed_int()), NULL, init1, NULL,
        NULL, NULL));
    REQUIRE_OK(kefir_ast_global_context_define_external(
        mem, &global_context, "x2",
        kefir_ast_type_pointer(mem, global_context.context.type_bundle, kefir_ast_type_signed_int()), NULL, init2, NULL,
        NULL, NULL));

    struct kefir_ast_translator_global_scope_layout translator_global_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, &module, &translator_global_scope));

    struct kefir_ast_translator_context translator_context;
    REQUIRE_OK(
        kefir_ast_translator_context_init(mem, &translator_context, &global_context.context, &env, &module, NULL));

    REQUIRE_OK(kefir_ast_translator_build_global_scope_layout(
        mem, &module, &global_context, &env, translator_context.debug_entries, &translator_global_scope));
    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, &module, &translator_global_scope));

    REQUIRE_OK(kefir_ast_translator_context_free(mem, &translator_context));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &translator_global_scope));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    REQUIRE_OK(kefir_ast_initializer_free(mem, init1));
    REQUIRE_OK(kefir_ast_initializer_free(mem, init2));
    REQUIRE_OK(kefir_ir_format_module(stdout, &module, false));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}
