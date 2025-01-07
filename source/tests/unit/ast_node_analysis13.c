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

#include <string.h>
#include "kefir/test/unit_test.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/function_declaration_context.h"
#include "kefir/ast/type_conv.h"
#include "kefir/test/util.h"

DEFINE_CASE(ast_node_analysis_inline_assembly1, "AST node analysis - inline assembly #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));

    struct kefir_ast_inline_assembly *inline_asm1 = kefir_ast_new_inline_assembly(
        &kft_mem, (struct kefir_ast_inline_assembly_qualifiers){.goto_qualifier = true}, "Some assembly code");

    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(inline_asm1)));
    ASSERT(inline_asm1->base.properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY);
    ASSERT(inline_asm1->base.properties.inline_assembly.origin_flow_control_point == NULL);
    ASSERT(inline_asm1->base.properties.inline_assembly.branching_point == NULL);

    struct kefir_ast_inline_assembly *inline_asm2 = kefir_ast_new_inline_assembly(
        &kft_mem, (struct kefir_ast_inline_assembly_qualifiers){.inline_qualifier = true}, "Some other assembly code");
    ASSERT_OK(kefir_ast_inline_assembly_add_output(
        &kft_mem, &global_context.symbols, inline_asm2, NULL, "constraint",
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, &global_context.symbols, "abc"))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(inline_asm2)));

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(inline_asm1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(inline_asm2)));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_inline_assembly2, "AST node analysis - inline assembly #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));

    struct kefir_ast_flow_control_structure_associated_scopes associated_scopes;
    ASSERT_OK(local_context.context.push_block(&kft_mem, &local_context.context, &associated_scopes.ordinary_scope,
                                               &associated_scopes.tag_scope));

    struct kefir_ast_flow_control_structure *flow_control = NULL;
    ASSERT_OK(kefir_ast_flow_control_tree_push(&kft_mem, &local_context.flow_control_tree,
                                               KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK, &associated_scopes,
                                               &flow_control));

    struct kefir_ast_inline_assembly *inline_asm1 = kefir_ast_new_inline_assembly(
        &kft_mem, (struct kefir_ast_inline_assembly_qualifiers){.goto_qualifier = true}, "Some assembly code");

    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm1)));
    ASSERT(inline_asm1->base.properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY);
    ASSERT(inline_asm1->base.properties.inline_assembly.origin_flow_control_point != NULL);
    ASSERT(inline_asm1->base.properties.inline_assembly.origin_flow_control_point->parent == flow_control);
    ASSERT(inline_asm1->base.properties.inline_assembly.branching_point != NULL);

    ASSERT_OK(local_context.context.define_identifier(
        &kft_mem, &local_context.context, true, "a1", kefir_ast_type_signed_int(),
        KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO, KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));
    ASSERT_OK(local_context.context.define_identifier(
        &kft_mem, &local_context.context, true, "a2", kefir_ast_type_signed_int(),
        KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO, KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));

    struct kefir_ast_inline_assembly *inline_asm2 = kefir_ast_new_inline_assembly(
        &kft_mem, (struct kefir_ast_inline_assembly_qualifiers){.inline_qualifier = true}, "Some other assembly code");
    ASSERT_OK(kefir_ast_inline_assembly_add_output(
        &kft_mem, &global_context.symbols, inline_asm2, "output1", "constraint1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, &global_context.symbols, "a1"))));
    ASSERT_OK(kefir_ast_inline_assembly_add_output(
        &kft_mem, &global_context.symbols, inline_asm2, NULL, "constraint1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, &global_context.symbols, "a2"))));
    ASSERT_OK(kefir_ast_inline_assembly_add_input(
        &kft_mem, &global_context.symbols, inline_asm2, NULL, "constraint3",
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, &global_context.symbols, "a1"))));
    ASSERT_OK(kefir_ast_inline_assembly_add_input(&kft_mem, &global_context.symbols, inline_asm2, NULL, "constraint3",
                                                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1020))));
    ASSERT_OK(kefir_ast_inline_assembly_add_clobber(&kft_mem, &global_context.symbols, inline_asm2, "mem"));
    ASSERT_OK(kefir_ast_inline_assembly_add_clobber(&kft_mem, &global_context.symbols, inline_asm2, "cc"));
    ASSERT_OK(kefir_ast_inline_assembly_add_jump_label(&kft_mem, &global_context.symbols, inline_asm2, "label1"));
    ASSERT_OK(kefir_ast_inline_assembly_add_jump_label(&kft_mem, &global_context.symbols, inline_asm2, "label2"));
    ASSERT_OK(kefir_ast_inline_assembly_add_jump_label(&kft_mem, &global_context.symbols, inline_asm2, "label3"));

    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm2)));
    ASSERT(inline_asm2->base.properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY);
    ASSERT(inline_asm2->base.properties.inline_assembly.origin_flow_control_point != NULL);
    ASSERT(inline_asm2->base.properties.inline_assembly.origin_flow_control_point->parent == flow_control);
    ASSERT(inline_asm2->base.properties.inline_assembly.branching_point != NULL);

    struct kefir_hashtree_node *label_node;
    ASSERT_OK(kefir_hashtree_at(&inline_asm2->base.properties.inline_assembly.branching_point->branches,
                                (kefir_hashtree_key_t) "label1", &label_node));
    const struct kefir_ast_scoped_identifier *scoped_id;
    ASSERT_OK(
        local_context.context.reference_label(&kft_mem, &local_context.context, "label1", NULL, NULL, &scoped_id));
    ASSERT(scoped_id->label.point == (void *) label_node->value);

    ASSERT_OK(kefir_hashtree_at(&inline_asm2->base.properties.inline_assembly.branching_point->branches,
                                (kefir_hashtree_key_t) "label2", &label_node));
    ASSERT_OK(
        local_context.context.reference_label(&kft_mem, &local_context.context, "label2", NULL, NULL, &scoped_id));
    ASSERT(scoped_id->label.point == (void *) label_node->value);

    ASSERT_OK(kefir_hashtree_at(&inline_asm2->base.properties.inline_assembly.branching_point->branches,
                                (kefir_hashtree_key_t) "label3", &label_node));
    ASSERT_OK(
        local_context.context.reference_label(&kft_mem, &local_context.context, "label3", NULL, NULL, &scoped_id));
    ASSERT(scoped_id->label.point == (void *) label_node->value);

    struct kefir_ast_inline_assembly *inline_asm3 = kefir_ast_new_inline_assembly(
        &kft_mem, (struct kefir_ast_inline_assembly_qualifiers){.inline_qualifier = true}, "Some other assembly code");
    ASSERT_OK(kefir_ast_inline_assembly_add_output(
        &kft_mem, &global_context.symbols, inline_asm3, "output1", "constraint1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, &global_context.symbols, "a4"))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm3)));

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(inline_asm1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(inline_asm2)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(inline_asm3)));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE
