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

#ifndef KEFIR_PARSER_BUILTINS_H_
#define KEFIR_PARSER_BUILTINS_H_

#include "kefir/parser/parser.h"
#include "kefir/ast/constants.h"

#define KEFIR_PARSER_BUILTIN_VA_START "__builtin_c23_va_start"
#define KEFIR_PARSER_BUILTIN_VA_END "__builtin_va_end"
#define KEFIR_PARSER_BUILTIN_VA_ARG "__builtin_va_arg"
#define KEFIR_PARSER_BUILTIN_VA_COPY "__builtin_va_copy"
#define KEFIR_PARSER_BUILTIN_ALLOCA "__builtin_alloca"
#define KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN "__builtin_alloca_with_align"
#define KEFIR_PARSER_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX "__builtin_alloca_with_align_and_max"
#define KEFIR_PARSER_BUILTIN_OFFSETOF "__builtin_offsetof"
#define KEFIR_PARSER_BUILTIN_TYPES_COMPATIBLE "__builtin_types_compatible_p"
#define KEFIR_PARSER_BUILTIN_CHOOSE_EXPRESSION "__builtin_choose_expr"
#define KEFIR_PARSER_BUILTIN_CONSTANT "__builtin_constant_p"
#define KEFIR_PARSER_BUILTIN_CLASSIFY_TYPE "__builtin_classify_type"
#define KEFIR_PARSER_BUILTIN_INF "__builtin_inf"
#define KEFIR_PARSER_BUILTIN_INFF "__builtin_inff"
#define KEFIR_PARSER_BUILTIN_INFL "__builtin_infl"
#define KEFIR_PARSER_BUILTIN_NAN "__builtin_nan"
#define KEFIR_PARSER_BUILTIN_NANF "__builtin_nanf"
#define KEFIR_PARSER_BUILTIN_NANL "__builtin_nanl"
#define KEFIR_PARSER_BUILTIN_ADD_OVERFLOW "__builtin_add_overflow"
#define KEFIR_PARSER_BUILTIN_SUB_OVERFLOW "__builtin_sub_overflow"
#define KEFIR_PARSER_BUILTIN_MUL_OVERFLOW "__builtin_mul_overflow"
#define KEFIR_PARSER_BUILTIN_FFSG "__builtin_ffsg"
#define KEFIR_PARSER_BUILTIN_CLZG "__builtin_clzg"
#define KEFIR_PARSER_BUILTIN_CTZG "__builtin_ctzg"
#define KEFIR_PARSER_BUILTIN_CLRSBG "__builtin_clrsbg"

kefir_result_t kefir_parser_get_builtin_operation(const char *, kefir_ast_builtin_operator_t *);

extern const char *KEFIR_PARSER_SUPPORTED_BUILTINS[];

#endif
