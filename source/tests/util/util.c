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

#include "kefir/test/util.h"
#include "kefir/codegen/amd64/system-v/platform.h"
#include <float.h>

static kefir_bool_t init_done = false;
static struct kefir_ir_target_platform IR_TARGET;
static struct kefir_ast_translator_environment TRANSLATOR_ENV;

struct kefir_ir_target_platform *kft_util_get_ir_target_platform() {
    if (!init_done) {
        REQUIRE(kefir_codegen_amd64_sysv_target_platform(&IR_TARGET) == KEFIR_OK, NULL);
        REQUIRE(kefir_ast_translator_environment_init(&TRANSLATOR_ENV, &IR_TARGET) == KEFIR_OK, NULL);
        init_done = true;
    }
    return &IR_TARGET;
}

struct kefir_ast_translator_environment *kft_util_get_translator_environment() {
    if (!init_done) {
        REQUIRE(kefir_codegen_amd64_sysv_target_platform(&IR_TARGET) == KEFIR_OK, NULL);
        REQUIRE(kefir_ast_translator_environment_init(&TRANSLATOR_ENV, &IR_TARGET) == KEFIR_OK, NULL);
        init_done = true;
    }
    return &TRANSLATOR_ENV;
}

const struct kefir_data_model_descriptor *kefir_util_default_data_model() {
    static const struct kefir_data_model_descriptor DATA_MODEL_DESCRIPTOR = {
        .model = KEFIR_DATA_MODEL_LP64,
        .byte_order = KEFIR_BYTE_ORDER_LITTLE_ENDIAN,
        .int_width = {.short_int = 16, .integer = 32, .long_int = 64, .long_long_int = 64},
        .floating_point = {.float_radix = FLT_RADIX,
                           .float_mantissa_digits = FLT_MANT_DIG,
                           .double_mantissa_digits = DBL_MANT_DIG,
                           .long_double_mantissa_digits = LDBL_MANT_DIG,
                           .float_digits = FLT_DIG,
                           .double_digits = DBL_DIG,
                           .long_double_digits = LDBL_DIG,
                           .float_min_exponent = FLT_MIN_EXP,
                           .double_min_exponent = DBL_MIN_EXP,
                           .long_double_min_exponent = LDBL_MIN_EXP,
                           .float_min10_exponent = FLT_MIN_10_EXP,
                           .double_min10_exponent = DBL_MIN_10_EXP,
                           .long_double_min10_exponent = LDBL_MIN_10_EXP,
                           .float_max_exponent = FLT_MAX_EXP,
                           .double_max_exponent = DBL_MAX_EXP,
                           .long_double_max_exponent = LDBL_MAX_EXP,
                           .float_max10_exponent = FLT_MAX_10_EXP,
                           .double_max10_exponent = DBL_MAX_10_EXP,
                           .long_double_max10_exponent = LDBL_MAX_10_EXP,
                           .float_max = FLT_MAX,
                           .double_max = DBL_MAX,
                           .long_double_max = LDBL_MAX,
                           .float_epsilon = FLT_EPSILON,
                           .double_epsilon = DBL_EPSILON,
                           .long_double_epsilon = LDBL_EPSILON,
                           .float_min = FLT_MIN,
                           .double_min = DBL_MIN,
                           .long_double_min = LDBL_MIN},
        .char_bit = 8};
    return &DATA_MODEL_DESCRIPTOR;
}

const struct kefir_ast_type_traits *kefir_util_default_type_traits() {
    static struct kefir_ast_type_traits DEFAULT_TYPE_TRAITS;
    static kefir_bool_t DEFAULT_TYPE_TRAITS_INIT_DONE = false;
    if (!DEFAULT_TYPE_TRAITS_INIT_DONE) {
        kefir_ast_type_traits_init(kefir_util_default_data_model(), &DEFAULT_TYPE_TRAITS);
        DEFAULT_TYPE_TRAITS_INIT_DONE = true;
    }
    return &DEFAULT_TYPE_TRAITS;
}
