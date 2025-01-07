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

#include "kefir/core/sort.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_mergesort(struct kefir_mem *mem, void *array, kefir_size_t element_size, kefir_size_t length,
                               kefir_sortcomparator_fn_t comparator_fn, void *comparator_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(array != NULL || length == 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid array"));
    REQUIRE(element_size > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid non-zero array element size"));
    REQUIRE(comparator_fn != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid sort comparator function"));

    REQUIRE(length > 1, KEFIR_OK);
    if (length == 2) {
        void *element1 = array;
        void *element2 = (void *) (((kefir_uptr_t) array) + element_size);
        kefir_int_t comparison;
        REQUIRE_OK(comparator_fn(element1, element2, &comparison, comparator_payload));
        if (comparison > 0) {
            void *tmp = KEFIR_MALLOC(mem, element_size);
            REQUIRE(tmp != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate mergesort temporary"));
            memcpy(tmp, element1, element_size);
            memcpy(element1, element2, element_size);
            memcpy(element2, tmp, element_size);
            KEFIR_FREE(mem, tmp);
        }
        return KEFIR_OK;
    }

    void *tmp_array = KEFIR_MALLOC(mem, element_size * length);
    REQUIRE(tmp_array != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate mergesort temporary"));
    memcpy(tmp_array, array, element_size * length);

    const kefir_size_t first_length = length / 2;
    const kefir_size_t second_length = length - length / 2;
    void *const first_half = tmp_array;
    void *const second_half = (void *) (((kefir_uptr_t) tmp_array) + first_length * element_size);
    kefir_result_t res =
        kefir_mergesort(mem, first_half, element_size, first_length, comparator_fn, comparator_payload);
    REQUIRE_CHAIN(&res,
                  kefir_mergesort(mem, second_half, element_size, second_length, comparator_fn, comparator_payload));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, tmp_array);
        return res;
    });

    kefir_size_t array_index = 0;
    kefir_size_t first_half_index = 0;
    kefir_size_t second_half_index = 0;
    for (; array_index < length; array_index++) {
        void *const array_target = (void *) (((kefir_uptr_t) array) + array_index * element_size);
        void *const first_half_source = (void *) (((kefir_uptr_t) first_half) + first_half_index * element_size);
        void *const second_half_source = (void *) (((kefir_uptr_t) second_half) + second_half_index * element_size);

        if (first_half_index == first_length) {
            const kefir_size_t num_of_elements = second_length - second_half_index;
            memcpy(array_target, second_half_source, num_of_elements * element_size);
            array_index += num_of_elements;
            break;
        }

        if (second_half_index == second_length) {
            const kefir_size_t num_of_elements = first_length - first_half_index;
            memcpy(array_target, first_half_source, num_of_elements * element_size);
            array_index += num_of_elements;
            break;
        }

        kefir_int_t comparison;
        REQUIRE_OK(comparator_fn(first_half_source, second_half_source, &comparison, comparator_payload));
        if (comparison <= 0) {
            memcpy(array_target, first_half_source, element_size);
            first_half_index++;
        } else {
            memcpy(array_target, second_half_source, element_size);
            second_half_index++;
        }
    }

    KEFIR_FREE(mem, tmp_array);
    REQUIRE(array_index == length, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected array index after sort"));
    return KEFIR_OK;
}
