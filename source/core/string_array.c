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

#include "kefir/core/string_array.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define INIT_CAPACITY 8
#define CAPACITY_INCREMENT 32

kefir_result_t kefir_string_array_init(struct kefir_mem *mem, struct kefir_string_array *array) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(array != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to string array"));

    array->array = KEFIR_MALLOC(mem, sizeof(char *) * INIT_CAPACITY);
    REQUIRE(array->array != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string array"));
    array->array[0] = NULL;
    array->length = 0;
    array->capacity = INIT_CAPACITY;
    return KEFIR_OK;
}

kefir_result_t kefir_string_array_free(struct kefir_mem *mem, struct kefir_string_array *array) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(array != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string array"));

    for (kefir_size_t i = 0; i < array->length; i++) {
        KEFIR_FREE(mem, array->array[i]);
    }
    KEFIR_FREE(mem, array->array);
    array->array = NULL;
    array->capacity = 0;
    array->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_string_array_append(struct kefir_mem *mem, struct kefir_string_array *array, const char *string) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(array != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string array"));
    REQUIRE(string != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));

    if (array->length + 1 == array->capacity) {
        kefir_size_t new_capacity = array->capacity * 2 + CAPACITY_INCREMENT;
        char **new_array = KEFIR_MALLOC(mem, sizeof(char *) * new_capacity);
        REQUIRE(new_array != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate string array"));
        memcpy(new_array, array->array, sizeof(char *) * array->length);
        KEFIR_FREE(mem, array->array);
        array->array = new_array;
        array->capacity = new_capacity;
    }

    array->array[array->length] = KEFIR_MALLOC(mem, strlen(string) + 1);
    REQUIRE(array->array[array->length] != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string"));
    strcpy(array->array[array->length], string);
    array->array[++array->length] = NULL;
    return KEFIR_OK;
}
