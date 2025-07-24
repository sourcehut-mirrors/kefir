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

#include "kefir/ast/declarator.h"
#include "kefir/ast/node.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"
#include <string.h>

DEFINE_CASE(ast_attribute_declaration1, "AST declarators - attribute declaration #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));

    struct kefir_ast_attribute_declaration *decl = kefir_ast_new_attribute_declaration(&kft_mem);
    ASSERT(decl != NULL);
    ASSERT(decl->base.klass->type == KEFIR_AST_ATTRIBUTE_DECLARATION);
    ASSERT(decl->base.self == decl);

    struct kefir_ast_attribute_list *attr_list = kefir_ast_new_attribute_list(&kft_mem);
    struct kefir_ast_attribute *attr;
    ASSERT_OK(
        kefir_ast_attribute_list_append(&kft_mem, global_context.context.symbols, "test", "test2", attr_list, &attr));
    ASSERT_OK(
        kefir_ast_attribute_list_append(&kft_mem, global_context.context.symbols, "test2", "test3", attr_list, &attr));
    ASSERT_OK(kefir_ast_node_attributes_append(&kft_mem, &decl->attributes, attr_list));
    ASSERT(kefir_list_length(&decl->attributes.attributes) == 1);
    ASSERT(((struct kefir_ast_attribute_list *) kefir_list_head(&decl->attributes.attributes)->value) == attr_list);

    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(decl)));
    ASSERT(decl->base.properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(decl)));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE