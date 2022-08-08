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
#include "kefir/codegen/amd64-sysv.h"
#include "kefir/ir/format.h"
#include "codegen.h"
#include "kefir/test/util.h"

#include "codegen.inc.c"

static kefir_result_t define_modify_function(struct kefir_mem *mem, struct function *func,
                                             struct kefir_ast_context_manager *context_manager, const char *name,
                                             const struct kefir_ast_type *param_type) {
    func->identifier = name;
    REQUIRE_OK(kefir_list_init(&func->args));

    struct kefir_ast_function_type *func_type = NULL;
    func->type =
        kefir_ast_type_function(mem, context_manager->current->type_bundle, kefir_ast_type_signed_int(), &func_type);
    REQUIRE_OK(
        kefir_ast_type_function_parameter(mem, context_manager->current->type_bundle, func_type, param_type, NULL));
    REQUIRE_OK(kefir_ast_type_function_parameter(mem, context_manager->current->type_bundle, func_type,
                                                 kefir_ast_type_unsigned_long(), NULL));

    REQUIRE_OK(kefir_ast_global_context_define_function(mem, context_manager->global, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                        true, func->identifier, func->type, NULL, NULL, NULL));

    REQUIRE_OK(kefir_ast_local_context_init(mem, context_manager->global, &func->local_context));
    REQUIRE_OK(kefir_ast_context_manager_attach_local(&func->local_context, context_manager));

    REQUIRE_OK(kefir_ast_local_context_define_auto(mem, context_manager->local, "param", param_type, NULL, NULL, NULL,
                                                   NULL, NULL));
    REQUIRE_OK(kefir_ast_local_context_define_auto(mem, context_manager->local, "value", kefir_ast_type_signed_long(),
                                                   NULL, NULL, NULL, NULL, NULL));

    REQUIRE_OK(kefir_list_insert_after(
        mem, &func->args, kefir_list_tail(&func->args),
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param"))));
    REQUIRE_OK(kefir_list_insert_after(
        mem, &func->args, kefir_list_tail(&func->args),
        KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))));

    struct kefir_ast_comma_operator *comma = kefir_ast_new_comma_operator(mem);

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "a")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "b")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "c")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "d")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "e")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "f")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "g")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "h")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    REQUIRE_OK(kefir_ast_comma_append(
        mem, comma,
        KEFIR_AST_NODE_BASE(kefir_ast_new_compound_assignment(
            mem, KEFIR_AST_ASSIGNMENT_MODULO,
            KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
                mem, context_manager->current->symbols,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "param")), "i")),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context_manager->current->symbols, "value"))))));

    func->body = KEFIR_AST_NODE_BASE(comma);

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
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "a",
                                              kefir_ast_type_signed_long(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 25)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "b",
                                              kefir_ast_type_signed_int(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 21)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "c",
                                              kefir_ast_type_signed_short(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 15)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "d",
                                              kefir_ast_type_signed_char(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 7)));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context_manager.current->symbols, struct_type1, "e",
                                           kefir_ast_type_signed_int(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, NULL,
                                              kefir_ast_type_signed_char(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 6)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "f",
                                              kefir_ast_type_signed_long(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 40)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "g",
                                              kefir_ast_type_signed_int(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 30)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "h",
                                              kefir_ast_type_signed_char(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 8)));
    REQUIRE_OK(kefir_ast_struct_type_bitfield(mem, context_manager.current->symbols, struct_type1, "i",
                                              kefir_ast_type_signed_short(), NULL,
                                              kefir_ast_constant_expression_integer(mem, 14)));

    struct function assign;
    REQUIRE_OK(define_modify_function(mem, &assign, &context_manager, "modify",
                                      kefir_ast_type_pointer(mem, context_manager.current->type_bundle, type1)));

    REQUIRE_OK(analyze_function(mem, &assign, &context_manager));

    struct kefir_ast_translator_context translator_context;
    REQUIRE_OK(
        kefir_ast_translator_context_init(mem, &translator_context, &global_context.context, &env, module, NULL));

    struct kefir_ast_translator_global_scope_layout global_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, module, &global_scope));
    REQUIRE_OK(kefir_ast_translator_build_global_scope_layout(mem, module, &global_context,
                                                              translator_context.environment, &global_scope));

    REQUIRE_OK(translate_function(mem, &assign, &context_manager, &global_scope, &translator_context));

    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, module, &global_scope));

    REQUIRE_OK(free_function(mem, &assign));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &global_scope));
    REQUIRE_OK(kefir_ast_translator_context_free(mem, &translator_context));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    GENCODE(generate_ir);
    return EXIT_SUCCESS;
}
