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

#include "kefir/codegen/asmcmp/debug.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct source_map_entry {
    kefir_asmcmp_instruction_index_t begin;
    kefir_asmcmp_instruction_index_t end;
    struct kefir_source_location source_location;
};

static kefir_result_t free_source_map_entry(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                                    kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                                    void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct source_map_entry *, entry,
        value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map entry"));

    memset(entry, 0, sizeof(struct source_map_entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_source_map_init(struct kefir_asmcmp_debug_info_source_map *source_map) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp source map"));

    REQUIRE_OK(kefir_hashtree_init(&source_map->map, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&source_map->map, free_source_map_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_source_map_free(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_source_map *source_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));

    REQUIRE_OK(kefir_hashtree_free(mem, &source_map->map));
    memset(source_map, 0, sizeof(struct kefir_asmcmp_debug_info_source_map));
    return KEFIR_OK;
}

static kefir_result_t free_definition_entry(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_debug_info_code_map_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info definition entry"));
    
    KEFIR_FREE(mem, entry->fragments);
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_code_map_init(struct kefir_asmcmp_debug_info_code_map *code_map) {
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info code map"));

    REQUIRE_OK(kefir_hashtable_init(&code_map->fragments, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&code_map->fragments, free_definition_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_code_map_free(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_code_map *code_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map"));

    REQUIRE_OK(kefir_hashtable_free(mem, &code_map->fragments));
    return KEFIR_OK;
}

static kefir_result_t free_value_entry(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_debug_info_value_map_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value map entry"));
    
    KEFIR_FREE(mem, entry->fragments);
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_value_map_init(struct kefir_asmcmp_debug_info_value_map *value_map) {
    REQUIRE(value_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info value map"));

    REQUIRE_OK(kefir_hashtable_init(&value_map->fragments, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&value_map->fragments, free_value_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_value_map_free(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_value_map *value_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(value_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value map"));

    REQUIRE_OK(kefir_hashtable_free(mem, &value_map->fragments));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_init(struct kefir_asmcmp_debug_info *debug_info) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info"));

    REQUIRE_OK(kefir_asmcmp_debug_info_source_map_init(&debug_info->source_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_code_map_init(&debug_info->code_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_value_map_init(&debug_info->value_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_free(struct kefir_mem *mem, struct kefir_asmcmp_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info"));

    REQUIRE_OK(kefir_asmcmp_debug_info_source_map_free(mem, &debug_info->source_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_code_map_free(mem, &debug_info->code_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_value_map_free(mem, &debug_info->value_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_source_map_add_location(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_source_map *source_map, struct kefir_string_pool *symbols, kefir_asmcmp_instruction_index_t begin, kefir_asmcmp_instruction_index_t end, const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));
    REQUIRE(begin != end, KEFIR_OK);
    REQUIRE(begin < end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Asmcmp source map begin index shall preceede end index"));
    REQUIRE(location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) begin, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct source_map_entry *, entry,
            node->value);
        REQUIRE(begin >= entry->end, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Overlapping asmcmp index ranges in location assignment"));
    }

    res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) end, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct source_map_entry *, entry,
            node->value);
        REQUIRE(begin >= entry->end, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Overlapping asmcmp index ranges in location assignment"));
    }

    struct source_map_entry *entry = KEFIR_MALLOC(mem, sizeof(struct source_map_entry));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp source map entry"));

    if (symbols != NULL) {
        entry->source_location.source = kefir_string_pool_insert(mem, symbols, location->source, NULL);
        REQUIRE_ELSE(entry->source_location.source != NULL, {
            KEFIR_FREE(mem, entry);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert source location into string pool");
        });
    } else {
        entry->source_location.source = location->source;
    }
    entry->source_location.line = location->line;
    entry->source_location.column = location->column;
    entry->begin = begin;
    entry->end = end;

    res = kefir_hashtree_insert(mem, &source_map->map, (kefir_hashtree_key_t) begin, (kefir_hashtree_value_t) entry);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_source_map_at(const struct kefir_asmcmp_debug_info_source_map *source_map, kefir_asmcmp_instruction_index_t index, const struct kefir_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));
    REQUIRE(location_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer source location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {   
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested location in asmcmp source map");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct source_map_entry *, entry,
        node->value);
    REQUIRE(index >= entry->begin && index < entry->end,
        KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested location in asmcmp source map"));
    *location_ptr = &entry->source_location;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_add_fragment(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_code_map *code_map,
    kefir_asmcmp_debug_info_code_reference_t code_reference,
    kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map"));
    REQUIRE(begin_label != KEFIR_ASMCMP_INDEX_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp label index"));
    REQUIRE(end_label != KEFIR_ASMCMP_INDEX_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp label index"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code_map->fragments, (kefir_hashtable_key_t) code_reference, &table_value);
    struct kefir_asmcmp_debug_info_code_map_entry *entry = NULL;
    if (res == KEFIR_NOT_FOUND) {
        entry = KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_debug_info_code_map_entry));
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp debug info definition entry"));
        entry->fragments = NULL;
        entry->fragments_capacity = 0;
        entry->fragments_length = 0;
        res = kefir_hashtable_insert(mem, &code_map->fragments, (kefir_hashtable_key_t) code_reference, (kefir_hashtable_value_t) entry);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        entry = (struct kefir_asmcmp_debug_info_code_map_entry *) table_value;
    }

    if (entry->fragments_length >= entry->fragments_capacity) {
        kefir_size_t new_capacity = MAX(entry->fragments_capacity * 2, 4);
        struct kefir_asmcmp_debug_info_code_fragment *new_fragments = KEFIR_REALLOC(mem, entry->fragments, sizeof(struct kefir_asmcmp_debug_info_code_fragment) * new_capacity);
        REQUIRE(new_fragments != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp debug info definition fragment"));
        entry->fragments = new_fragments;
        entry->fragments_capacity = new_capacity;
    }

    entry->fragments[entry->fragments_length].begin_label = begin_label;
    entry->fragments[entry->fragments_length].end_label = end_label;
    entry->fragments_length++;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_value_map_add_fragment(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_value_map *value_map,
    kefir_asmcmp_debug_info_value_reference_t value_reference, kefir_asmcmp_debug_info_value_location_reference_t location_ref,
    kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(value_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value map"));
    REQUIRE(begin_label != KEFIR_ASMCMP_INDEX_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp label index"));
    REQUIRE(end_label != KEFIR_ASMCMP_INDEX_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp label index"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&value_map->fragments, (kefir_hashtable_key_t) value_reference, &table_value);
    struct kefir_asmcmp_debug_info_value_map_entry *entry = NULL;
    if (res == KEFIR_NOT_FOUND) {
        entry = KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_debug_info_value_map_entry));
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp debug info value map entry"));
        entry->fragments = NULL;
        entry->fragments_capacity = 0;
        entry->fragments_length = 0;
        res = kefir_hashtable_insert(mem, &value_map->fragments, (kefir_hashtable_key_t) value_reference, (kefir_hashtable_value_t) entry);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        entry = (struct kefir_asmcmp_debug_info_value_map_entry *) table_value;
    }

    if (entry->fragments_length >= entry->fragments_capacity) {
        kefir_size_t new_capacity = MAX(entry->fragments_capacity * 2, 4);
        struct kefir_asmcmp_debug_info_value_fragment *new_fragments = KEFIR_REALLOC(mem, entry->fragments, sizeof(struct kefir_asmcmp_debug_info_value_fragment) * new_capacity);
        REQUIRE(new_fragments != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp debug info value map fragment"));
        entry->fragments = new_fragments;
        entry->fragments_capacity = new_capacity;
    }

    entry->fragments[entry->fragments_length].location_ref = location_ref;
    entry->fragments[entry->fragments_length].begin_label = begin_label;
    entry->fragments[entry->fragments_length].end_label = end_label;
    entry->fragments_length++;
    return KEFIR_OK;
}

static kefir_result_t coalesce_code_map_entry(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_code_map_entry *entry,
    struct kefir_hashtree *fragment_tree) {

    for (kefir_size_t i = 0; i < entry->fragments_length; i++) {
        struct kefir_asmcmp_debug_info_code_fragment *fragment = &entry->fragments[i];
        if (fragment->begin_label == fragment->end_label) {
            continue;
        }
        
        kefir_hashtree_key_t key = (((kefir_uint64_t) fragment->begin_label) << 32) | (kefir_uint32_t) fragment->end_label;
        kefir_result_t res = kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) 0);
        if (res == KEFIR_ALREADY_EXISTS) {
            continue;
        }
        REQUIRE_OK(res);
    }

    kefir_bool_t reached_fixpoint = false;
    for (; !reached_fixpoint;) {
        reached_fixpoint = true;

        struct kefir_hashtree_node_iterator tree_iter;
        struct kefir_hashtree_node *next_node = NULL; 
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(fragment_tree, &tree_iter);
            node != NULL;
            node = next_node) {
            next_node = kefir_hashtree_next(&tree_iter);
            kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
            kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

            struct kefir_hashtree_node *other_node;
            kefir_result_t res = kefir_hashtree_lower_bound(fragment_tree, (kefir_hashtree_key_t) (((kefir_uint64_t) end_label) << 32), &other_node);
            if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(kefir_hashtree_min(fragment_tree, &other_node));
            } else {
                REQUIRE_OK(res);
                other_node = kefir_hashtree_next_node(fragment_tree, other_node);
            }

            if (other_node == NULL) {
                continue;
            }

            kefir_asmcmp_label_index_t other_begin_label = ((kefir_uint64_t) other_node->key) >> 32;
            kefir_asmcmp_label_index_t other_end_label = (kefir_uint32_t) other_node->key;

            if (other_begin_label == end_label) {
                if (next_node != NULL && next_node->key == other_node->key) {
                    next_node = kefir_hashtree_next(&tree_iter);
                }

                reached_fixpoint = false;
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, node->key));
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, other_node->key));

                kefir_hashtree_key_t key = (((kefir_uint64_t) begin_label) << 32) | (kefir_uint32_t) other_end_label;
                res = kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) 0);
                if (res == KEFIR_ALREADY_EXISTS) {
                    continue;
                }
                REQUIRE_OK(res);
            }
        }
    }

    entry->fragments_length = 0;
    struct kefir_hashtree_node_iterator tree_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(fragment_tree, &tree_iter);
        node != NULL;
        node = kefir_hashtree_next(&tree_iter)) {
        kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
        kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;
        entry->fragments[entry->fragments_length].begin_label = begin_label;
        entry->fragments[entry->fragments_length].end_label = end_label;
        entry->fragments_length++;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_coalesce(struct kefir_mem *mem, struct kefir_asmcmp_debug_info_code_map *code_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map"));

    kefir_result_t res;
    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
    struct kefir_hashtable_iterator iter;
    for (res = kefir_hashtable_iter(&code_map->fragments, &iter, &key, &value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&iter, &key, &value)) {
        ASSIGN_DECL_CAST(struct kefir_asmcmp_debug_info_code_map_entry *, entry,
            value);

        struct kefir_hashtree fragment_tree;
        REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

        kefir_result_t res = coalesce_code_map_entry(mem, entry, &fragment_tree);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_free(mem, &fragment_tree);
            return res;
        });
        REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_iter(const struct kefir_asmcmp_debug_info_code_map *code_map, struct kefir_asmcmp_code_map_iterator *iter, kefir_asmcmp_debug_info_code_reference_t *code_ref_ptr) {
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info code map iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_iter(&code_map->fragments, &iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug info code map iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(code_ref_ptr, key);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_next(struct kefir_asmcmp_code_map_iterator *iter, kefir_asmcmp_debug_info_code_reference_t *code_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug info code map iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(code_ref_ptr, key);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_fragment_iter(const struct kefir_asmcmp_debug_info_code_map *code_map, kefir_asmcmp_debug_info_code_reference_t code_ref, struct kefir_asmcmp_code_map_fragment_iterator *iter, const struct kefir_asmcmp_debug_info_code_fragment **fragment_ptr) {
    REQUIRE(code_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info code fragment iterator"));

    kefir_hashtree_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code_map->fragments, (kefir_hashtable_key_t) code_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp debug information value map entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_asmcmp_debug_info_code_map_entry *, entry, table_value);
    iter->entry = entry;
    iter->index = 0;
    REQUIRE(iter->index < entry->fragments_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug information code fragments"));
    ASSIGN_PTR(fragment_ptr, &iter->entry->fragments[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_code_map_fragment_next(struct kefir_asmcmp_code_map_fragment_iterator *iter, const struct kefir_asmcmp_debug_info_code_fragment **fragment_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code fragment iterator"));

    iter->index++;
    REQUIRE(iter->index < iter->entry->fragments_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug information code fragments"));
    ASSIGN_PTR(fragment_ptr, &iter->entry->fragments[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_value_map_iter(const struct kefir_asmcmp_debug_info_value_map *value_map, struct kefir_asmcmp_value_map_iterator *iter, kefir_asmcmp_debug_info_value_reference_t *code_ref_ptr) {
    REQUIRE(value_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info value map iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_iter(&value_map->fragments, &iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug info value map iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(code_ref_ptr, key);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_value_map_next(struct kefir_asmcmp_value_map_iterator *iter, kefir_asmcmp_debug_info_value_reference_t *code_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info code map iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug info code map iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(code_ref_ptr, key);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_value_map_fragment_iter(const struct kefir_asmcmp_debug_info_value_map *value_map, kefir_asmcmp_debug_info_value_reference_t value_ref, struct kefir_asmcmp_value_map_fragment_iterator *iter, const struct kefir_asmcmp_debug_info_value_fragment **fragment_ptr) {
    REQUIRE(value_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info value fragment iterator"));

    kefir_hashtree_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&value_map->fragments, (kefir_hashtable_key_t) value_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp debug information value map entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_asmcmp_debug_info_value_map_entry *, entry, table_value);
    iter->entry = entry;
    iter->index = 0;
    REQUIRE(iter->index < entry->fragments_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug information value fragments"));
    ASSIGN_PTR(fragment_ptr, &iter->entry->fragments[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_value_map_fragment_next(struct kefir_asmcmp_value_map_fragment_iterator *iter, const struct kefir_asmcmp_debug_info_value_fragment **fragment_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info value fragment iterator"));

    iter->index++;
    REQUIRE(iter->index < iter->entry->fragments_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of asmcmp debug information value fragments"));
    ASSIGN_PTR(fragment_ptr, &iter->entry->fragments[iter->index]);
    return KEFIR_OK;
}
