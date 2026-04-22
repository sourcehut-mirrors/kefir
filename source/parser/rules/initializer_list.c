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

#include "kefir/parser/rule_helpers.h"
#include "kefir/core/source_error.h"

static kefir_result_t scan_initializer(struct kefir_mem *mem, struct kefir_parser *parser,
                                       struct kefir_ast_initializer *initializer) {
    struct kefir_ast_initializer_designation *designation = NULL;
    struct kefir_ast_initializer *subinitializer = NULL;

    kefir_result_t res = parser->ruleset.initializer_designation(mem, parser, &designation);
    REQUIRE(res == KEFIR_OK || res == KEFIR_NO_MATCH, res);
    res = parser->ruleset.initializer(mem, parser, &subinitializer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (designation != NULL) {
            kefir_ast_initializer_designation_free(mem, designation);
        }
        return res;
    });
    res = kefir_ast_initializer_list_append(mem, &initializer->list, designation, subinitializer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_initializer_free(mem, subinitializer);
        if (designation != NULL) {
            kefir_ast_initializer_designation_free(mem, designation);
        }
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t scan_initializer_list(struct kefir_mem *mem, struct kefir_parser *parser, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct kefir_ast_initializer **, initializer, payload);
    REQUIRE(PARSER_TOKEN_IS_LEFT_BRACE(parser, 0), KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Cannot match initializer list"));
    REQUIRE_OK(PARSER_SHIFT(parser));
    *initializer = kefir_ast_new_list_initializer(mem);
    REQUIRE(initializer != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocae AST initializer list"));
    (*initializer)->source_location = *PARSER_TOKEN_LOCATION(parser, 0);

    kefir_bool_t has_syntax_errors = false;
    kefir_bool_t scan_initializers = !PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0);
    while (scan_initializers) {
        struct kefir_parser_checkpoint checkpoint;
        kefir_result_t res = kefir_parser_checkpoint_save(parser, &checkpoint);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_initializer_free(mem, *initializer);
            return res;
        });

        res = scan_initializer(mem, parser, *initializer);
        if (res == KEFIR_SYNTAX_ERROR && KEFIR_PARSER_DO_ERROR_RECOVERY(parser)) {
            has_syntax_errors = true;
            parser->encountered_errors++;
            res = kefir_parser_checkpoint_restore(parser, &checkpoint);
            REQUIRE_CHAIN(
                &res, kefir_parser_error_recovery_skip_garbage(
                          parser, &(struct kefir_parser_error_recovery_context) {
                                      .sync_points = {
                                          .semicolon = true, .comma = true, .parentheses = true, .brackets = true}}));
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_initializer_free(mem, *initializer);
                return res;
            });
        } else {
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_initializer_free(mem, *initializer);
                return res;
            });
        }
        if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
            res = PARSER_SHIFT(parser);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_initializer_free(mem, *initializer);
                *initializer = NULL;
                return res;
            });
            scan_initializers = !PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0);
        } else {
            scan_initializers = false;
        }
    }

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN_SET(
        &res, PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected right brace"));
    REQUIRE_CHAIN(&res, PARSER_SHIFT(parser));
    REQUIRE_ELSE(res == KEFIR_OK, {
        *initializer = NULL;
        *initializer = NULL;
        return res;
    });
    return has_syntax_errors ? KEFIR_SYNTAX_ERROR : KEFIR_OK;
}

kefir_result_t kefir_parser_scan_initializer_list(struct kefir_mem *mem, struct kefir_parser *parser,
                                                  struct kefir_ast_initializer **initializer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(initializer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST initializer"));

    REQUIRE_OK(kefir_parser_try_invoke_impl(mem, parser, scan_initializer_list, initializer));
    return KEFIR_OK;
}
