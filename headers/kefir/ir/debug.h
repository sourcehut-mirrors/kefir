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

#ifndef KEFIR_IR_DEBUG_H_
#define KEFIR_IR_DEBUG_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/source_location.h"
#include "kefir/core/interval_tree.h"
#include "kefir/core/list.h"

typedef kefir_id_t kefir_ir_debug_entry_id_t;
#define KEFIR_IR_DEBUG_ENTRY_ID_NONE KEFIR_ID_NONE

typedef enum kefir_ir_debug_entry_tag {
    KEFIR_IR_DEBUG_ENTRY_TYPE_VOID,
    KEFIR_IR_DEBUG_ENTRY_TYPE_BOOLEAN,
    KEFIR_IR_DEBUG_ENTRY_TYPE_CHARACTER,
    KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_CHARACTER,
    KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_CHARACTER,
    KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT,
    KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT,
    KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT,
    KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT,
    KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER,
    KEFIR_IR_DEBUG_ENTRY_TYPE_ENUMERATION,
    KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE,
    KEFIR_IR_DEBUG_ENTRY_TYPE_UNION,
    KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY,
    KEFIR_IR_DEBUG_ENTRY_TYPE_FUNCTION,
    KEFIR_IR_DEBUG_ENTRY_TYPE_BUILTIN,
    KEFIR_IR_DEBUG_ENTRY_ENUMERATOR,
    KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER,
    KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER,
    KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG
} kefir_ir_debug_entry_tag_t;

typedef enum kefir_ir_debug_entry_attribute_tag {
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME = 0,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CONSTANT_UINT,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET,
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE,
    // Auxillary
    KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_COUNT
} kefir_ir_debug_entry_attribute_tag_t;

typedef struct kefir_ir_debug_entry_attribute {
    kefir_ir_debug_entry_attribute_tag_t tag;

    union {
        const char *name;
        kefir_size_t size;
        kefir_size_t alignment;
        kefir_size_t length;
        kefir_uint64_t constant_uint;
        kefir_size_t offset;
        kefir_size_t bitwidth;
        kefir_size_t bitoffset;
        kefir_ir_debug_entry_id_t entry_id;
    };
} kefir_ir_debug_entry_attribute_t;

typedef struct kefir_ir_debug_entry {
    kefir_ir_debug_entry_id_t identifier;
    kefir_ir_debug_entry_id_t parent_identifier;
    kefir_ir_debug_entry_tag_t tag;
    struct kefir_hashtree attributes;
    struct kefir_list children;
} kefir_ir_debug_entry_t;

typedef struct kefir_ir_debug_entries {
    struct kefir_hashtree entries;
    kefir_ir_debug_entry_id_t next_entry_id;
} kefir_ir_debug_entries_t;

#define KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(_name) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, \
        .name = (_name) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(_size) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, \
        .size = (_size) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(_align) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, \
        .alignment = (_align) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_LENGTH(_length) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH, \
        .length = (_length) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_CONSTANT_UINT(_const_uint) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CONSTANT_UINT, \
        .constant_uint = (_const_uint) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(_offset) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET, \
        .offset = (_offset) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_BITOFFSET(_bitoffset) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET, \
        .bitoffset = (_bitoffset) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_BITWIDTH(_bitwidth) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH, \
        .bitwidth = (_bitwidth) \
    })
#define KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(_entry_id) ((struct kefir_ir_debug_entry_attribute) { \
        .tag = KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, \
        .entry_id = (_entry_id) \
    })

typedef struct kefir_ir_source_location {
    struct kefir_source_location location;
    kefir_size_t begin;
    kefir_size_t end;

    struct kefir_ir_source_location *next;
} kefir_ir_source_location_t;

typedef struct kefir_ir_source_map {
    struct kefir_interval_tree locations;
} kefir_ir_source_map_t;

typedef struct kefir_ir_source_map_iterator {
    struct kefir_interval_tree_iterator iter;
    struct kefir_ir_source_location *source_location;
} kefir_ir_source_map_iterator_t;

typedef struct kefir_ir_function_debug_info {
    struct kefir_ir_source_map source_map;
    struct kefir_source_location source_location;
} kefir_ir_function_debug_info_t;

typedef struct kefir_ir_module_debug_info {
    struct kefir_ir_debug_entries entries;
} kefir_ir_module_debug_info_t;

kefir_result_t kefir_ir_debug_entries_init(struct kefir_ir_debug_entries *);
kefir_result_t kefir_ir_debug_entries_free(struct kefir_mem *, struct kefir_ir_debug_entries *);

kefir_result_t kefir_ir_debug_entry_get(const struct kefir_ir_debug_entries *, kefir_ir_debug_entry_id_t, const struct kefir_ir_debug_entry **);
kefir_result_t kefir_ir_debug_entry_get_attribute(const struct kefir_ir_debug_entries *, kefir_ir_debug_entry_id_t, kefir_ir_debug_entry_attribute_tag_t, const struct kefir_ir_debug_entry_attribute **);

kefir_result_t kefir_ir_debug_entry_new(struct kefir_mem *, struct kefir_ir_debug_entries *, kefir_ir_debug_entry_tag_t, kefir_ir_debug_entry_id_t *);
kefir_result_t kefir_ir_debug_entry_new_child(struct kefir_mem *, struct kefir_ir_debug_entries *, kefir_ir_debug_entry_id_t, kefir_ir_debug_entry_tag_t, kefir_ir_debug_entry_id_t *);
kefir_result_t kefir_ir_debug_entry_add_attribute(struct kefir_mem *, struct kefir_ir_debug_entries *, struct kefir_string_pool *, kefir_ir_debug_entry_id_t, const struct kefir_ir_debug_entry_attribute *);

typedef struct kefir_ir_debug_entry_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_ir_debug_entry_iterator_t;

typedef struct kefir_ir_debug_entry_attribute_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_ir_debug_entry_attribute_iterator_t;

typedef struct kefir_ir_debug_entry_child_iterator {
    const struct kefir_list_entry *iter;
} kefir_ir_debug_entry_child_iterator_t;

kefir_result_t kefir_ir_debug_entry_iter(const struct kefir_ir_debug_entries *, struct kefir_ir_debug_entry_iterator *, kefir_ir_debug_entry_id_t *);
kefir_result_t kefir_ir_debug_entry_next(struct kefir_ir_debug_entry_iterator *, kefir_ir_debug_entry_id_t *);

kefir_result_t kefir_ir_debug_entry_attribute_iter(const struct kefir_ir_debug_entries *, kefir_ir_debug_entry_id_t, struct kefir_ir_debug_entry_attribute_iterator *, const struct kefir_ir_debug_entry_attribute **);
kefir_result_t kefir_ir_debug_entry_attribute_next(struct kefir_ir_debug_entry_attribute_iterator *, const struct kefir_ir_debug_entry_attribute **);

kefir_result_t kefir_ir_debug_entry_child_iter(const struct kefir_ir_debug_entries *, kefir_ir_debug_entry_id_t, struct kefir_ir_debug_entry_child_iterator *, kefir_ir_debug_entry_id_t *);
kefir_result_t kefir_ir_debug_entry_child_next(struct kefir_ir_debug_entry_child_iterator *, kefir_ir_debug_entry_id_t *);

kefir_result_t kefir_ir_source_map_init(struct kefir_ir_source_map *);
kefir_result_t kefir_ir_source_map_free(struct kefir_mem *, struct kefir_ir_source_map *);

kefir_result_t kefir_ir_source_map_insert(struct kefir_mem *, struct kefir_ir_source_map *, struct kefir_string_pool *,
                                          const struct kefir_source_location *, kefir_size_t, kefir_size_t);
kefir_result_t kefir_ir_source_map_find(const struct kefir_ir_source_map *, kefir_size_t,
                                        const struct kefir_ir_source_location **);

kefir_result_t kefir_ir_source_map_iter(const struct kefir_ir_source_map *, struct kefir_ir_source_map_iterator *,
                                        const struct kefir_ir_source_location **);
kefir_result_t kefir_ir_source_map_next(struct kefir_ir_source_map_iterator *,
                                        const struct kefir_ir_source_location **);

kefir_result_t kefir_ir_function_debug_info_init(struct kefir_ir_function_debug_info *);
kefir_result_t kefir_ir_function_debug_info_free(struct kefir_mem *, struct kefir_ir_function_debug_info *);

const struct kefir_source_location *kefir_ir_function_debug_info_source_location(
    const struct kefir_ir_function_debug_info *);
kefir_result_t kefir_ir_function_debug_info_set_source_location(struct kefir_mem *,
                                                                struct kefir_ir_function_debug_info *,
                                                                struct kefir_string_pool *,
                                                                const struct kefir_source_location *);

kefir_result_t kefir_ir_module_debug_info_init(struct kefir_ir_module_debug_info *);
kefir_result_t kefir_ir_module_debug_info_free(struct kefir_mem *, struct kefir_ir_module_debug_info *);

#endif
