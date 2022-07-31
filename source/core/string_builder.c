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

#include "kefir/core/string_builder.h"
#include "kefir/core/error.h"
#include "kefir/core/os_error.h"
#include "kefir/core/util.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BUF_INC 64

kefir_result_t kefir_string_builder_init(struct kefir_string_builder *builder) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to string builder"));

    builder->capacity = 0;
    builder->length = 0;
    builder->string = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_string_builder_free(struct kefir_mem *mem, struct kefir_string_builder *builder) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string builder"));

    if (builder->string != NULL) {
        KEFIR_FREE(mem, builder->string);
    }
    builder->string = NULL;
    builder->length = 0;
    builder->capacity = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_string_builder_printf(struct kefir_mem *mem, struct kefir_string_builder *builder, const char *fmt,
                                           ...) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string builder"));
    REQUIRE(fmt != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid format string"));

    va_list args, args2;

    // Determine formatted string length and reallocate target string if necessary
    va_start(args, fmt);
    int length = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    REQUIRE(length >= 0, KEFIR_SET_OS_ERROR("Failed to format a string"));
    if (builder->length + length >= builder->capacity) {
        kefir_size_t new_capacity = builder->length + length + 1 + BUF_INC;
        char *new_string = KEFIR_MALLOC(mem, new_capacity);
        if (builder->string != NULL) {
            strcpy(new_string, builder->string);
            KEFIR_FREE(mem, builder->string);
        } else {
            *new_string = '\0';
        }
        builder->string = new_string;
        builder->capacity = new_capacity;
    }

    // Perform formatting
    va_start(args2, fmt);
    length = vsnprintf(builder->string + builder->length, length + 1, fmt, args2);
    va_end(args2);
    REQUIRE(length >= 0, KEFIR_SET_OS_ERROR("Failed to format a string"));
    builder->length += length;

    return KEFIR_OK;
}
