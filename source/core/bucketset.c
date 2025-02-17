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

#include "kefir/core/bucketset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define BUCKET_SIZEOF(_capacity) (sizeof(struct kefir_bucketset_bucket) + (_capacity) * sizeof(kefir_bucketset_entry_t))
#define BUCKET_INIT_CAPACITY 16
#define BUCKET_MAX_LENGTH 1024
#define BUCKET_NEW_CAPACITY(_capacity) ((_capacity) + 16)

static kefir_result_t free_bucket(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                  kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, value);
    REQUIRE(bucket != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set bucket"));

    memset(bucket, 0, BUCKET_SIZEOF(bucket->capacity));
    KEFIR_FREE(mem, bucket);
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_init(struct kefir_bucketset *bucketset, const struct kefir_bucketset_ops *ops) {
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtree ops"));

    REQUIRE_OK(kefir_hashtree_init(&bucketset->buckets, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&bucketset->buckets, free_bucket, NULL));
    bucketset->num_of_buckets = 1;
    bucketset->max_bucket_length = 0;
    bucketset->ops = ops;
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_free(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    REQUIRE_OK(kefir_hashtree_free(mem, &bucketset->buckets));
    memset(bucketset, 0, sizeof(struct kefir_bucketset));
    return KEFIR_OK;
}

static kefir_result_t get_bucket(struct kefir_mem *mem, struct kefir_bucketset *bucketset, kefir_size_t index,
                                 struct kefir_bucketset_bucket **bucket_ptr) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&bucketset->buckets, (kefir_bucketset_entry_t) index, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);
        if (bucket->length == bucket->capacity) {
            const kefir_size_t bucket_new_capacity = BUCKET_NEW_CAPACITY(bucket->capacity);
            bucket = KEFIR_REALLOC(mem, bucket, BUCKET_SIZEOF(bucket_new_capacity));
            REQUIRE(bucket != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate bucket set bucket"));
            bucket->capacity = bucket_new_capacity;
            node->value = (kefir_hashtree_value_t) bucket;
        }
        *bucket_ptr = bucket;
    } else {
        const kefir_size_t init_capacity = BUCKET_INIT_CAPACITY;
        struct kefir_bucketset_bucket *bucket = KEFIR_MALLOC(mem, BUCKET_SIZEOF(init_capacity));
        REQUIRE(bucket != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bucket set bucket"));
        bucket->capacity = init_capacity;
        bucket->length = 0;

        kefir_result_t res = kefir_hashtree_insert(mem, &bucketset->buckets, (kefir_hashtree_key_t) index,
                                                   (kefir_hashtree_value_t) bucket);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, bucket);
            return res;
        });
        *bucket_ptr = bucket;
    }
    return KEFIR_OK;
}

static kefir_result_t insert_entry(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                   kefir_bucketset_entry_t entry) {
    const kefir_hashtree_hash_t hash = bucketset->ops->hash(entry, bucketset->ops->payload);
    const kefir_size_t index = hash % bucketset->num_of_buckets;
    struct kefir_bucketset_bucket *bucket = NULL;
    REQUIRE_OK(get_bucket(mem, bucketset, index, &bucket));

    kefir_size_t insert_idx;
    kefir_result_t res =
        bucketset->ops->insert_position(bucket->entries, bucket->length, entry, &insert_idx, bucketset->ops->payload);
    REQUIRE(res != KEFIR_ALREADY_EXISTS, KEFIR_OK);
    REQUIRE_OK(res);

    const kefir_size_t move_elements = bucket->length - insert_idx;
    if (move_elements > 0) {
        memmove(&bucket->entries[insert_idx + 1], &bucket->entries[insert_idx],
                sizeof(kefir_bucketset_entry_t) * move_elements);
    }
    bucket->entries[insert_idx] = entry;
    bucket->length++;
    bucketset->max_bucket_length = MAX(bucketset->max_bucket_length, bucket->length);
    return KEFIR_OK;
}

static kefir_result_t fill_bucketset(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                     struct kefir_hashtree *old_buckets) {
    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(old_buckets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);
        for (kefir_size_t i = 0; i < bucket->length; i++) {
            REQUIRE_OK(insert_entry(mem, bucketset, bucket->entries[i]));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_add(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                   kefir_bucketset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));
    REQUIRE(!kefir_bucketset_has(bucketset, entry), KEFIR_OK);

    if (bucketset->max_bucket_length > BUCKET_MAX_LENGTH) {
        struct kefir_hashtree old_buckets = bucketset->buckets;
        const kefir_size_t old_num_of_buckets = bucketset->num_of_buckets;
        const kefir_size_t old_max_bucket_length = bucketset->max_bucket_length;

        bucketset->max_bucket_length = 0;
        bucketset->num_of_buckets++;
        kefir_result_t res = kefir_hashtree_init(&bucketset->buckets, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&bucketset->buckets, free_bucket, NULL));
        REQUIRE_CHAIN(&res, fill_bucketset(mem, bucketset, &old_buckets));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_free(mem, &bucketset->buckets);
            bucketset->buckets = old_buckets;
            bucketset->max_bucket_length = old_max_bucket_length;
            bucketset->num_of_buckets = old_num_of_buckets;
            return res;
        });

        REQUIRE_OK(kefir_hashtree_free(mem, &old_buckets));
    }

    REQUIRE_OK(insert_entry(mem, bucketset, entry));
    return KEFIR_OK;
}

kefir_bool_t kefir_bucketset_has(const struct kefir_bucketset *bucketset, kefir_bucketset_entry_t entry) {
    REQUIRE(bucketset != NULL, false);

    const kefir_hashtree_hash_t hash = bucketset->ops->hash(entry, bucketset->ops->payload);
    const kefir_size_t index = hash % bucketset->num_of_buckets;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&bucketset->buckets, (kefir_bucketset_entry_t) index, &node);
    REQUIRE(res == KEFIR_OK, false);
    ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);

    return bucketset->ops->contains(bucket->entries, bucket->length, entry, bucketset->ops->payload);
}

kefir_result_t kefir_bucketset_delete(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                      kefir_bucketset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    const kefir_hashtree_hash_t hash = bucketset->ops->hash(entry, bucketset->ops->payload);
    const kefir_size_t index = hash % bucketset->num_of_buckets;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&bucketset->buckets, (kefir_bucketset_entry_t) index, &node);
    REQUIRE(res == KEFIR_OK, false);
    ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);

    kefir_size_t delete_offset = 0;
    res = bucketset->ops->find(bucket->entries, bucket->length, entry, &delete_offset, bucketset->ops->payload);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    if (delete_offset + 1 < bucket->length) {
        memmove(&bucket->entries[delete_offset], &bucket->entries[delete_offset + 1],
                sizeof(kefir_bucketset_entry_t) * (bucket->length - 1 - delete_offset));
    }
    bucket->length--;

    if (bucket->length == 0) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &bucketset->buckets, (kefir_hashtree_key_t) index));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_clean(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    REQUIRE_OK(kefir_hashtree_clean(mem, &bucketset->buckets));
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_clean_nofree(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&bucketset->buckets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);
        bucket->length = 0;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_merge(struct kefir_mem *mem, struct kefir_bucketset *target,
                                     const struct kefir_bucketset *source) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target bucket set"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source bucket set"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&source->buckets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_bucketset_bucket *, bucket, node->value);
        for (kefir_size_t i = 0; i < bucket->length; i++) {
            REQUIRE_OK(kefir_bucketset_add(mem, target, bucket->entries[i]));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_intersect(struct kefir_mem *mem, struct kefir_bucketset *target,
                                         const struct kefir_bucketset *source) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target bucket set"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source bucket set"));

    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&target->buckets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_bucketset_bucket *, bucket, node->value);
        struct kefir_bucketset_bucket *new_bucket = KEFIR_MALLOC(mem, BUCKET_SIZEOF(bucket->capacity));
        REQUIRE(new_bucket != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate new bucketset bucket"));
        new_bucket->capacity = bucket->capacity;
        new_bucket->length = 0;

        for (kefir_size_t i = 0; i < bucket->length; i++) {
            if (kefir_bucketset_has(source, bucket->entries[i])) {
                new_bucket->entries[new_bucket->length++] = bucket->entries[i];
            }
        }
        KEFIR_FREE(mem, bucket);
        node->value = (kefir_hashtree_value_t) new_bucket;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_iter(const struct kefir_bucketset *bucketset, struct kefir_bucketset_iterator *iter,
                                    kefir_bucketset_entry_t *entry_ptr) {
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set iterator"));

    REQUIRE_OK(kefir_hashtree_min(&bucketset->buckets, &iter->node));
    const struct kefir_bucketset_bucket *bucket = NULL;
    for (; iter->node != NULL;) {
        bucket = (const struct kefir_bucketset_bucket *) iter->node->value;
        if (bucket->length > 0) {
            break;
        } else {
            iter->node = kefir_hashtree_next_node(&bucketset->buckets, iter->node);
        }
    }
    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
    iter->bucketset = bucketset;
    iter->index = 0;
    ASSIGN_PTR(entry_ptr, bucket->entries[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_next(struct kefir_bucketset_iterator *iter, kefir_bucketset_entry_t *entry_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set iterator"));
    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);

    ASSIGN_DECL_CAST(const struct kefir_bucketset_bucket *, bucket, iter->node->value);
    if (iter->index + 1 == bucket->length) {
        iter->node = kefir_hashtree_next_node(&iter->bucketset->buckets, iter->node);
        for (; iter->node != NULL;) {
            bucket = (const struct kefir_bucketset_bucket *) iter->node->value;
            if (bucket->length > 0) {
                break;
            } else {
                iter->node = kefir_hashtree_next_node(&iter->bucketset->buckets, iter->node);
            }
        }
        REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
        iter->index = 0;
    } else {
        iter->index++;
    }
    ASSIGN_PTR(entry_ptr, bucket->entries[iter->index]);
    return KEFIR_OK;
}

static kefir_bucketset_hash_t uint_hash(kefir_bucketset_entry_t entry, void *payload) {
    UNUSED(payload);
    return (kefir_hashtree_hash_t) entry;
}

static const kefir_bucketset_entry_t *uint_binary_search(const kefir_bucketset_entry_t *entries, kefir_size_t length,
                                                         kefir_bucketset_entry_t entry) {
    if (length == 0) {
        return NULL;
    } else if (length == 1) {
        return entries;
    } else {
        const kefir_size_t middle_index = length / 2;
        const kefir_bucketset_entry_t middle_entry = entries[middle_index];
        if (entry < middle_entry) {
            return uint_binary_search(entries, middle_index, entry);
        } else if (entry == middle_entry) {
            return &entries[middle_index];
        } else {
            return uint_binary_search(&entries[middle_index], length - middle_index, entry);
        }
    }
}

static kefir_bool_t uint_contains(const kefir_bucketset_entry_t *entries, kefir_size_t length,
                                  kefir_bucketset_entry_t entry, void *payload) {
    UNUSED(payload);
    REQUIRE(entries != NULL, false);
    REQUIRE(length > 0, false);
    UNUSED(uint_binary_search);

    const kefir_bucketset_entry_t *found_entry = uint_binary_search(entries, length, entry);
    REQUIRE(found_entry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected binary search result"));
    return *found_entry == entry;
}

static kefir_result_t uint_find(const kefir_bucketset_entry_t *entries, kefir_size_t length,
                                kefir_bucketset_entry_t entry, kefir_size_t *position_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket entries"));
    REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested bucketset entry"));
    REQUIRE(position_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to array index"));

    const kefir_bucketset_entry_t *found_entry = uint_binary_search(entries, length, entry);
    REQUIRE(found_entry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected binary search result"));
    REQUIRE(*found_entry == entry, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested bucketset entry"));
    *position_ptr = found_entry - entries;
    return KEFIR_OK;
}

static kefir_result_t uint_insert_position(const kefir_bucketset_entry_t *entries, kefir_size_t length,
                                           kefir_bucketset_entry_t entry, kefir_size_t *position_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket entries"));
    REQUIRE(position_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to array index"));
    if (length == 0) {
        *position_ptr = 0;
        return KEFIR_OK;
    }

    const kefir_bucketset_entry_t *found_entry = uint_binary_search(entries, length, entry);
    REQUIRE(found_entry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected binary search result"));
    if (*found_entry == entry) {
        return KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Identical entry already exists in the bucket");
    } else if (*found_entry < entry) {
        *position_ptr = (found_entry - entries) + 1;
    } else {
        *position_ptr = found_entry - entries;
    }
    return KEFIR_OK;
}

const struct kefir_bucketset_ops kefir_bucketset_uint_ops = {.hash = uint_hash,
                                                             .contains = uint_contains,
                                                             .find = uint_find,
                                                             .insert_position = uint_insert_position,
                                                             .payload = NULL};
