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

#include "kefir/parser/rule_helpers.h"
#include "kefir/ast/downcast.h"

static kefir_result_t scan_specifiers(struct kefir_mem *mem, struct kefir_parser *parser,
                                      struct kefir_ast_declarator_specifier_list *specifiers) {
    REQUIRE_OK(kefir_ast_declarator_specifier_list_init(specifiers));
    kefir_result_t res = parser->ruleset.declaration_specifier_list(mem, parser, specifiers);
    if (res == KEFIR_NO_MATCH && parser->configuration->implicit_function_definition_int) {
        res = KEFIR_OK;
        struct kefir_ast_declarator_specifier *specifier = kefir_ast_type_specifier_int(mem);
        REQUIRE_CHAIN_SET(&res, specifier != NULL,
                          KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate AST int type specifier"));
        REQUIRE_CHAIN(&res, kefir_ast_declarator_specifier_list_append(mem, specifiers, specifier));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_declarator_specifier_free(mem, specifier);
            kefir_ast_declarator_specifier_list_free(mem, specifiers);
            return res;
        });
    } else {
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_declarator_specifier_list_free(mem, specifiers);
            return res;
        });
    }
    return KEFIR_OK;
}

static kefir_result_t free_declaration(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                       void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, entry->value);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    return KEFIR_OK;
}

static kefir_result_t scan_declaration_list(struct kefir_mem *mem, struct kefir_parser *parser,
                                            struct kefir_list *declaration_list) {
    REQUIRE_OK(kefir_list_init(declaration_list));
    REQUIRE_OK(kefir_list_on_remove(declaration_list, free_declaration, NULL));

    kefir_result_t res = KEFIR_OK;
    kefir_bool_t scan_declarations = true;
    while (scan_declarations) {
        struct kefir_ast_node_base *declaration = NULL;
        res = KEFIR_PARSER_RULE_APPLY(mem, parser, declaration, &declaration);
        if (res == KEFIR_OK) {
            res = kefir_list_insert_after(mem, declaration_list, kefir_list_tail(declaration_list), declaration);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_AST_NODE_FREE(mem, declaration);
                scan_declarations = false;
            });
        } else {
            scan_declarations = false;
        }
    }

    REQUIRE_ELSE(res == KEFIR_NO_MATCH, {
        kefir_list_free(mem, declaration_list);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t update_function_parameter_scope(struct kefir_mem *mem, struct kefir_parser *parser,
                                                      struct kefir_ast_declarator *declarator) {
    const struct kefir_ast_declarator_function *decl_func = NULL;
    REQUIRE_OK(kefir_ast_declarator_unpack_function(declarator, &decl_func));
    REQUIRE(decl_func != NULL, KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Expected function declarator"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&decl_func->parameters); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, iter->value);

        struct kefir_ast_declaration *param_decl = NULL;
        kefir_result_t res = kefir_ast_downcast_declaration(param, &param_decl, false);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_parser_update_scope_with_declaration(mem, parser, param_decl));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_components(struct kefir_mem *mem, struct kefir_parser *parser,
                                      struct kefir_ast_declarator_specifier_list *specifiers,
                                      struct kefir_ast_declarator **declarator, struct kefir_list *declaration_list,
                                      struct kefir_ast_node_base **compound_statement,
                                      struct kefir_ast_node_attributes *attributes) {
    REQUIRE_OK(scan_specifiers(mem, parser, specifiers));
    kefir_result_t res = parser->ruleset.declarator(mem, parser, declarator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    res = kefir_ast_node_attributes_move(&(*declarator)->attributes, attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    res = kefir_parser_scope_push_block(mem, parser->scope);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_declarator_free(mem, *declarator);
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    res = update_function_parameter_scope(mem, parser, *declarator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_parser_scope_pop_block(mem, parser->scope);
        kefir_ast_declarator_free(mem, *declarator);
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    res = scan_declaration_list(mem, parser, declaration_list);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_parser_scope_pop_block(mem, parser->scope);
        kefir_ast_declarator_free(mem, *declarator);
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    res = KEFIR_PARSER_RULE_APPLY(mem, parser, compound_statement, compound_statement);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_parser_scope_pop_block(mem, parser->scope);
        kefir_list_free(mem, declaration_list);
        kefir_ast_declarator_free(mem, *declarator);
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    REQUIRE_CHAIN(&res, kefir_parser_scope_pop_block(mem, parser->scope));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, declaration_list);
        kefir_ast_declarator_free(mem, *declarator);
        kefir_ast_declarator_specifier_list_free(mem, specifiers);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(function_definition)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                                struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    struct kefir_ast_declarator_specifier_list specifiers;
    struct kefir_ast_declarator *declarator = NULL;
    struct kefir_list declaration_list;
    struct kefir_ast_node_base *compound_statement_node = NULL;

    struct kefir_ast_pragma_state pragmas;
    REQUIRE_OK(kefir_parser_pragmas_collect(&pragmas, &parser->pragmas));

    kefir_result_t res = KEFIR_OK;
    struct kefir_ast_node_attributes attributes;
    REQUIRE_OK(kefir_ast_node_attributes_init(&attributes));
    SCAN_ATTRIBUTES(&res, mem, parser, &attributes);
    REQUIRE_CHAIN(&res, scan_components(mem, parser, &specifiers, &declarator, &declaration_list,
                                        &compound_statement_node, &attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_node_attributes_free(mem, &attributes);
        return res;
    });
    res = kefir_ast_node_attributes_free(mem, &attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, compound_statement_node);
        kefir_list_free(mem, &declaration_list);
        kefir_ast_declarator_free(mem, declarator);
        kefir_ast_declarator_specifier_list_free(mem, &specifiers);
        return res;
    });

    struct kefir_ast_compound_statement *compound_statement = NULL;
    REQUIRE_MATCH_OK(
        &res, kefir_ast_downcast_compound_statement(compound_statement_node, &compound_statement, true),
        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected compound statement rule to produce a compound statement"));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, compound_statement_node);
        kefir_list_free(mem, &declaration_list);
        kefir_ast_declarator_free(mem, declarator);
        kefir_ast_declarator_specifier_list_free(mem, &specifiers);
        return res;
    });

    struct kefir_ast_function_definition *func_definition =
        kefir_ast_new_function_definition(mem, declarator, compound_statement);
    REQUIRE_ELSE(func_definition != NULL, {
        KEFIR_AST_NODE_FREE(mem, compound_statement_node);
        kefir_list_free(mem, &declaration_list);
        kefir_ast_declarator_free(mem, declarator);
        kefir_ast_declarator_specifier_list_free(mem, &specifiers);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST function definition");
    });

    func_definition->pragmas = pragmas;

    res = kefir_ast_declarator_specifier_list_clone(mem, &func_definition->specifiers, &specifiers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(func_definition));
        kefir_list_free(mem, &declaration_list);
        kefir_ast_declarator_specifier_list_free(mem, &specifiers);
        return res;
    });

    res = kefir_ast_declarator_specifier_list_free(mem, &specifiers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(func_definition));
        kefir_list_free(mem, &declaration_list);
        return res;
    });

    res = kefir_list_move_all(&func_definition->declarations, &declaration_list);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(func_definition));
        kefir_list_free(mem, &declaration_list);
        return res;
    });

    res = kefir_list_free(mem, &declaration_list);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(func_definition));
        return res;
    });

    *result = KEFIR_AST_NODE_BASE(func_definition);
    return KEFIR_OK;
}
