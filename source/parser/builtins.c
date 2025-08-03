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

#include "kefir/parser/builtins.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

const char *KEFIR_PARSER_SUPPORTED_BUILTINS[] = {KEFIR_PARSER_BUILTIN_VA_START,
                                                 KEFIR_PARSER_BUILTIN_VA_END,
                                                 KEFIR_PARSER_BUILTIN_VA_ARG,
                                                 KEFIR_PARSER_BUILTIN_VA_COPY,
                                                 KEFIR_PARSER_BUILTIN_ALLOCA,
                                                 KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN,
                                                 KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX,
                                                 KEFIR_PARSER_BUILTIN_OFFSETOF,
                                                 KEFIR_PARSER_BUILTIN_TYPES_COMPATIBLE,
                                                 KEFIR_PARSER_BUILTIN_CHOOSE_EXPRESSION,
                                                 KEFIR_PARSER_BUILTIN_CONSTANT,
                                                 KEFIR_PARSER_BUILTIN_CLASSIFY_TYPE,
                                                 KEFIR_PARSER_BUILTIN_INF,
                                                 KEFIR_PARSER_BUILTIN_INFF,
                                                 KEFIR_PARSER_BUILTIN_INFL,
                                                 KEFIR_PARSER_BUILTIN_NAN,
                                                 KEFIR_PARSER_BUILTIN_NANF,
                                                 KEFIR_PARSER_BUILTIN_NANL,
                                                 KEFIR_PARSER_BUILTIN_ADD_OVERFLOW,
                                                 KEFIR_PARSER_BUILTIN_SUB_OVERFLOW,
                                                 KEFIR_PARSER_BUILTIN_MUL_OVERFLOW,
                                                 KEFIR_PARSER_BUILTIN_FFSG,
                                                 KEFIR_PARSER_BUILTIN_CLZG,
                                                 KEFIR_PARSER_BUILTIN_CTZG,
                                                 KEFIR_PARSER_BUILTIN_CLRSBG,
                                                 KEFIR_PARSER_BUILTIN_POPCOUNTG,
                                                 KEFIR_PARSER_BUILTIN_PARITYG,
                                                 KEFIR_PARSER_BUILTIN_KEFIR_INT_PRECISION,
                                                 NULL};

static const struct {
    const char *identifier;
    kefir_ast_builtin_operator_t builtin_op;
} BUILTINS[] = {{KEFIR_PARSER_BUILTIN_VA_START, KEFIR_AST_BUILTIN_VA_START},
                {KEFIR_PARSER_BUILTIN_VA_END, KEFIR_AST_BUILTIN_VA_END},
                {KEFIR_PARSER_BUILTIN_VA_COPY, KEFIR_AST_BUILTIN_VA_COPY},
                {KEFIR_PARSER_BUILTIN_VA_ARG, KEFIR_AST_BUILTIN_VA_ARG},
                {KEFIR_PARSER_BUILTIN_ALLOCA, KEFIR_AST_BUILTIN_ALLOCA},
                {KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN, KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN},
                {KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX, KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX},
                {KEFIR_PARSER_BUILTIN_OFFSETOF, KEFIR_AST_BUILTIN_OFFSETOF},
                {KEFIR_PARSER_BUILTIN_TYPES_COMPATIBLE, KEFIR_AST_BUILTIN_TYPES_COMPATIBLE},
                {KEFIR_PARSER_BUILTIN_CHOOSE_EXPRESSION, KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION},
                {KEFIR_PARSER_BUILTIN_CONSTANT, KEFIR_AST_BUILTIN_CONSTANT},
                {KEFIR_PARSER_BUILTIN_CLASSIFY_TYPE, KEFIR_AST_BUILTIN_CLASSIFY_TYPE},
                {KEFIR_PARSER_BUILTIN_INF, KEFIR_AST_BUILTIN_INFINITY_FLOAT64},
                {KEFIR_PARSER_BUILTIN_INFF, KEFIR_AST_BUILTIN_INFINITY_FLOAT32},
                {KEFIR_PARSER_BUILTIN_INFL, KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE},
                {KEFIR_PARSER_BUILTIN_NAN, KEFIR_AST_BUILTIN_NAN_FLOAT64},
                {KEFIR_PARSER_BUILTIN_NANF, KEFIR_AST_BUILTIN_NAN_FLOAT32},
                {KEFIR_PARSER_BUILTIN_NANL, KEFIR_AST_BUILTIN_NAN_LONG_DOUBLE},
                {KEFIR_PARSER_BUILTIN_ADD_OVERFLOW, KEFIR_AST_BUILTIN_ADD_OVERFLOW},
                {KEFIR_PARSER_BUILTIN_SUB_OVERFLOW, KEFIR_AST_BUILTIN_SUB_OVERFLOW},
                {KEFIR_PARSER_BUILTIN_MUL_OVERFLOW, KEFIR_AST_BUILTIN_MUL_OVERFLOW},
                {KEFIR_PARSER_BUILTIN_FFSG, KEFIR_AST_BUILTIN_FFSG},
                {KEFIR_PARSER_BUILTIN_CLZG, KEFIR_AST_BUILTIN_CLZG},
                {KEFIR_PARSER_BUILTIN_CTZG, KEFIR_AST_BUILTIN_CTZG},
                {KEFIR_PARSER_BUILTIN_CLRSBG, KEFIR_AST_BUILTIN_CLRSBG},
                {KEFIR_PARSER_BUILTIN_POPCOUNTG, KEFIR_AST_BUILTIN_POPCOUNTG},
                {KEFIR_PARSER_BUILTIN_PARITYG, KEFIR_AST_BUILTIN_PARITYG},
                {KEFIR_PARSER_BUILTIN_KEFIR_INT_PRECISION, KEFIR_AST_BUILTIN_KEFIR_INT_PRECISION}};
static const kefir_size_t BUILTIN_COUNT = sizeof(BUILTINS) / sizeof(BUILTINS[0]);

kefir_result_t kefir_parser_get_builtin_operation(const char *identifier, kefir_ast_builtin_operator_t *builtin_op) {
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(builtin_op != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to builtin operator"));

    for (kefir_size_t i = 0; i < BUILTIN_COUNT; i++) {
        if (strcmp(BUILTINS[i].identifier, identifier) == 0) {
            *builtin_op = BUILTINS[i].builtin_op;
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Provided identifier is not a builtin");
}
