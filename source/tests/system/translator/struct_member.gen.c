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

#include <stdlib.h>
#include <stdio.h>
#include "kefir/ir/function.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/module.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/scope/global_scope_layout.h"
#include "kefir/ast-translator/scope/local_scope_layout.h"
#include "kefir/ast/context_manager.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast-translator/context.h"
#include "kefir/ast-translator/scope/translator.h"
#include "codegen.h"
#include "kefir/test/util.h"

#include "codegen.inc.c"

static kefir_result_t define_sum_function(struct kefir_mem *mem, struct function *func,
                                          struct kefir_ast_context_manager *context_manager, const char *name,
                                          const struct kefir_ast_type *param_type) {
    func->identifier = name;
    REQUIRE_OK(kefir_list_init(&func->args));

    struct kefir_ast_function_type *func_type = NULL;
    func->type =
        kefir_ast_type_function(mem, context_manager->current->type_bundle, kefir_ast_type_double(), &func_type);
    REQUIRE_OK(
        kefir_ast_type_function_parameter(mem, context_manager->current->type_bundle, func_type, param_type, NULL));

    REQUIRE_OK(kefir_ast_global_context_define_function(mem, context_manager->global, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                        true, func->identifier, func->type, NULL, NULL, NULL));

    REQUIRE_OK(kefir_ast_local_context_init(mem, context_manager->global, &func->local_context));
    REQUIRE_OK(kefir_ast_context_manager_attach_local(&func->local_context, context_manager));

    REQUIRE_OK(kefir_ast_local_context_define_auto(mem, context_manager->local, "param", param_type, NULL, NULL, NULL,
                                                   NULL, NULL));

    REQUIRE_OK(kefir_list_insert_after(
        mem, &func->args, kefir_list_tail(&func->args),
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param"))));

    struct kefir_ast_node_base *add3 = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_ADD,
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
            mem, context_manager->current->symbols,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "ptr")),
            "field3")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
            mem, context_manager->current->symbols,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "ptr")),
            "field4"))));

    struct kefir_ast_node_base *add2 = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_ADD,
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
            mem, context_manager->current->symbols,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "ptr")),
            "field2")),
        add3));

    struct kefir_ast_node_base *add1 = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_ADD,
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
            mem, context_manager->current->symbols,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "ptr")),
            "field1")),
        add2));

    func->body = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_ADD,
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
            mem, context_manager->current->symbols,
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "arg")),
        add1));

    REQUIRE_OK(kefir_ast_context_manager_detach_local(context_manager));
    return KEFIR_OK;
}

static kefir_result_t generate_ir(struct kefir_mem *mem, struct kefir_ir_module *module,
                                  struct kefir_ir_target_platform *ir_platform) {
    struct kefir_ast_translator_environment env;
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, ir_platform));

    struct kefir_ast_context_manager context_manager;
    struct kefir_ast_global_context global_context;
    REQUIRE_OK(
        kefir_ast_global_context_init(mem, kefir_util_default_type_traits(), &env.target_env, &global_context, NULL));
    REQUIRE_OK(kefir_ast_context_manager_init(&global_context, &context_manager));

    struct kefir_ast_struct_type *struct_type1 = NULL;
    const struct kefir_ast_type *type1 =
        kefir_ast_type_structure(mem, context_manager.current->type_bundle, NULL, &struct_type1);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type1, "field1",
                                           kefir_ast_type_signed_int(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type1, "field2",
                                           kefir_ast_type_unsigned_long(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type1, "field3",
                                           kefir_ast_type_boolean(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type1, "field4",
                                           kefir_ast_type_float(), NULL));

    struct kefir_ast_struct_type *struct_type2 = NULL;
    const struct kefir_ast_type *type2 =
        kefir_ast_type_structure(mem, context_manager.current->type_bundle, NULL, &struct_type2);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type2, "ptr",
                                           kefir_ast_type_pointer(mem, context_manager.current->type_bundle, type1),
                                           NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type2, "arg",
                                           kefir_ast_type_double(), NULL));

    struct function sum;
    REQUIRE_OK(define_sum_function(mem, &sum, &context_manager, "sum", type2));

    REQUIRE_OK(analyze_function(mem, &sum, &context_manager));

    struct kefir_ast_translator_context translator_context;
    REQUIRE_OK(
        kefir_ast_translator_context_init(mem, &translator_context, &global_context.context, &env, module, NULL));

    struct kefir_ast_translator_global_scope_layout global_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, module, &global_scope));
    REQUIRE_OK(kefir_ast_translator_build_global_scope_layout(mem, module, &global_context,
                                                              translator_context.environment, &global_scope));

    REQUIRE_OK(translate_function(mem, &sum, &context_manager, &global_scope, &translator_context));

    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, module, &global_scope));

    REQUIRE_OK(free_function(mem, &sum));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &global_scope));
    REQUIRE_OK(kefir_ast_translator_context_free(mem, &translator_context));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    GENCODE(generate_ir);
    return EXIT_SUCCESS;
}
