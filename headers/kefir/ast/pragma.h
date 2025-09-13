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

#ifndef KEFIR_AST_PRAGMA_H_
#define KEFIR_AST_PRAGMA_H_

#include "kefir/core/basic-types.h"
#include "kefir/ast/base.h"

typedef enum kefir_ast_pragma_on_off_value {
    KEFIR_AST_PRAGMA_VALUE_ON,
    KEFIR_AST_PRAGMA_VALUE_OFF,
    KEFIR_AST_PRAGMA_VALUE_DEFAULT
} kefir_ast_pragma_on_off_value_t;

typedef enum kefir_ast_pragma_direction_value {
    KEFIR_AST_PRAGMA_VALUE_FE_DOWNWARD,
    KEFIR_AST_PRAGMA_VALUE_FE_TONEAREST,
    KEFIR_AST_PRAGMA_VALUE_FE_TONEARESTFROMZERO,
    KEFIR_AST_PRAGMA_VALUE_FE_TOWARDZERO,
    KEFIR_AST_PRAGMA_VALUE_FE_UPWARD,
    KEFIR_AST_PRAGMA_VALUE_FE_DYNAMIC
} kefir_ast_pragma_direction_value_t;

typedef enum kefir_ast_pragma_dec_direction_value {
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_DOWNWARD,
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_TONEAREST,
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_TONEARESTFROMZERO,
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_TOWARDZERO,
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_UPWARD,
    KEFIR_AST_PRAGMA_VALUE_FE_DEC_DYNAMIC
} kefir_ast_pragma_dec_direction_value_t;

typedef struct kefir_ast_pragma_state {
    struct {
        kefir_bool_t present;
        kefir_ast_pragma_on_off_value_t value;
    } fp_contract;

    struct {
        kefir_bool_t present;
        kefir_ast_pragma_on_off_value_t value;
    } fenv_access;

    struct {
        kefir_bool_t present;
        kefir_ast_pragma_on_off_value_t value;
    } cx_limited_range;

    struct {
        kefir_bool_t present;
        kefir_ast_pragma_direction_value_t value;
    } fenv_round;

    struct {
        kefir_bool_t present;
        kefir_ast_pragma_dec_direction_value_t value;
    } fenv_dec_round;
} kefir_parser_pragma_state_t;

#define KEFIR_AST_PRAGMA_STATE_IS_PRESENT(_state)                                                            \
    ((_state)->fp_contract.present || (_state)->fenv_access.present || (_state)->cx_limited_range.present || \
     (_state)->fenv_round.present || (_state)->fenv_dec_round.present)

#define KEFIR_AST_PRAGMA_STATE_FENV_ACCESS_ON(_state) \
    ((_state)->fenv_access.present && (_state)->fenv_access.value == KEFIR_AST_PRAGMA_VALUE_ON)
#define KEFIR_AST_PRAGMA_STATE_FP_CONTRACT_OFF(_state) \
    ((_state)->fp_contract.present && (_state)->fp_contract.value == KEFIR_AST_PRAGMA_VALUE_OFF)

kefir_result_t kefir_ast_pragma_state_init(struct kefir_ast_pragma_state *);
kefir_result_t kefir_ast_pragma_state_merge(struct kefir_ast_pragma_state *, const struct kefir_ast_pragma_state *);

#endif
