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
#include "kefir/parser/builder.h"
#include "kefir/core/source_error.h"

static kefir_result_t match_on_off_pragma_param(kefir_ast_pragma_on_off_value_t *value_ptr,
                                                kefir_pragma_token_parameter_t param,
                                                const struct kefir_source_location *source_location) {
    switch (param) {
        case KEFIR_PRAGMA_TOKEN_PARAM_DEFAULT:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_DEFAULT;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_ON:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_ON;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_OFF:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_OFF;
            break;

        default:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location, "Unexpected on-off pragma parameter");
    }
    return KEFIR_OK;
}

static kefir_result_t match_direction_pragma_param(kefir_ast_pragma_direction_value_t *value_ptr,
                                                   kefir_pragma_token_parameter_t param,
                                                   const struct kefir_source_location *source_location) {
    switch (param) {
        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DOWNWARD:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DOWNWARD;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_TONEAREST:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_TONEAREST;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_TONEARESTFROMZERO:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_TONEARESTFROMZERO;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_TOWARDZERO:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_TOWARDZERO;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_UPWARD:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_UPWARD;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DYNAMIC:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DYNAMIC;
            break;

        default:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location, "Unexpected direction pragma parameter");
    }
    return KEFIR_OK;
}

static kefir_result_t match_dec_direction_pragma_param(kefir_ast_pragma_dec_direction_value_t *value_ptr,
                                                       kefir_pragma_token_parameter_t param,
                                                       const struct kefir_source_location *source_location) {
    switch (param) {
        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_DOWNWARD:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_DOWNWARD;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_TONEAREST:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_TONEAREST;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_TONEARESTFROMZERO:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_TONEARESTFROMZERO;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_TOWARDZERO:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_TOWARDZERO;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_UPWARD:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_UPWARD;
            break;

        case KEFIR_PRAGMA_TOKEN_PARAM_FE_DEC_DYNAMIC:
            *value_ptr = KEFIR_AST_PRAGMA_VALUE_FE_DEC_DYNAMIC;
            break;

        default:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                          "Unexpected decimal direction pragma parameter");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_parser_scan_pragma(struct kefir_ast_pragma_state *state, kefir_pragma_token_type_t type,
                                        kefir_pragma_token_parameter_t param,
                                        const struct kefir_source_location *source_location) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST pragma state"));

    switch (type) {
        case KEFIR_PRAGMA_TOKEN_FP_CONTRACT:
            REQUIRE_OK(match_on_off_pragma_param(&state->fp_contract.value, param, source_location));
            state->fp_contract.present = true;
            break;

        case KEFIR_PRAGMA_TOKEN_FENV_ACCESS:
            REQUIRE_OK(match_on_off_pragma_param(&state->fenv_access.value, param, source_location));
            state->fenv_access.present = true;
            break;

        case KEFIR_PRAGMA_TOKEN_CX_LIMITED_RANGE:
            REQUIRE_OK(match_on_off_pragma_param(&state->cx_limited_range.value, param, source_location));
            state->cx_limited_range.present = true;
            break;

        case KEFIR_PRAGMA_TOKEN_FENV_ROUND:
            REQUIRE_OK(match_direction_pragma_param(&state->fenv_round.value, param, source_location));
            state->fenv_round.present = true;
            break;

        case KEFIR_PRAGMA_TOKEN_FENV_DEC_ROUND:
            REQUIRE_OK(match_dec_direction_pragma_param(&state->fenv_dec_round.value, param, source_location));
            state->fenv_dec_round.present = true;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));

    REQUIRE_OK(kefir_parser_ast_builder_translation_unit(mem, builder));

    kefir_result_t res;
    while (!PARSER_TOKEN_IS_SENTINEL(builder->parser, 0)) {
        while (PARSER_TOKEN_IS_PUNCTUATOR_EXT(builder->parser, 0, KEFIR_PUNCTUATOR_SEMICOLON, false)) {
            PARSER_SHIFT(builder->parser);
        }
        if (PARSER_TOKEN_IS_PRAGMA(builder->parser, 0)) {
            const struct kefir_token *token = PARSER_CURSOR_EXT(builder->parser, 0, false);
            REQUIRE_OK(kefir_parser_scan_pragma(&builder->parser->pragmas.file_scope, token->pragma,
                                                token->pragma_param, &token->source_location));
            PARSER_SHIFT_EXT(builder->parser, false);
            continue;
        }
        if (!PARSER_TOKEN_IS_SENTINEL(builder->parser, 0)) {
            REQUIRE_MATCH_OK(&res,
                             kefir_parser_ast_builder_scan(
                                 mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, external_declaration), NULL),
                             KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                                    "Expected declaration or function definition"));
            REQUIRE_OK(kefir_parser_ast_builder_translation_unit_append(mem, builder));
        }
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(translation_unit)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                             struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
