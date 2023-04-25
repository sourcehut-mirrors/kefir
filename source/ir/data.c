/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include "kefir/ir/builtins.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

#define BLOCK_CAPACITY 128
#define BLOCK_SIZE (BLOCK_CAPACITY * sizeof(struct kefir_ir_data_value))

static kefir_result_t on_block_init(struct kefir_mem *mem, struct kefir_block_tree *tree, kefir_size_t block_id,
                                    void *block, void *payload) {
    UNUSED(mem);
    UNUSED(tree);
    UNUSED(block_id);
    UNUSED(payload);
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid block"));

    ASSIGN_DECL_CAST(struct kefir_ir_data_value *, value_block, block);
    for (kefir_size_t i = 0; i < BLOCK_CAPACITY; i++) {
        value_block[i] = (struct kefir_ir_data_value){.type = KEFIR_IR_DATA_VALUE_UNDEFINED};
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_alloc(struct kefir_mem *mem, kefir_ir_data_storage_t storage,
                                   const struct kefir_ir_type *type, kefir_id_t type_id, struct kefir_ir_data *data) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type pointer"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));

    data->storage = storage;
    data->total_length = kefir_ir_type_slots(type);
    REQUIRE_OK(kefir_block_tree_init(&data->value_tree, BLOCK_SIZE));
    REQUIRE_OK(kefir_block_tree_on_block_init(&data->value_tree, on_block_init, NULL));
    data->type = type;
    data->type_id = type_id;
    data->finalized = false;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_free(struct kefir_mem *mem, struct kefir_ir_data *data) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE_OK(kefir_block_tree_free(mem, &data->value_tree));
    data->type = NULL;
    return KEFIR_OK;
}

static kefir_result_t value_entry_at(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                     struct kefir_ir_data_value **entry) {
    kefir_size_t block_id, block_offset;
    REQUIRE_OK(kefir_block_tree_get_block_offset(&data->value_tree, index * sizeof(struct kefir_ir_data_value),
                                                 &block_id, &block_offset));
    void *block;
    REQUIRE_OK(kefir_block_tree_block(mem, &data->value_tree, block_id, &block));
    ASSIGN_DECL_CAST(struct kefir_ir_data_value *, value_block, block);
    *entry = &value_block[block_offset / sizeof(struct kefir_ir_data_value)];
    return KEFIR_OK;
}

static kefir_result_t value_get_entry(const struct kefir_ir_data *data, kefir_size_t index,
                                      struct kefir_ir_data_value **value_ptr) {
    kefir_size_t block_id, block_offset;
    REQUIRE_OK(kefir_block_tree_get_block_offset(&data->value_tree, index * sizeof(struct kefir_ir_data_value),
                                                 &block_id, &block_offset));
    void *block;
    kefir_result_t res = kefir_block_tree_get_block(&data->value_tree, block_id, &block);
    if (res == KEFIR_NOT_FOUND) {
        *value_ptr = NULL;
        return KEFIR_OK;
    } else {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct kefir_ir_data_value *, value_block, block);
        *value_ptr = &value_block[block_offset / sizeof(struct kefir_ir_data_value)];
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_integer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         kefir_int64_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
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
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    kefir_uint64_t currentValue = 0;
    if (entry->type == KEFIR_IR_DATA_VALUE_INTEGER) {
        currentValue = entry->value.integer;
    } else {
        REQUIRE(entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "IR data cannot have non-integral type"));
        entry->type = KEFIR_IR_DATA_VALUE_INTEGER;
    }

    REQUIRE(width <= 64, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "IR data bitfield cannot be wider than 64 bits"));
    if (width == 64) {
        currentValue = value;
    } else {
        const kefir_uint64_t mask = (1ull << (width + 1)) - 1;
        currentValue = currentValue & (~(mask << offset));
        currentValue |= (value & mask) << offset;
    }
    entry->value.integer = currentValue;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_float32(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         kefir_float32_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
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
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_LONG_DOUBLE;
    entry->value.long_double = value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_string(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                        kefir_ir_string_literal_type_t type, const void *value, kefir_size_t length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_STRING;
    entry->value.raw.data = value;
    switch (type) {
        case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
            entry->value.raw.length = length;
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE16:
            entry->value.raw.length = length * sizeof(kefir_char16_t);
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE32:
            entry->value.raw.length = length * sizeof(kefir_char32_t);
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_pointer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                         const char *reference, kefir_size_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_POINTER;
    entry->value.pointer.reference = reference;
    entry->value.pointer.offset = offset;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_string_pointer(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                                kefir_id_t id, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_STRING_POINTER;
    entry->value.string_ptr.id = id;
    entry->value.string_ptr.offset = offset;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_set_raw(struct kefir_mem *mem, struct kefir_ir_data *data, kefir_size_t index,
                                     const void *raw, kefir_size_t length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    REQUIRE(!data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot modify finalized data"));

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_entry_at(mem, data, index, &entry));

    entry->type = KEFIR_IR_DATA_VALUE_RAW;
    entry->value.raw.data = raw;
    entry->value.raw.length = length;
    return KEFIR_OK;
}

struct finalize_param {
    struct kefir_ir_type_visitor *visitor;
    struct kefir_ir_data *data;
    kefir_size_t slot;
    kefir_bool_t defined;
    struct kefir_block_tree_iterator block_iterator;
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

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, param->slot++, &entry));
    if (entry != NULL) {
        REQUIRE(
            entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "IR data for structure/union cannot have directly assigned value"));
    }

    struct finalize_param subparam = {.visitor = param->visitor,
                                      .data = param->data,
                                      .slot = param->slot,
                                      .defined = false,
                                      .block_iterator = param->block_iterator};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &subparam, index + 1, typeentry->param));
    param->slot = subparam.slot;
    param->defined = param->defined || subparam.defined;
    param->block_iterator = subparam.block_iterator;

    if (entry != NULL) {
        entry->defined = subparam.defined;
        if (subparam.defined) {
            entry->type = KEFIR_IR_DATA_VALUE_AGGREGATE;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t finalize_array(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct finalize_param *, param, payload);

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, param->slot++, &entry));
    struct finalize_param subparam = {.visitor = param->visitor,
                                      .data = param->data,
                                      .slot = param->slot,
                                      .defined = false,
                                      .block_iterator = param->block_iterator};

    const kefir_size_t array_element_slots = kefir_ir_type_slots_of(type, index + 1);
    const kefir_size_t array_end_slot = param->slot + array_element_slots * (kefir_size_t) typeentry->param;
    for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
        REQUIRE_OK(kefir_block_tree_iter_skip_to(&param->data->value_tree, &subparam.block_iterator,
                                                 subparam.slot * sizeof(struct kefir_ir_data_value)));
        if (subparam.block_iterator.block == NULL) {
            // No initialized IR data blocks available => stop traversal
            break;
        } else if (subparam.slot < subparam.block_iterator.block_id * BLOCK_CAPACITY) {
            // Some undefined slots can be skipped
            const kefir_size_t undefined_slots =
                MIN(array_end_slot, subparam.block_iterator.block_id * BLOCK_CAPACITY) - subparam.slot;
            const kefir_size_t undefined_elements = undefined_slots / array_element_slots;
            if (undefined_elements > 0) {
                subparam.slot += undefined_elements * array_element_slots;
                i += undefined_elements - 1;
            }
            continue;
        }

        REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &subparam, index + 1, 1));
    }

    param->slot = subparam.slot;
    param->block_iterator = subparam.block_iterator;

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

static kefir_result_t finalize_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct finalize_param *, param, payload);

    struct kefir_ir_data_value *entry;
    REQUIRE_OK(value_get_entry(param->data, param->slot, &entry));
    switch ((kefir_ir_builtin_type_t) typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            if (entry != NULL) {
                REQUIRE(entry->type == KEFIR_IR_DATA_VALUE_UNDEFINED,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Vararg built-in data cannot be initialized"));
            }
            param->slot++;
            break;

        case KEFIR_IR_TYPE_BUILTIN_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected built-in type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_finalize(struct kefir_ir_data *data) {
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data pointer"));
    struct kefir_ir_type_visitor visitor;
    struct finalize_param param = {.visitor = &visitor, .data = data, .slot = 0, .defined = false};

    REQUIRE_OK(kefir_block_tree_iter(&data->value_tree, &param.block_iterator));

    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, finalize_unsupported));
    KEFIR_IR_TYPE_VISITOR_INIT_SCALARS(&visitor, finalize_scalar);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = finalize_struct_union;
    visitor.visit[KEFIR_IR_TYPE_UNION] = finalize_struct_union;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = finalize_array;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = finalize_builtin;
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

static kefir_result_t update_map_iter(struct kefir_ir_data_map_iterator *iter) {
    iter->has_mapped_values = iter->value_iter.block != NULL;
    iter->next_mapped_slot = iter->has_mapped_values ? iter->value_iter.block_id * BLOCK_CAPACITY : 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_map_iter(const struct kefir_ir_data *data, struct kefir_ir_data_map_iterator *iter) {
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data map iterator"));

    REQUIRE_OK(kefir_block_tree_iter(&data->value_tree, &iter->value_iter));
    REQUIRE_OK(update_map_iter(iter));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_map_next(struct kefir_ir_data_map_iterator *iter) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data map iterator"));

    REQUIRE_OK(kefir_block_tree_next(&iter->value_iter));
    REQUIRE_OK(update_map_iter(iter));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_data_map_skip_to(const struct kefir_ir_data *data, struct kefir_ir_data_map_iterator *iter,
                                         kefir_size_t slot) {
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data map iterator"));

    REQUIRE_OK(
        kefir_block_tree_iter_skip_to(&data->value_tree, &iter->value_iter, slot * sizeof(struct kefir_ir_data_value)));
    REQUIRE_OK(update_map_iter(iter));
    return KEFIR_OK;
}
