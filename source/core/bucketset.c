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
#include "kefir/core/hash.h"
#include <string.h>

#define BUCKET_SIZEOF(_capacity) (sizeof(struct kefir_bucketset_bucket) + (_capacity) * sizeof(kefir_bucketset_entry_t))
#define BUCKET_INIT_CAPACITY 16
#define BUCKET_MAX_LENGTH 1024
#define BUCKET_NEW_CAPACITY(_capacity) ((_capacity) + 16)

kefir_result_t kefir_bucketset_init(struct kefir_bucketset *bucketset, const struct kefir_bucketset_ops *ops) {
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtree ops"));

    bucketset->bucket_arr = NULL;
    bucketset->num_of_buckets = 0;
    bucketset->max_bucket_length = 0;
    bucketset->ops = ops;
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_free(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    for (kefir_size_t i = 0; i < bucketset->num_of_buckets; i++) {
        if (bucketset->bucket_arr[i] != NULL) {
            KEFIR_FREE(mem, bucketset->bucket_arr[i]);
        }
    }
    KEFIR_FREE(mem, bucketset->bucket_arr);
    memset(bucketset, 0, sizeof(struct kefir_bucketset));
    return KEFIR_OK;
}

static kefir_result_t get_bucket(struct kefir_mem *mem, struct kefir_bucketset *bucketset, kefir_size_t index,
                                 struct kefir_bucketset_bucket **bucket_ptr) {
    REQUIRE(index < bucketset->num_of_buckets, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested bucket is outside of bucketset range"));
    
    struct kefir_bucketset_bucket *bucket = bucketset->bucket_arr[index];
    if (bucket != NULL) {
        if (bucket->length == bucket->capacity) {
            const kefir_size_t bucket_new_capacity = BUCKET_NEW_CAPACITY(bucket->capacity);
            bucket = KEFIR_REALLOC(mem, bucket, BUCKET_SIZEOF(bucket_new_capacity));
            REQUIRE(bucket != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate bucket set bucket"));
            bucket->capacity = bucket_new_capacity;
            bucketset->bucket_arr[index] = bucket;
        }
        *bucket_ptr = bucket;
    } else {
        const kefir_size_t init_capacity = BUCKET_INIT_CAPACITY;
        struct kefir_bucketset_bucket *bucket = KEFIR_MALLOC(mem, BUCKET_SIZEOF(init_capacity));
        REQUIRE(bucket != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bucket set bucket"));
        bucket->capacity = init_capacity;
        bucket->length = 0;

        bucketset->bucket_arr[index] = bucket;
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
                                     struct kefir_bucketset_bucket **old_buckets, kefir_size_t old_num_of_buckets) {
    for (kefir_size_t i = 0; i < old_num_of_buckets; i++) {
        struct kefir_bucketset_bucket *bucket = old_buckets[i];
        if (bucket != NULL) {
            for (kefir_size_t i = 0; i < bucket->length; i++) {
                REQUIRE_OK(insert_entry(mem, bucketset, bucket->entries[i]));
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_add(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                   kefir_bucketset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));
    REQUIRE(!kefir_bucketset_has(bucketset, entry), KEFIR_OK);

    if (bucketset->max_bucket_length > BUCKET_MAX_LENGTH || bucketset->num_of_buckets == 0) {
        struct kefir_bucketset_bucket **old_buckets = bucketset->bucket_arr;
        const kefir_size_t old_num_of_buckets = bucketset->num_of_buckets;
        const kefir_size_t old_max_bucket_length = bucketset->max_bucket_length;

        bucketset->max_bucket_length = 0;
        bucketset->num_of_buckets++;
        bucketset->bucket_arr = KEFIR_MALLOC(mem, sizeof(struct kefir_bucketset_bucket *) * bucketset->num_of_buckets);
        memset(bucketset->bucket_arr, 0, sizeof(struct kefir_bucketset_bucket *) * bucketset->num_of_buckets);
        kefir_result_t res = fill_bucketset(mem, bucketset, old_buckets, old_num_of_buckets);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, bucketset->bucket_arr);
            bucketset->bucket_arr = old_buckets;
            bucketset->max_bucket_length = old_max_bucket_length;
            bucketset->num_of_buckets = old_num_of_buckets;
            return res;
        });

        KEFIR_FREE(mem, old_buckets);
    }

    REQUIRE_OK(insert_entry(mem, bucketset, entry));
    return KEFIR_OK;
}

kefir_bool_t kefir_bucketset_has(const struct kefir_bucketset *bucketset, kefir_bucketset_entry_t entry) {
    REQUIRE(bucketset != NULL, false);
    REQUIRE(bucketset->num_of_buckets > 0, false);

    const kefir_hashtree_hash_t hash = bucketset->ops->hash(entry, bucketset->ops->payload);
    const kefir_size_t index = hash % bucketset->num_of_buckets;
    struct kefir_bucketset_bucket *bucket = bucketset->bucket_arr[index];

    return bucket != NULL && bucketset->ops->contains(bucket->entries, bucket->length, entry, bucketset->ops->payload);
}

kefir_result_t kefir_bucketset_delete(struct kefir_mem *mem, struct kefir_bucketset *bucketset,
                                      kefir_bucketset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));
    REQUIRE(bucketset->num_of_buckets > 0, KEFIR_OK);

    const kefir_hashtree_hash_t hash = bucketset->ops->hash(entry, bucketset->ops->payload);
    const kefir_size_t index = hash % bucketset->num_of_buckets;
    struct kefir_bucketset_bucket *bucket = bucketset->bucket_arr[index];
    REQUIRE(bucket != NULL, KEFIR_OK);

    kefir_size_t delete_offset = 0;
    kefir_result_t res = bucketset->ops->find(bucket->entries, bucket->length, entry, &delete_offset, bucketset->ops->payload);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    if (delete_offset + 1 < bucket->length) {
        memmove(&bucket->entries[delete_offset], &bucket->entries[delete_offset + 1],
                sizeof(kefir_bucketset_entry_t) * (bucket->length - 1 - delete_offset));
    }
    bucket->length--;

    if (bucket->length == 0) {
        KEFIR_FREE(mem, bucketset->bucket_arr[index]);
        bucketset->bucket_arr[index] = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_clean(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    for (kefir_size_t i = 0; i < bucketset->num_of_buckets; i++) {
        KEFIR_FREE(mem, bucketset->bucket_arr[i]);
    }
    KEFIR_FREE(mem, bucketset->bucket_arr);
    bucketset->num_of_buckets = 0;
    bucketset->max_bucket_length = 0;
    bucketset->bucket_arr = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_clean_nofree(struct kefir_mem *mem, struct kefir_bucketset *bucketset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));

    for (kefir_size_t i = 0; i < bucketset->num_of_buckets; i++) {
        struct kefir_bucketset_bucket *bucket = bucketset->bucket_arr[i];
        if (bucket != NULL) {
            bucket->length = 0;
        }
    }
    bucketset->max_bucket_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_merge(struct kefir_mem *mem, struct kefir_bucketset *target,
                                     const struct kefir_bucketset *source) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target bucket set"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source bucket set"));

    for (kefir_size_t i = 0; i < source->num_of_buckets; i++) {
        struct kefir_bucketset_bucket *bucket = source->bucket_arr[i];
        if (bucket != NULL) {
            for (kefir_size_t i = 0; i < bucket->length; i++) {
                REQUIRE_OK(kefir_bucketset_add(mem, target, bucket->entries[i]));
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_intersect(struct kefir_mem *mem, struct kefir_bucketset *target,
                                         const struct kefir_bucketset *source) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target bucket set"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source bucket set"));

    for (kefir_size_t i = 0; i < target->num_of_buckets; i++) {
        struct kefir_bucketset_bucket *bucket = target->bucket_arr[i];
        if (bucket == NULL) {
            continue;
        }

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
        target->bucket_arr[i] = new_bucket;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_iter(const struct kefir_bucketset *bucketset, struct kefir_bucketset_iterator *iter,
                                    kefir_bucketset_entry_t *entry_ptr) {
    REQUIRE(bucketset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid bucket set"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set iterator"));

    iter->bucket_index = 0;
    for (; iter->bucket_index < bucketset->num_of_buckets && bucketset->bucket_arr[iter->bucket_index] == NULL; iter->bucket_index++) {}
    REQUIRE(iter->bucket_index < bucketset->num_of_buckets, KEFIR_ITERATOR_END);
    iter->bucketset = bucketset;
    iter->index = 0;
    ASSIGN_PTR(entry_ptr, bucketset->bucket_arr[iter->bucket_index]->entries[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_bucketset_next(struct kefir_bucketset_iterator *iter, kefir_bucketset_entry_t *entry_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to bucket set iterator"));
    REQUIRE(iter->bucket_index < iter->bucketset->num_of_buckets, KEFIR_ITERATOR_END);

    if (iter->index + 1 == iter->bucketset->bucket_arr[iter->bucket_index]->length) {
        iter->bucket_index++;
        for (; iter->bucket_index < iter->bucketset->num_of_buckets && iter->bucketset->bucket_arr[iter->bucket_index] == NULL; iter->bucket_index++) {}
        REQUIRE(iter->bucket_index < iter->bucketset->num_of_buckets, KEFIR_ITERATOR_END);
        iter->index = 0;
    } else {
        iter->index++;
    }
    ASSIGN_PTR(entry_ptr, iter->bucketset->bucket_arr[iter->bucket_index]->entries[iter->index]);
    return KEFIR_OK;
}

static kefir_bucketset_hash_t uint_hash(kefir_bucketset_entry_t entry, void *payload) {
    UNUSED(payload);
    return (kefir_hashtree_hash_t) kefir_splitmix64((kefir_uint64_t) entry);
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
