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

#include "kefir/ast/pragma.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_pragma_state_init(struct kefir_ast_pragma_state *state) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST pragma state"));

    *state = (struct kefir_ast_pragma_state) {0};
    return KEFIR_OK;
}

kefir_result_t kefir_ast_pragma_state_merge(struct kefir_ast_pragma_state *state,
                                            const struct kefir_ast_pragma_state *merged_state) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST pragma state"));
    REQUIRE(merged_state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST pragma state"));

#define MERGE(_id)                                  \
    if (merged_state->_id.present) {                \
        state->_id.value = merged_state->_id.value; \
        state->_id.present = true;                  \
    }

    MERGE(fp_contract)
    MERGE(fenv_access)
    MERGE(fenv_round)
    MERGE(fenv_dec_round)
    MERGE(cx_limited_range)
    MERGE(pack)

    return KEFIR_OK;
}

kefir_result_t kefir_ast_pragma_state_merge_pack(struct kefir_ast_pragma_state *state,
                                                 const struct kefir_ast_pragma_state *merged_state) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST pragma state"));
    REQUIRE(merged_state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST pragma state"));

    MERGE(pack)

#undef MERGE
    return KEFIR_OK;
}
