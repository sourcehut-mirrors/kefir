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

#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define BLOCK_SIZE (4096 / sizeof(kefir_queue_entry_t))

static kefir_result_t block_free(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                 void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid queue list entry"));
    ASSIGN_DECL_CAST(struct kefir_queue_entry_t *, block, entry->value);
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid queue block"));

    KEFIR_FREE(mem, block);
    return KEFIR_OK;
}

kefir_result_t kefir_queue_init(struct kefir_queue *queue) {
    REQUIRE(queue != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to queue"));

    REQUIRE_OK(kefir_list_init(&queue->blocks));
    REQUIRE_OK(kefir_list_on_remove(&queue->blocks, block_free, NULL));
    queue->head_index = 0;
    queue->tail_index = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_queue_free(struct kefir_mem *mem, struct kefir_queue *queue) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(queue != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid queue"));

    REQUIRE_OK(kefir_list_free(mem, &queue->blocks));
    memset(queue, 0, sizeof(struct kefir_queue));
    return KEFIR_OK;
}

kefir_bool_t kefir_queue_is_empty(const struct kefir_queue *queue) {
    REQUIRE(queue != NULL, true);
    const kefir_size_t num_of_blocks = kefir_list_length(&queue->blocks);
    if (num_of_blocks == 0) {
        return true;
    } else if (num_of_blocks == 1) {
        return queue->head_index == queue->tail_index;
    } else {
        return false;
    }
}

kefir_result_t kefir_queue_push(struct kefir_mem *mem, struct kefir_queue *queue, kefir_queue_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(queue != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid queue"));

    kefir_queue_entry_t *block = NULL;
    if (queue->tail_index == BLOCK_SIZE || kefir_list_tail(&queue->blocks) == NULL) {
        kefir_queue_entry_t *new_block = KEFIR_MALLOC(mem, sizeof(kefir_queue_entry_t) * BLOCK_SIZE);
        REQUIRE(new_block != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate queue block"));
        kefir_result_t res =
            kefir_list_insert_after(mem, &queue->blocks, kefir_list_tail(&queue->blocks), (void *) new_block);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, new_block);
            return res;
        });
        queue->tail_index = 0;
        block = new_block;
    } else {
        struct kefir_list_entry *tail = kefir_list_tail(&queue->blocks);
        REQUIRE(tail != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve queue tail block"));
        block = tail->value;
    }

    block[queue->tail_index++] = entry;

    return KEFIR_OK;
}

kefir_result_t kefir_queue_pop_first(struct kefir_mem *mem, struct kefir_queue *queue, kefir_queue_entry_t *entry_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(queue != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid queue"));

    struct kefir_list_entry *head = kefir_list_head(&queue->blocks);
    struct kefir_list_entry *tail = kefir_list_tail(&queue->blocks);
    REQUIRE(head != NULL, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Queue is empty"));
    REQUIRE(head != tail || queue->head_index < queue->tail_index,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Queue is empty"));

    ASSIGN_DECL_CAST(const kefir_queue_entry_t *, block, head->value);
    ASSIGN_PTR(entry_ptr, block[queue->head_index]);

    queue->head_index++;
    if (head != tail && queue->head_index == BLOCK_SIZE) {
        REQUIRE_OK(kefir_list_pop(mem, &queue->blocks, head));
        queue->head_index = 0;
    } else if (head == tail && queue->head_index == queue->tail_index) {
        REQUIRE_OK(kefir_list_pop(mem, &queue->blocks, head));
        queue->head_index = 0;
        queue->tail_index = 0;
    }

    return KEFIR_OK;
}
