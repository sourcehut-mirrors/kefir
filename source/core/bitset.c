/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/core/bitset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_bitset_init(struct kefir_bitset *bitset) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bitset"));

    bitset->content = NULL;
    bitset->length = 0;
    bitset->capacity = 0;
    bitset->static_content = false;
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_init_static(struct kefir_bitset *bitset, kefir_uint64_t *content, kefir_size_t capacity,
                                        kefir_size_t length) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bitset"));
    REQUIRE(content != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to static bitset content"));
    REQUIRE(capacity > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty static bitset content"));

    bitset->content = content;
    bitset->capacity = capacity;
    bitset->length = length;
    bitset->static_content = true;

    memset(content, 0, sizeof(kefir_uint64_t) * capacity);
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_free(struct kefir_mem *mem, struct kefir_bitset *bitset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));

    if (!bitset->static_content) {
        KEFIR_FREE(mem, bitset->content);
    }
    bitset->content = NULL;
    bitset->length = 0;
    bitset->capacity = 0;
    bitset->static_content = false;
    return KEFIR_OK;
}

#define BITS_PER_ENTRY (sizeof(kefir_uint64_t) * CHAR_BIT)

kefir_result_t kefir_bitset_get(const struct kefir_bitset *bitset, kefir_size_t index, kefir_bool_t *value_ptr) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(index < bitset->length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested bit is outside of bitset bounds"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    const kefir_size_t position = index / BITS_PER_ENTRY;
    const kefir_size_t offset = index % BITS_PER_ENTRY;

    kefir_uint64_t value = (bitset->content[position] >> offset) & 1;
    *value_ptr = (value == 1);
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_set(const struct kefir_bitset *bitset, kefir_size_t index, kefir_bool_t bit_value) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(index < bitset->length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested bit is outside of bitset bounds"));

    const kefir_size_t position = index / BITS_PER_ENTRY;
    const kefir_size_t offset = index % BITS_PER_ENTRY;

    const kefir_uint64_t mask = ~(1ull << offset);
    const kefir_uint64_t update = (bit_value ? 1ull : 0ull) << offset;

    kefir_uint64_t value = bitset->content[position];
    value = (value & mask) | update;
    bitset->content[position] = value;
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_find(const struct kefir_bitset *bitset, kefir_bool_t value, kefir_size_t begin,
                                 kefir_size_t *result_ptr) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to index"));

    for (kefir_size_t i = begin; i < bitset->length; i++) {
        kefir_bool_t bit;
        REQUIRE_OK(kefir_bitset_get(bitset, i, &bit));
        if (bit == value) {
            *result_ptr = i;
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested bit in bitset");
}

kefir_result_t kefir_bitset_set_consecutive(const struct kefir_bitset *bitset, kefir_size_t index, kefir_size_t length,
                                            kefir_bool_t value) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));

    for (kefir_size_t i = 0; i < length; i++) {
        REQUIRE_OK(kefir_bitset_set(bitset, index + i, value));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_find_consecutive(const struct kefir_bitset *bitset, kefir_bool_t value, kefir_size_t length,
                                             kefir_size_t begin, kefir_size_t *result_ptr) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to index"));

    kefir_size_t index = begin;
    for (; index < bitset->length; index++) {
        REQUIRE_OK(kefir_bitset_find(bitset, value, index, &index));
        kefir_bool_t found = true;
        for (kefir_size_t i = index; i < index + length; i++) {
            if (i >= bitset->length) {
                found = false;
                break;
            }

            kefir_bool_t bit;
            REQUIRE_OK(kefir_bitset_get(bitset, i, &bit));
            if (bit != value) {
                found = false;
                break;
            }
        }

        if (found) {
            *result_ptr = index;
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested consequtive bit in bitset");
}

kefir_result_t kefir_bitset_clear(const struct kefir_bitset *bitset) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));

    if (bitset->content != NULL) {
        memset(bitset->content, 0, sizeof(kefir_uint64_t) * bitset->capacity);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_length(const struct kefir_bitset *bitset, kefir_size_t *length_ptr) {
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(length_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bitset length"));

    *length_ptr = bitset->length;
    return KEFIR_OK;
}

kefir_result_t kefir_bitset_resize(struct kefir_mem *mem, struct kefir_bitset *bitset, kefir_size_t new_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bitset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bitset"));
    REQUIRE(!bitset->static_content, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot resize static bitset"));
    REQUIRE(new_length != bitset->length, KEFIR_OK);

    if (new_length < bitset->length) {
        bitset->length = new_length;
        return KEFIR_OK;
    }

    if (new_length >= bitset->capacity * BITS_PER_ENTRY) {
        kefir_size_t new_capacity = new_length / BITS_PER_ENTRY + 8;
        kefir_uint64_t *new_content = KEFIR_MALLOC(mem, sizeof(kefir_uint64_t) * new_capacity);
        REQUIRE(new_content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bitset content"));

        if (bitset->content != NULL) {
            memcpy(new_content, bitset->content, bitset->capacity * sizeof(kefir_uint64_t));
        }
        KEFIR_FREE(mem, bitset->content);
        bitset->content = new_content;
        bitset->capacity = new_capacity;
    }

    kefir_size_t old_length = bitset->length;
    bitset->length = new_length;
    for (kefir_size_t i = old_length; i < new_length; i++) {
        REQUIRE_OK(kefir_bitset_set(bitset, i, false));
    }
    return KEFIR_OK;
}
