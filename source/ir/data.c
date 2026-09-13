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

#include "kefir/ir/data.h"
#include "kefir/ir/instr.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

#define INIT_BLOCK_CAPACITY 8
struct value_block {
    kefir_size_t length;
    struct kefir_ir_data_value values[];
};

static kefir_result_t on_block_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct value_block *, value_block, value);

    if (value_block != NULL) {
        for (kefir_size_t i = 0; i < value_block->length; i++) {
            if (value_block->values[i].type == KEFIR_IR_DATA_VALUE_BITS) {
                KEFIR_FREE(mem, value_block->values[i].value.large->bits.bits);
            }
        }
        KEFIR_FREE(mem, value_block);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_alloc(struct kefir_mem *mem, struct kefir_memory_arena *arena, kefir_ir_data_storage_t storage,
                                   const struct kefir_ir_type *type, kefir_id_t type_id, struct kefir_ir_data *data) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type pointer"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));

    data->storage = storage;
    data->total_length = kefir_ir_type_slots(type);
    REQUIRE_OK(kefir_hashtree_init(&data->values, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&data->values, on_block_removal, NULL));
    data->arena = arena;
    data->type = type;
    data->type_id = type_id;
    data->finalized = false;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_free(struct kefir_mem *mem, struct kefir_ir_data *data) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE_OK(kefir_hashtree_free(mem, &data->values));
    data->type = NULL;
    return KEFIR_OK;
}

static kefir_result_t value_entry_at(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                     struct kefir_ir_data_value **entry) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&data->values, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_min(&data->values, &node));
    } else {
        REQUIRE_OK(res);
    }

    struct value_block *value_block = NULL;
    kefir_size_t begin_offset = 0;
    if (node != NULL && node->key <= index && node->key + ((const struct value_block *) node->value)->length <= index && index < node->key + ((const struct value_block *) node->value)->length + INIT_BLOCK_CAPACITY) {
        value_block = (struct value_block *) node->value;
        kefir_size_t new_length = value_block->length + INIT_BLOCK_CAPACITY;
        value_block = KEFIR_REALLOC(mem, value_block, sizeof(struct value_block) + sizeof(struct kefir_ir_data_value) * new_length);
        REQUIRE(value_block != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data values"));
    
        for (kefir_size_t i = value_block->length; i < new_length; i++) {
            value_block->values[i] = (struct kefir_ir_data_value){.type = KEFIR_IR_DATA_VALUE_UNDEFINED};
        }
        value_block->length = new_length;
        node->value = (kefir_hashtree_value_t) value_block;
        begin_offset = node->key;

        for (;;) {
            struct kefir_hashtree_node *next_node = kefir_hashtree_next_node(&data->values, node);
            if (next_node == NULL || next_node->key > node->key + value_block->length) {
                break;
            }
            ASSIGN_DECL_CAST(struct value_block *, next_block, next_node->value);

            kefir_size_t new_length = value_block->length + next_block->length;
            value_block = KEFIR_REALLOC(mem, value_block, sizeof(struct value_block) + sizeof(struct kefir_ir_data_value) * new_length);
            REQUIRE(value_block != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data values"));
            
            memcpy(&value_block->values[value_block->length], next_block->values, sizeof(struct kefir_ir_data_value) * next_block->length);
            value_block->length = new_length;
            node->value = (kefir_hashtree_value_t) value_block;

            KEFIR_FREE(mem, next_block);
            next_node->value = (kefir_hashtree_value_t) NULL;

            REQUIRE_OK(kefir_hashtree_delete(mem, &data->values, (kefir_hashtree_key_t) next_node->key));
        }
    } else if (node == NULL || node->key > index || node->key + ((const struct value_block *) node->value)->length <= index) {
        value_block = KEFIR_MALLOC(mem, sizeof(struct value_block) + sizeof(struct kefir_ir_data_value) * INIT_BLOCK_CAPACITY);
        REQUIRE(value_block != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data values"));

        value_block->length = INIT_BLOCK_CAPACITY;
        for (kefir_size_t i = 0; i < value_block->length; i++) {
            value_block->values[i] = (struct kefir_ir_data_value){.type = KEFIR_IR_DATA_VALUE_UNDEFINED};
        }

        begin_offset = index / value_block->length * value_block->length;
        res = kefir_hashtree_insert(mem, &data->values, (kefir_hashtree_key_t) begin_offset, (kefir_hashtree_value_t) value_block);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, value_block);
            return res;
        });
    } else {
        begin_offset = node->key;
        value_block = (struct value_block *) node->value;
    }
    *entry = &value_block->values[index - begin_offset];
    return KEFIR_OK;
}

static kefir_result_t value_get_entry(const struct kefir_ir_data *data, kefir_size_t index,
                                      struct kefir_ir_data_value **value_ptr) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&data->values, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_min(&data->values, &node));
    } else {
        REQUIRE_OK(res);
    }

    if (node == NULL || node->key > index || node->key + ((const struct value_block *) node->value)->length <= index) {
        *value_ptr = NULL;
    } else {
        ASSIGN_DECL_CAST(struct value_block *, value_block,
            node->value);
        *value_ptr = &value_block->values[index - node->key];
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_integer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         kefir_int64_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_INTEGER;
    entry->value.integer = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_bitfield(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                          kefir_uint64_t value, kefir_size_t offset, kefir_size_t width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    const kefir_size_t tail_bit_offset = offset + width;
    const kefir_size_t container_width = sizeof(kefir_uint64_t) * CHAR_BIT;
    const kefir_size_t bit_num_of_containers = tail_bit_offset / container_width;
    if (entry->type == KEFIR_IR_DATA_VALUE_BITS) {
        if (bit_num_of_containers >= entry->value.large->bits.length) {
            const kefir_size_t new_length = bit_num_of_containers + 1;
            kefir_uint64_t *const new_bits = KEFIR_MALLOC(mem, sizeof(kefir_uint64_t) * new_length);
            REQUIRE(new_bits != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bitfield data"));
            memset(new_bits, 0, sizeof(kefir_uint64_t) * new_length);
            memcpy(new_bits, entry->value.large->bits.bits, sizeof(kefir_uint64_t) * entry->value.large->bits.length);
            KEFIR_FREE(mem, entry->value.large->bits.bits);
            entry->value.large->bits.length = new_length;
            entry->value.large->bits.bits = new_bits;
        }
    } else {
        REQUIRE(entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "IR data cannot have non-integral type"));
        entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
        REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));
        entry->type = KEFIR_IR_DATA_VALUE_BITS;
        entry->value.large->bits.length = bit_num_of_containers + 1;
        entry->value.large->bits.bits = KEFIR_MALLOC(mem, sizeof(kefir_uint64_t) * entry->value.large->bits.length);
        REQUIRE(entry->value.large->bits.bits != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bitfield data"));
        for (kefir_size_t i = 0; i < entry->value.large->bits.length; i++) {
            entry->value.large->bits.bits[i] = 0;
        }
    }

    REQUIRE(width <= 64, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "IR data bitfield cannot be wider than 64 bits"));
    while (width > 0) {
        const kefir_size_t bit_container = offset / container_width;
        const kefir_size_t bit_offset = offset % container_width;
        const kefir_size_t part_width = MIN(width, container_width - bit_offset);
        kefir_uint64_t *container = &entry->value.large->bits.bits[bit_container];
        if (part_width == 64) {
            *container = value;
        } else {
            const kefir_uint64_t mask = (1ull << part_width) - 1;
            *container = *container & (~(mask << bit_offset));
            *container |= (value & mask) << bit_offset;
            value >>= part_width;
        }

        width -= part_width;
        offset += part_width;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_float32(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         kefir_float32_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_FLOAT32;
    entry->value.float32 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_float64(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         kefir_float64_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_FLOAT64;
    entry->value.float64 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_long_double(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                             kefir_long_double_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_LONG_DOUBLE;
    entry->value.large->long_double[0] = kefir_ir_long_double_upper_half(value);
    entry->value.large->long_double[1] = kefir_ir_long_double_lower_half(value);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_decimal32(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                           kefir_dfp_decimal32_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_DECIMAL32;
    entry->value.decimal32 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_decimal64(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                           kefir_dfp_decimal64_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_DECIMAL64;
    entry->value.decimal64 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_decimal128(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                            kefir_dfp_decimal128_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_DECIMAL128;
    entry->value.large->decimal128 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_complex_float32(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                                 kefir_float32_t real, kefir_float32_t imaginary) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32;
    entry->value.complex_float32.real = real;
    entry->value.complex_float32.imaginary = imaginary;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_complex_float64(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                                 kefir_float64_t real, kefir_float64_t imaginary) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64;
    entry->value.large->complex_float64.real = real;
    entry->value.large->complex_float64.imaginary = imaginary;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_complex_long_double(struct kefir_mem *mem, struct kefir_ir_data *data,
                                                     kefir_size_t index, kefir_long_double_t real,
                                                     kefir_long_double_t imaginary) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    if (entry->type != KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE) {
        REQUIRE(entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected IR data entry type"));
        entry->value.big = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_big_value), _Alignof(union kefir_ir_data_big_value));
        REQUIRE(entry->value.big != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data entry value"));
        entry->type = KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE;
    }
    entry->value.big->complex_long_double.real[0] = kefir_ir_long_double_upper_half(real);
    entry->value.big->complex_long_double.real[1] = kefir_ir_long_double_lower_half(real);
    entry->value.big->complex_long_double.imaginary[0] = kefir_ir_long_double_upper_half(imaginary);
    entry->value.big->complex_long_double.imaginary[1] = kefir_ir_long_double_lower_half(imaginary);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_string(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                        kefir_ir_string_literal_type_t type, const void *value, kefir_size_t length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_STRING;
    entry->value.large->raw.data = value;
    switch (type) {
        case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
            entry->value.large->raw.length = length;
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE16:
            entry->value.large->raw.length = length * sizeof(kefir_char16_t);
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE32:
            entry->value.large->raw.length = length * sizeof(kefir_char32_t);
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_pointer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         const char *reference, kefir_size_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_POINTER;
    entry->value.large->pointer.reference = reference;
    entry->value.large->pointer.offset = offset;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_string_pointer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                                kefir_id_t id, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_STRING_POINTER;
    entry->value.large->string_ptr.id = id;
    entry->value.large->string_ptr.offset = offset;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_raw(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                     const void *raw, kefir_size_t length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->value.large = kefir_memory_arena_alloc(data->arena, sizeof(union kefir_ir_data_large_value), _Alignof(union kefir_ir_data_large_value));
    REQUIRE(entry->value.large != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR data value"));

    entry->type = KEFIR_IR_DATA_VALUE_RAW;
    entry->value.large->raw.data = raw;
    entry->value.large->raw.length = length;
    return KEFIR_OK;
}

struct finalize_param {
    struct kefir_mem *mem;
    struct kefir_ir_type_visitor *visitor;
    struct kefir_ir_data *data;
    kefir_size_t slot;
    kefir_bool_t defined;
};

static kefir_result_t finalize_unsupported(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unsupported IR data type");
}

static kefir_result_t finalize_scalar(const struct kefir_ir_type *type, kefir_size_t index,
                                      const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct finalize_param *, param, payload);

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, param->slot++, &entry));
    if (entry != NULL) {
        entry->defined = entry->type != KEFIR_IR_DATA_VALUE_UNDEFINED;
        param->defined = param->defined || entry->defined;
    }
    return KEFIR_OK;
}

static kefir_result_t finalize_struct_union(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct finalize_param *, param, payload);

    const kefir_size_t entry_slot = param->slot++;
    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, entry_slot, &entry));
    if (entry != NULL) {
        REQUIRE(
            entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "IR data for structure/union cannot have directly assigned value"));
    }

    struct finalize_param subparam = {.mem = param->mem,
                                      .visitor = param->visitor,
                                      .data = param->data,
                                      .slot = param->slot,
                                      .defined = false};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &subparam, index + 1, typeentry->param));
    param->slot = subparam.slot;
    param->defined = param->defined || subparam.defined;

    if (subparam.defined) {
        REQUIRE_OK(value_entry_at(param->mem, param->data, entry_slot, &entry));
        entry->defined = subparam.defined;
        entry->type = KEFIR_IR_DATA_VALUE_AGGREGATE;
    }

    return KEFIR_OK;
}

static kefir_result_t finalize_array(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct finalize_param *, param, payload);

    const kefir_size_t entry_slot = param->slot++;
    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, entry_slot, &entry));
    struct finalize_param subparam = {.mem = param->mem,
                                      .visitor = param->visitor,
                                      .data = param->data,
                                      .slot = param->slot,
                                      .defined = false};

    const kefir_size_t array_element_slots = kefir_ir_type_slots_of(type, index + 1);
    const kefir_size_t array_end_slot = param->slot + array_element_slots * (kefir_size_t) typeentry->param;
    for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
        kefir_size_t closest_block;
        kefir_result_t res = kefir_ir_data_find_closest_block(param->data, subparam.slot, &closest_block);
        if (res == KEFIR_NOT_FOUND) {
            // No initialized IR data blocks available => stop traversal
            break;
        } 
        REQUIRE_OK(res);
        if (subparam.slot < closest_block) {
            // Some undefined slots can be skipped
            const kefir_size_t undefined_slots =
                MIN(array_end_slot, closest_block) - subparam.slot;
            const kefir_size_t undefined_elements = undefined_slots / array_element_slots;
            if (undefined_elements > 0) {
                subparam.slot += undefined_elements * array_element_slots;
                i += undefined_elements - 1;
                continue;
            }
        }

        REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &subparam, index + 1, 1));
    }

    param->slot = subparam.slot;

    if (subparam.defined || entry != NULL) {
        REQUIRE_OK(value_entry_at(param->mem, param->data, entry_slot, &entry));
    }
    if (entry != NULL) {
        entry->defined = subparam.defined || entry->type != KEFIR_IR_DATA_VALUE_UNDEFINED;
        param->defined = param->defined || entry->defined;
        if (subparam.defined) {
            REQUIRE(entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
                    KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE,
                                    "Array data cannot simultaneously have directly assigned and aggregate values"));
            entry->type = KEFIR_IR_DATA_VALUE_AGGREGATE;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_finalize(struct kefir_mem *mem, struct kefir_ir_data *data) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    struct kefir_ir_type_visitor visitor;
    struct finalize_param param = {.mem = mem, .visitor = &visitor, .data = data, .slot = 0, .defined = false};

    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, finalize_unsupported));
    KEFIR_IR_TYPE_VISITOR_INIT_SCALARS(&visitor, finalize_scalar);
    KEFIR_IR_TYPE_VISITOR_INIT_COMPLEX(&visitor, finalize_scalar);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = finalize_struct_union;
    visitor.visit[KEFIR_IR_TYPE_UNION] = finalize_struct_union;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = finalize_array;
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(data->type, &visitor, &param, 0, kefir_ir_type_length(data->type)));
    data->finalized = true;
    data->defined = param.defined;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_value_at(const struct kefir_ir_data *data, kefir_size_t index,
                                      const struct kefir_ir_data_value **value_ptr) {
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR data value"));

    struct kefir_ir_data_value *value = NULL;
    REQUIRE_OK(value_get_entry(data, index, &value));
    if (value == NULL) {
        static const struct kefir_ir_data_value EMPTY_VALUE = {.type = KEFIR_IR_DATA_VALUE_UNDEFINED};
        *value_ptr = &EMPTY_VALUE;
    } else {
        *value_ptr = value;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_find_closest_block(const struct kefir_ir_data *data, kefir_size_t index, kefir_size_t *closest_index_ptr) {
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(index < data->total_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index exceeds IR data length"));
    REQUIRE(closest_index_ptr, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Requested pointer to IR data index"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_lower_bound(&data->values, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_min(&data->values, &node));
    } else {
        REQUIRE_OK(res);
    }

    if (node != NULL && index >= node->key + ((const struct value_block *) node->value)->length) {
        node = kefir_hashtree_next_node(&data->values, node);
    }
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find IR data closest block"));
    *closest_index_ptr = node->key;
    return KEFIR_OK;
}
