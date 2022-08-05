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

#include "kefir/core/mem.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/scope/translator.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast-translator/context.h"
#include "kefir/ast-translator/scope/local_scope_layout.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/test/util.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/format.h"
#include "./expression.h"
#include <stdio.h>

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_ast_translator_environment env;
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_global_context global_context;
    REQUIRE_OK(
        kefir_ast_global_context_init(mem, kefir_util_default_type_traits(), &env.target_env, &global_context, NULL));
    REQUIRE_OK(kefir_ast_global_context_declare_external(mem, &global_context, "variable", kefir_ast_type_signed_int(),
                                                         NULL, NULL, NULL));

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    struct kefir_ast_translator_global_scope_layout translator_global_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, &module, &translator_global_scope));

    struct kefir_ast_translator_context global_translator_context;
    REQUIRE_OK(kefir_ast_translator_context_init(mem, &global_translator_context, &global_context.context, &env,
                                                 &module, NULL));
    REQUIRE_OK(
        kefir_ast_translator_build_global_scope_layout(mem, &module, &global_context, &env, &translator_global_scope));
    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, &module, &translator_global_scope));
    struct kefir_irbuilder_block builder;

    REQUIRE_OK(global_context.context.define_identifier(
        mem, &global_context.context, true, "abc", kefir_ast_type_signed_int(),
        KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN, KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));
    REQUIRE_OK(global_context.context.define_identifier(
        mem, &global_context.context, true, "lde", kefir_ast_type_double(), KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN,
        KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));

    struct kefir_ast_inline_assembly *inline_asm0 = kefir_ast_new_inline_assembly(
        mem, (struct kefir_ast_inline_assembly_qualifiers){0}, "inline assembly code here.\ntest...test...test...");
    struct kefir_ast_inline_assembly *inline_asm1 =
        kefir_ast_new_inline_assembly(mem, (struct kefir_ast_inline_assembly_qualifiers){.inline_qualifier = true},
                                      "some assembly code here.\n"
                                      "no outputs or inputs are permitted in global context");
    REQUIRE_OK(kefir_ast_analyze_node(mem, &global_context.context, KEFIR_AST_NODE_BASE(inline_asm0)));
    REQUIRE_OK(kefir_ast_analyze_node(mem, &global_context.context, KEFIR_AST_NODE_BASE(inline_asm1)));
    REQUIRE_OK(
        kefir_ast_translate_inline_assembly(mem, KEFIR_AST_NODE_BASE(inline_asm0), NULL, &global_translator_context));
    REQUIRE_OK(
        kefir_ast_translate_inline_assembly(mem, KEFIR_AST_NODE_BASE(inline_asm1), NULL, &global_translator_context));

    FUNC2("inline_asm1", {
        REQUIRE_OK(local_context.context.define_identifier(
            mem, &local_context.context, true, "ld1", kefir_ast_type_long_double(),
            KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO, KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));

        struct kefir_ast_inline_assembly *inline_asm2 =
            kefir_ast_new_inline_assembly(mem, (struct kefir_ast_inline_assembly_qualifiers){0},
                                          "some assembly code here.\n"
                                          "register %0 is an output\n"
                                          "register %1 is an input\n"
                                          "rax is clobber");
        REQUIRE_OK(kefir_ast_inline_assembly_add_output(
            mem, &global_context.symbols, inline_asm2, NULL, "=rm",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "abc"))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_output(
            mem, &global_context.symbols, inline_asm2, NULL, "+rm",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "ld1"))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_input(
            mem, &global_context.symbols, inline_asm2, NULL, "m",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "abc"))));
        REQUIRE_OK(
            kefir_ast_inline_assembly_add_input(mem, &global_context.symbols, inline_asm2, NULL, "m",
                                                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long_double(mem, 4.15L))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_clobber(mem, &global_context.symbols, inline_asm2, "rax"));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_analyze_node(mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm2)));
        REQUIRE_OK(kefir_ast_translate_inline_assembly(mem, KEFIR_AST_NODE_BASE(inline_asm2), &builder,
                                                       &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(inline_asm2)));
    });

    FUNC2("inline_asm2", {
        REQUIRE_OK(local_context.context.define_identifier(
            mem, &local_context.context, true, "chr", kefir_ast_type_char(), KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO,
            KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));
        REQUIRE_OK(local_context.context.define_identifier(
            mem, &local_context.context, true, "chr2", kefir_ast_type_char(), KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO,
            KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL, NULL));

        struct kefir_ast_inline_assembly *inline_asm2 =
            kefir_ast_new_inline_assembly(mem, (struct kefir_ast_inline_assembly_qualifiers){0},
                                          "some assembly code here.\n"
                                          "register %0 is an output\n"
                                          "register %1 is an input\n"
                                          "rax, rbx, rcx are clobbers");
        REQUIRE_OK(kefir_ast_inline_assembly_add_output(
            mem, &global_context.symbols, inline_asm2, "null", "=r",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "abc"))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_output(
            mem, &global_context.symbols, inline_asm2, NULL, "+m",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "chr"))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_output(
            mem, &global_context.symbols, inline_asm2, "alternativeId", "=r",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "chr2"))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_input(
            mem, &global_context.symbols, inline_asm2, NULL, "r",
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, &global_context.symbols, "lde"))));
        REQUIRE_OK(
            kefir_ast_inline_assembly_add_input(mem, &global_context.symbols, inline_asm2, NULL, "m",
                                                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long_double(mem, 4.15L))));
        REQUIRE_OK(kefir_ast_inline_assembly_add_clobber(mem, &global_context.symbols, inline_asm2, "rax"));
        REQUIRE_OK(kefir_ast_inline_assembly_add_clobber(mem, &global_context.symbols, inline_asm2, "rbx"));
        REQUIRE_OK(kefir_ast_inline_assembly_add_clobber(mem, &global_context.symbols, inline_asm2, "rcx"));

        struct kefir_ast_inline_assembly *inline_asm3 = kefir_ast_new_inline_assembly(
            mem, (struct kefir_ast_inline_assembly_qualifiers){.volatile_qualifier = true}, "another assembly");

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_analyze_node(mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm2)));
        REQUIRE_OK(kefir_ast_analyze_node(mem, &local_context.context, KEFIR_AST_NODE_BASE(inline_asm3)));
        REQUIRE_OK(kefir_ast_translate_inline_assembly(mem, KEFIR_AST_NODE_BASE(inline_asm2), &builder,
                                                       &local_translator_context));
        REQUIRE_OK(kefir_ast_translate_inline_assembly(mem, KEFIR_AST_NODE_BASE(inline_asm3), &builder,
                                                       &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(inline_asm2)));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(inline_asm3)));
    });

    REQUIRE_OK(kefir_ir_format_module(stdout, &module));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(inline_asm0)));
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(inline_asm1)));
    REQUIRE_OK(kefir_ast_translator_context_free(mem, &global_translator_context));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &translator_global_scope));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}
