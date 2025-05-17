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

#include "kefir/lexer/context.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_lexer_context_default(struct kefir_lexer_context *context) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser context"));

    context->integer_max_value = KEFIR_INT_MAX;
    context->uinteger_max_value = KEFIR_UINT_MAX;
    context->long_max_value = KEFIR_LONG_MAX;
    context->ulong_max_value = KEFIR_ULONG_MAX;
    context->long_long_max_value = KEFIR_LONG_LONG_MAX;
    context->ulong_long_max_value = KEFIR_ULONG_LONG_MAX;
    context->newline = U'\n';
    return KEFIR_OK;
}

static kefir_result_t match_max_value_by_width(kefir_size_t width, kefir_bool_t signedness, kefir_uint64_t *value) {
    switch (width) {
        case 8:
            if (!signedness) {
                *value = KEFIR_UINT8_MAX;
            } else {
                *value = KEFIR_INT8_MAX;
            }
            return KEFIR_OK;

        case 16:
            if (!signedness) {
                *value = KEFIR_UINT16_MAX;
            } else {
                *value = KEFIR_INT16_MAX;
            }
            return KEFIR_OK;

        case 32:
            if (!signedness) {
                *value = KEFIR_UINT32_MAX;
            } else {
                *value = KEFIR_INT32_MAX;
            }
            return KEFIR_OK;

        case 64:
            if (!signedness) {
                *value = KEFIR_UINT64_MAX;
            } else {
                *value = KEFIR_INT64_MAX;
            }
            return KEFIR_OK;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected integer width");
    }
}

kefir_result_t kefir_lexer_context_integral_width_from_data_model(
    struct kefir_lexer_context *context, const struct kefir_data_model_descriptor *data_model) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser context"));
    REQUIRE(data_model != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid data model"));

    REQUIRE_OK(match_max_value_by_width(data_model->scalar_width.int_bits, true, &context->integer_max_value));
    REQUIRE_OK(match_max_value_by_width(data_model->scalar_width.int_bits, false, &context->uinteger_max_value));
    REQUIRE_OK(match_max_value_by_width(data_model->scalar_width.long_bits, true, &context->long_max_value));
    REQUIRE_OK(match_max_value_by_width(data_model->scalar_width.long_bits, false, &context->ulong_max_value));
    REQUIRE_OK(match_max_value_by_width(data_model->scalar_width.long_long_bits, true, &context->long_long_max_value));
    REQUIRE_OK(
        match_max_value_by_width(data_model->scalar_width.long_long_bits, false, &context->ulong_long_max_value));
    return KEFIR_OK;
}
