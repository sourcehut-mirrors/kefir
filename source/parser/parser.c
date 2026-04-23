/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/parser/parser.h"
#include "kefir/parser/rule_helpers.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/extensions.h"

static const struct kefir_parser_configuration DefaultConfiguration = {.fail_on_attributes = false,
                                                                       .implicit_function_definition_int = false,
                                                                       .designated_initializer_colons = false,
                                                                       .label_addressing = false,
                                                                       .statement_expressions = false,
                                                                       .omitted_conditional_operand = false,
                                                                       .fail_on_assembly = false,
                                                                       .switch_case_ranges = false,
                                                                       .designator_subscript_ranges = false,
                                                                       .max_errors = (kefir_uint32_t) -1};

kefir_result_t kefir_parser_configuration_default(struct kefir_parser_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser configuration"));

    *config = DefaultConfiguration;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_init(struct kefir_mem *mem, struct kefir_parser *parser, struct kefir_string_pool *symbols,
                                 struct kefir_parser_token_cursor *cursor,
                                 const struct kefir_parser_extensions *extensions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor"));

    REQUIRE_OK(kefir_parser_ruleset_init(&parser->ruleset));
    REQUIRE_OK(kefir_parser_scope_init(mem, &parser->local_scope, symbols));
    parser->symbols = symbols;
    parser->cursor = cursor;
    parser->scope = &parser->local_scope;
    parser->extensions = extensions;
    parser->extension_payload = NULL;
    parser->configuration = &DefaultConfiguration;
    parser->encountered_errors = 0;

    REQUIRE_OK(kefir_parser_pragmas_init(&parser->pragmas));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, parser, on_init);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_parser_scope_free(mem, &parser->local_scope);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_parser_free(struct kefir_mem *mem, struct kefir_parser *parser) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, parser, on_free);
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_parser_pragmas_free(mem, &parser->pragmas));
    REQUIRE_OK(kefir_parser_scope_free(mem, &parser->local_scope));
    parser->cursor = NULL;
    parser->symbols = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_consume_pack_pragmas(struct kefir_mem *mem, struct kefir_parser *parser) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));

    while (PARSER_TOKEN_IS_PRAGMA(parser, 0)) {
        const struct kefir_token *token = PARSER_CURSOR_EXT(parser, 0, false);
        if (token->pragma->pragma != KEFIR_PRAGMA_TOKEN_PACK_PUSH &&
            token->pragma->pragma != KEFIR_PRAGMA_TOKEN_PACK_POP &&
            token->pragma->pragma != KEFIR_PRAGMA_TOKEN_PACK_VALUE) {
            break;
        }
        kefir_result_t res =
            kefir_parser_scan_pragma(mem, &parser->pragmas, &parser->pragmas.file_scope, token->pragma->pragma,
                                     token->pragma->pragma_param, &token->source_location);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
        }
        PARSER_SHIFT_EXT(parser, false);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_parser_try_invoke(struct kefir_mem *mem, struct kefir_parser *parser,
                                       kefir_parser_invocable_fn_t function, void *payload) {
    return kefir_parser_try_invoke_impl(mem, parser, function, payload);
}

kefir_result_t kefir_parser_apply(struct kefir_mem *mem, struct kefir_parser *parser, struct kefir_ast_node_base **node,
                                  kefir_parser_rule_fn_t rule, void *payload) {
    return kefir_parser_apply_impl(mem, parser, node, rule, payload);
}

kefir_result_t kefir_parser_set_scope(struct kefir_parser *parser, struct kefir_parser_scope *scope) {
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));

    if (scope != NULL) {
        parser->scope = scope;
    } else {
        parser->scope = &parser->local_scope;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_parser_checkpoint_save(const struct kefir_parser *parser,
                                            struct kefir_parser_checkpoint *checkpoint) {
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(checkpoint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser checkpoint"));

    REQUIRE_OK(kefir_parser_token_cursor_save(parser->cursor, &checkpoint->cursor));
    checkpoint->pragmas_file_scope = parser->pragmas.file_scope;
    checkpoint->pragmas_in_function_scope = parser->pragmas.in_function_scope;
    checkpoint->pack_stack_top = parser->pragmas.pack.stack_top;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_checkpoint_restore(struct kefir_parser *parser,
                                               const struct kefir_parser_checkpoint *checkpoint) {
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(checkpoint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser checkpoint"));

    REQUIRE_OK(kefir_parser_token_cursor_restore(parser->cursor, checkpoint->cursor));
    parser->pragmas.file_scope = checkpoint->pragmas_file_scope;
    parser->pragmas.in_function_scope = checkpoint->pragmas_in_function_scope;
    parser->pragmas.pack.stack_top = checkpoint->pack_stack_top;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_error_recovery_skip_garbage(struct kefir_parser *parser,
                                                        const struct kefir_parser_error_recovery_context *context) {
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser error recovery context"));

    kefir_uint64_t open_braces = 0, open_parens = 0, open_brackets = 0;
    kefir_bool_t first_step = true;

    while (!PARSER_TOKEN_IS_SENTINEL(parser, 0)) {
        if (PARSER_TOKEN_IS_LEFT_BRACE(parser, 0)) {
            open_braces++;
        } else if (PARSER_TOKEN_IS_LEFT_BRACKET(parser, 0) && context->sync_points.brackets) {
            open_brackets++;
        } else if (PARSER_TOKEN_IS_PUNCTUATOR_EXT(parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE, false) &&
                   context->sync_points.parentheses) {
            open_parens++;
        } else if (PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0)) {
            if (open_braces > 1) {
                open_braces--;
            } else if (open_braces == 1) {
                PARSER_SHIFT_EXT(parser, false);
                break;
            } else if (open_braces == 0 && !first_step) {
                break;
            }
        } else if (PARSER_TOKEN_IS_RIGHT_BRACKET(parser, 0) && context->sync_points.brackets) {
            if (open_brackets > 1) {
                open_brackets--;
            } else if (open_brackets == 1) {
                PARSER_SHIFT_EXT(parser, false);
                break;
            } else if (open_brackets == 0 && !first_step) {
                break;
            }
        } else if (PARSER_TOKEN_IS_PUNCTUATOR_EXT(parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, false) &&
                   context->sync_points.parentheses) {
            if (open_parens > 1) {
                open_parens--;
            } else if (open_parens == 1) {
                PARSER_SHIFT_EXT(parser, false);
                break;
            } else if (open_parens == 0 && !first_step) {
                break;
            }
        } else if (((PARSER_TOKEN_IS_PUNCTUATOR_EXT(parser, 0, KEFIR_PUNCTUATOR_SEMICOLON, false) &&
                     context->sync_points.semicolon) ||
                    (PARSER_TOKEN_IS_PUNCTUATOR_EXT(parser, 0, KEFIR_PUNCTUATOR_COMMA, false) &&
                     context->sync_points.comma) ||
                    (PARSER_TOKEN_IS_PRAGMA(parser, 0) && context->sync_points.pragmas)) &&
                   open_braces == 0 && (open_brackets == 0 || !context->sync_points.brackets) &&
                   (open_parens == 0 || !context->sync_points.parentheses) && !first_step) {
            break;
        }
        PARSER_SHIFT_EXT(parser, false);
        first_step = false;
    }
    return KEFIR_OK;
}
