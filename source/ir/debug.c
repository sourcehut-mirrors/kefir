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

#include "kefir/ir/debug.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_debug_entry(struct kefir_mem *mem, struct kefir_ir_debug_entry *entry) {
    REQUIRE_OK(kefir_hashtree_free(mem, &entry->attributes));
    REQUIRE_OK(kefir_list_free(mem, &entry->children));
    memset(entry, 0, sizeof(struct kefir_ir_debug_entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

static kefir_result_t free_root_debug_entry(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                            kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ir_debug_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entry"));

    REQUIRE_OK(free_debug_entry(mem, entry));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entries_init(struct kefir_ir_debug_entries *entries) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entries"));

    REQUIRE_OK(kefir_hashtree_init(&entries->entries, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&entries->entries, free_root_debug_entry, NULL));
    entries->next_entry_id = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entries_free(struct kefir_mem *mem, struct kefir_ir_debug_entries *entries) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));

    REQUIRE_OK(kefir_hashtree_free(mem, &entries->entries));
    memset(entries, 0, sizeof(struct kefir_ir_debug_entries));
    return KEFIR_OK;
}

static kefir_result_t debug_entry_get(const struct kefir_ir_debug_entries *entries,
                                      kefir_ir_debug_entry_id_t identifier, struct kefir_ir_debug_entry **entry_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(entry_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&entries->entries, (kefir_hashtree_key_t) identifier, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested IR debug entry");
    }
    REQUIRE_OK(res);

    *entry_ptr = (struct kefir_ir_debug_entry *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_get(const struct kefir_ir_debug_entries *entries,
                                        kefir_ir_debug_entry_id_t identifier,
                                        const struct kefir_ir_debug_entry **entry_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(entry_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, identifier, &entry));
    *entry_ptr = entry;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_get_attribute(const struct kefir_ir_debug_entries *entries,
                                                  kefir_ir_debug_entry_id_t identifier,
                                                  kefir_ir_debug_entry_attribute_tag_t attribute_tag,
                                                  const struct kefir_ir_debug_entry_attribute **attribute_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(attribute_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry attribute"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, identifier, &entry));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&entry->attributes, (kefir_hashtree_key_t) attribute_tag, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested IR debug entry attribute");
    }
    REQUIRE_OK(res);

    *attribute_ptr = (const struct kefir_ir_debug_entry_attribute *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_has_attribute(const struct kefir_ir_debug_entries *entries,
                                                  kefir_ir_debug_entry_id_t identifier,
                                                  kefir_ir_debug_entry_attribute_tag_t attribute_tag,
                                                  kefir_bool_t *has_attr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(has_attr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, identifier, &entry));

    *has_attr = kefir_hashtree_has(&entry->attributes, (kefir_hashtree_key_t) attribute_tag);
    return KEFIR_OK;
}

static kefir_result_t free_debug_entry_attribute(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                 kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                 void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ir_debug_entry_attribute *, attribute, value);
    REQUIRE(attribute != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entry attribute"));

    memset(attribute, 0, sizeof(struct kefir_ir_debug_entry_attribute));
    KEFIR_FREE(mem, attribute);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_new(struct kefir_mem *mem, struct kefir_ir_debug_entries *entries,
                                        kefir_ir_debug_entry_tag_t entry_tag, kefir_ir_debug_entry_id_t *entry_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(entry_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer tp IR debug entry identifier"));

    struct kefir_ir_debug_entry *entry = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_debug_entry));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR debug entry"));

    entry->parent_identifier = KEFIR_IR_DEBUG_ENTRY_ID_NONE;
    entry->identifier = entries->next_entry_id;
    entry->tag = entry_tag;

    kefir_result_t res = kefir_hashtree_init(&entry->attributes, &kefir_hashtree_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&entry->attributes, free_debug_entry_attribute, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&entry->children));
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &entries->entries, (kefir_hashtree_key_t) entry->identifier,
                                              (kefir_hashtree_value_t) entry));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return KEFIR_OK;
    });

    entries->next_entry_id++;
    *entry_id_ptr = entry->identifier;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_new_child(struct kefir_mem *mem, struct kefir_ir_debug_entries *entries,
                                              kefir_ir_debug_entry_id_t parent_entry_id,
                                              kefir_ir_debug_entry_tag_t entry_tag,
                                              kefir_ir_debug_entry_id_t *entry_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(entry_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer tp IR debug entry identifier"));

    struct kefir_ir_debug_entry *parent_entry;
    REQUIRE_OK(debug_entry_get(entries, parent_entry_id, &parent_entry));

    struct kefir_ir_debug_entry *child_entry = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_debug_entry));
    REQUIRE(child_entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR debug entry"));

    child_entry->parent_identifier = parent_entry_id;
    child_entry->identifier = entries->next_entry_id;
    child_entry->tag = entry_tag;

    kefir_result_t res = kefir_hashtree_init(&child_entry->attributes, &kefir_hashtree_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&child_entry->attributes, free_debug_entry_attribute, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&child_entry->children));
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &entries->entries, (kefir_hashtree_key_t) child_entry->identifier,
                                              (kefir_hashtree_value_t) child_entry));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, child_entry);
        return KEFIR_OK;
    });

    res = kefir_list_insert_after(mem, &parent_entry->children, kefir_list_tail(&parent_entry->children),
                                  (void *) child_entry);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_delete(mem, &entries->entries, (kefir_hashtree_key_t) child_entry->identifier);
        return res;
    });

    entries->next_entry_id++;
    *entry_id_ptr = child_entry->identifier;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_add_attribute(struct kefir_mem *mem, struct kefir_ir_debug_entries *entries,
                                                  struct kefir_string_pool *symbols, kefir_ir_debug_entry_id_t entry_id,
                                                  const struct kefir_ir_debug_entry_attribute *attribute) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(attribute != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entry attribute"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, entry_id, &entry));

    struct kefir_ir_debug_entry_attribute *attr = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_debug_entry_attribute));
    REQUIRE(attr != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR debug entry attribute"));

    *attr = *attribute;
    switch (attr->tag) {
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME:
            REQUIRE_ELSE(attr->name != NULL, {
                KEFIR_FREE(mem, attr);
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-NULL IR debug entry name attribute");
            });
            if (symbols != NULL) {
                attr->name = kefir_string_pool_insert(mem, symbols, attr->name, NULL);
                REQUIRE_ELSE(attr->name != NULL, {
                    KEFIR_FREE(mem, attr);
                    return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                           "Failed to insert IR debug entry name attribute into string pool");
                });
            }
            break;

        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE:
            REQUIRE_ELSE(kefir_hashtree_has(&entries->entries, (kefir_hashtree_key_t) attr->type_id), {
                KEFIR_FREE(mem, attr);
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                       "Expected valid IR debug entry entry identifier attribute");
            });
            break;

        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION:
            REQUIRE_ELSE(attr->source_location != NULL, {
                KEFIR_FREE(mem, attr);
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-NULL IR debug entry name attribute");
            });
            if (symbols != NULL) {
                attr->source_location = kefir_string_pool_insert(mem, symbols, attr->source_location, NULL);
                REQUIRE_ELSE(attr->source_location != NULL, {
                    KEFIR_FREE(mem, attr);
                    return KEFIR_SET_ERROR(
                        KEFIR_OBJALLOC_FAILURE,
                        "Failed to insert IR debug entry source location attribute into string pool");
                });
            }
            break;

        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CONSTANT_UINT:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_FUNCTION_PROTOTYPED_FLAG:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LOCAL_VARIABLE:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_GLOBAL_VARIABLE:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_THREAD_LOCAL_VARIABLE:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_PARAMETER:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_EXTERNAL:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_DECLARATION:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE:
        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN:
            // Intentionally left blank
            break;

        case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected IR debug entry attribute tag");
    }

    kefir_result_t res =
        kefir_hashtree_insert(mem, &entry->attributes, (kefir_hashtree_key_t) attr->tag, (kefir_hashtree_value_t) attr);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "IR debug entry already has attribute with identical tag");
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attr);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_iter(const struct kefir_ir_debug_entries *entries,
                                         struct kefir_ir_debug_entry_iterator *iter,
                                         kefir_ir_debug_entry_id_t *entry_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_iter(&entries->entries, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);

    for (;;) {
        ASSIGN_DECL_CAST(const struct kefir_ir_debug_entry *, entry, node->value);

        if (entry->parent_identifier == KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
            break;
        } else {
            node = kefir_hashtree_next(&iter->iter);
            REQUIRE(node != NULL, KEFIR_ITERATOR_END);
        }
    }

    ASSIGN_PTR(entry_ptr, (kefir_ir_debug_entry_id_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_next(struct kefir_ir_debug_entry_iterator *iter,
                                         kefir_ir_debug_entry_id_t *entry_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entry iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);

    for (;;) {
        ASSIGN_DECL_CAST(const struct kefir_ir_debug_entry *, entry, node->value);

        if (entry->parent_identifier == KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
            break;
        } else {
            node = kefir_hashtree_next(&iter->iter);
            REQUIRE(node != NULL, KEFIR_ITERATOR_END);
        }
    }

    ASSIGN_PTR(entry_ptr, (kefir_ir_debug_entry_id_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_attribute_iter(const struct kefir_ir_debug_entries *entries,
                                                   kefir_ir_debug_entry_id_t entry_id,
                                                   struct kefir_ir_debug_entry_attribute_iterator *iter,
                                                   const struct kefir_ir_debug_entry_attribute **attribute_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry attribute iterator"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, entry_id, &entry));

    struct kefir_hashtree_node *node = kefir_hashtree_iter(&entry->attributes, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);

    ASSIGN_PTR(attribute_ptr, (const struct kefir_ir_debug_entry_attribute *) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_attribute_next(struct kefir_ir_debug_entry_attribute_iterator *iter,
                                                   const struct kefir_ir_debug_entry_attribute **attribute_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entry attribute iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);

    ASSIGN_PTR(attribute_ptr, (const struct kefir_ir_debug_entry_attribute *) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_child_iter(const struct kefir_ir_debug_entries *entries,
                                               kefir_ir_debug_entry_id_t entry_id,
                                               struct kefir_ir_debug_entry_child_iterator *iter,
                                               kefir_ir_debug_entry_id_t *child_entry_id_ptr) {
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry attribute iterator"));

    struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(debug_entry_get(entries, entry_id, &entry));

    iter->iter = kefir_list_head(&entry->children);
    REQUIRE(iter->iter != NULL, KEFIR_ITERATOR_END);

    ASSIGN_DECL_CAST(const struct kefir_ir_debug_entry *, child_entry, iter->iter->value);
    ASSIGN_PTR(child_entry_id_ptr, child_entry->identifier);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_entry_child_next(struct kefir_ir_debug_entry_child_iterator *iter,
                                               kefir_ir_debug_entry_id_t *child_entry_id_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry attribute iterator"));

    kefir_list_next(&iter->iter);
    REQUIRE(iter->iter != NULL, KEFIR_ITERATOR_END);

    ASSIGN_DECL_CAST(const struct kefir_ir_debug_entry *, child_entry, iter->iter->value);
    ASSIGN_PTR(child_entry_id_ptr, child_entry->identifier);
    return KEFIR_OK;
}

static kefir_result_t on_source_location_remove(struct kefir_mem *mem, struct kefir_interval_tree *tree,
                                                kefir_interval_tree_key_t begin, kefir_interval_tree_key_t end,
                                                kefir_interval_tree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(begin);
    UNUSED(end);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ir_debug_source_location *, source_location, value);
    REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source location"));

    while (source_location != NULL) {
        struct kefir_ir_debug_source_location *next_location = source_location->next;
        memset(source_location, 0, sizeof(struct kefir_ir_debug_source_location));
        KEFIR_FREE(mem, source_location);
        source_location = next_location;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_init(struct kefir_ir_debug_function_source_map *source_map) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source map"));

    REQUIRE_OK(kefir_interval_tree_init(&source_map->locations));
    REQUIRE_OK(kefir_interval_tree_on_remove(&source_map->locations, on_source_location_remove, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_free(struct kefir_mem *mem,
                                                       struct kefir_ir_debug_function_source_map *source_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));

    REQUIRE_OK(kefir_interval_tree_free(mem, &source_map->locations));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_insert(struct kefir_mem *mem,
                                                         struct kefir_ir_debug_function_source_map *source_map,
                                                         struct kefir_string_pool *strings,
                                                         const struct kefir_source_location *source_location,
                                                         kefir_size_t begin, kefir_size_t end) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(source_location != NULL && source_location->source != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));
    REQUIRE(begin <= end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Source location IR range shall be non-negative"));

    struct kefir_ir_debug_source_location *target = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_debug_source_location));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR source location"));
    if (strings != NULL) {
        target->location.source = kefir_string_pool_insert(mem, strings, source_location->source, NULL);
        REQUIRE_ELSE(target->location.source != NULL, {
            KEFIR_FREE(mem, target);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                   "Failed to insert source location identifier into a string pool");
        });
    } else {
        target->location.source = source_location->source;
    }
    target->location.line = source_location->line;
    target->location.column = source_location->column;
    target->begin = begin;
    target->end = end;
    target->next = NULL;

    kefir_result_t res =
        kefir_interval_tree_insert(mem, &source_map->locations, (kefir_interval_tree_key_t) begin,
                                   (kefir_interval_tree_key_t) end, (kefir_interval_tree_value_t) target);
    if (res == KEFIR_ALREADY_EXISTS) {
        struct kefir_interval_tree_node *node = NULL;
        res = kefir_interval_tree_get(&source_map->locations, (kefir_interval_tree_key_t) begin,
                                      (kefir_interval_tree_key_t) end, &node);
        if (res == KEFIR_OK) {
            ASSIGN_DECL_CAST(struct kefir_ir_debug_source_location *, next_target, node->value);

            target->next = next_target;
            node->value = (kefir_interval_tree_value_t) target;
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, target);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_find(const struct kefir_ir_debug_function_source_map *source_map,
                                                       kefir_size_t positon,
                                                       const struct kefir_ir_debug_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(location_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source location"));

    struct kefir_interval_tree_finder finder;
    kefir_result_t res;
    struct kefir_interval_tree_node *node;
    const struct kefir_ir_debug_source_location *most_precise_location = NULL;
    for (res = kefir_interval_tree_find(&source_map->locations, (kefir_interval_tree_key_t) positon, &finder, &node);
         res == KEFIR_OK; res = kefir_interval_tree_find_next(&source_map->locations, &finder, &node)) {

        ASSIGN_DECL_CAST(const struct kefir_ir_debug_source_location *, source_location, node->value);
        for (; source_location != NULL; source_location = source_location->next) {
            REQUIRE(positon >= source_location->begin && positon < source_location->end,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected source location"));
#define WIDTH(_loc) ((_loc)->end - (_loc)->begin)
            if (most_precise_location == NULL || WIDTH(source_location) <= WIDTH(most_precise_location)) {
                most_precise_location = source_location;
            }
#undef WIDTH
        }
    }

    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(most_precise_location != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find source location for requested position"));
    *location_ptr = most_precise_location;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_iter(const struct kefir_ir_debug_function_source_map *source_map,
                                                       struct kefir_ir_debug_function_source_map_iterator *iter,
                                                       const struct kefir_ir_debug_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source map iterator"));

    struct kefir_interval_tree_node *node;
    REQUIRE_OK(kefir_interval_tree_iter(&source_map->locations, &iter->iter, &node));
    ASSIGN_DECL_CAST(const struct kefir_ir_debug_source_location *, source_location, node->value);
    iter->source_location = source_location->next;
    ASSIGN_PTR(location_ptr, source_location);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_debug_function_source_map_next(struct kefir_ir_debug_function_source_map_iterator *iter,
                                                       const struct kefir_ir_debug_source_location **location_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map iterator"));

    if (iter->source_location == NULL) {
        struct kefir_interval_tree_node *node;
        REQUIRE_OK(kefir_interval_tree_next(&iter->iter, &node));
        ASSIGN_DECL_CAST(const struct kefir_ir_debug_source_location *, source_location, node->value);
        iter->source_location = source_location->next;
        ASSIGN_PTR(location_ptr, source_location);
    } else {
        const struct kefir_ir_debug_source_location *source_location = iter->source_location;
        iter->source_location = source_location->next;
        ASSIGN_PTR(location_ptr, source_location);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_function_debug_info_init(struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR function debug info"));

    debug_info->source_location.source = NULL;
    debug_info->source_location.line = 0;
    debug_info->source_location.column = 0;
    debug_info->subprogram_id = KEFIR_IR_DEBUG_ENTRY_ID_NONE;
    REQUIRE_OK(kefir_ir_debug_function_source_map_init(&debug_info->source_map));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_function_debug_info_free(struct kefir_mem *mem,
                                                 struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function debug info"));

    REQUIRE_OK(kefir_ir_debug_function_source_map_free(mem, &debug_info->source_map));
    return KEFIR_OK;
}

const struct kefir_source_location *kefir_ir_function_debug_info_source_location(
    const struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(debug_info != NULL, NULL);
    REQUIRE(debug_info->source_location.source != NULL, NULL);
    return &debug_info->source_location;
}

kefir_result_t kefir_ir_function_debug_info_set_source_location(struct kefir_mem *mem,
                                                                struct kefir_ir_function_debug_info *debug_info,
                                                                struct kefir_string_pool *symbols,
                                                                const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function debug info"));

    if (location != NULL && location->source != NULL) {
        if (symbols != NULL) {
            debug_info->source_location.source = kefir_string_pool_insert(mem, symbols, location->source, NULL);
            REQUIRE(debug_info->source_location.source != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                    "Failed to insert IR function source location into string pool"));
        } else {
            debug_info->source_location.source = location->source;
        }
        debug_info->source_location.line = location->line;
        debug_info->source_location.column = location->column;
    } else {
        debug_info->source_location.source = NULL;
        debug_info->source_location.line = 0;
        debug_info->source_location.column = 0;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_debug_info_init(struct kefir_ir_module_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR module debug info"));

    REQUIRE_OK(kefir_ir_debug_entries_init(&debug_info->entries));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_debug_info_free(struct kefir_mem *mem, struct kefir_ir_module_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module debug info"));

    REQUIRE_OK(kefir_ir_debug_entries_free(mem, &debug_info->entries));
    return KEFIR_OK;
}
