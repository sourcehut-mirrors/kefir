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

#include "kefir/parser/pragma.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_parser_pragmas_init(struct kefir_parser_pragmas *pragmas) {
    REQUIRE(pragmas != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser pragmas"));

    REQUIRE_OK(kefir_ast_pragma_state_init(&pragmas->file_scope));
    pragmas->in_function_scope = false;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_pragmas_collect(struct kefir_ast_pragma_state *state,
                                            const struct kefir_parser_pragmas *pragmas) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST pragma state"));
    REQUIRE(pragmas != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser pragmas"));

    *state = pragmas->file_scope;
    return KEFIR_OK;
}
