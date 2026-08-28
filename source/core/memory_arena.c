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

#include "kefir/core/memory_arena.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#ifndef KEFIR_MEMORY_ARENA_MMAP_BACKED
#define CHUNK_CAPACITY 4096

kefir_result_t kefir_memory_arena_init(struct kefir_mem *mem, struct kefir_memory_arena *arena) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to memory arena"));

    arena->mem = mem;
    arena->chunk = NULL;
    arena->special_chunk = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_memory_arena_free(struct kefir_memory_arena *arena) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));

    for (struct kefir_memory_arena_chunk *chunk = arena->chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        KEFIR_FREE(arena->mem, chunk);
        chunk = prev;
    }

    for (struct kefir_memory_arena_chunk *chunk = arena->special_chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        KEFIR_FREE(arena->mem, chunk);
        chunk = prev;
    }

    memset(arena, 0, sizeof(struct kefir_memory_arena));
    return KEFIR_OK;
}

kefir_result_t kefir_memory_arena_reset(struct kefir_memory_arena *arena) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));

    for (struct kefir_memory_arena_chunk *chunk = arena->chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        KEFIR_FREE(arena->mem, chunk);
        chunk = prev;
    }

    for (struct kefir_memory_arena_chunk *chunk = arena->special_chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        KEFIR_FREE(arena->mem, chunk);
        chunk = prev;
    }

    arena->chunk = NULL;
    arena->special_chunk = NULL;
    return KEFIR_OK;
}

void *kefir_memory_arena_alloc(struct kefir_memory_arena *arena, kefir_size_t size, kefir_size_t alignment) {
    REQUIRE(arena != NULL, NULL);

    alignment = MAX(alignment, 1);
    if (size > CHUNK_CAPACITY) {
        struct kefir_memory_arena_chunk *chunk = KEFIR_MALLOC(arena->mem, sizeof(struct kefir_memory_arena_chunk) + size);
        REQUIRE(chunk != NULL, NULL);

        chunk->prev = arena->special_chunk;
        chunk->top = size;
        arena->special_chunk = chunk;
        return chunk->chunk;
    }

    if (arena->chunk == NULL) {
        struct kefir_memory_arena_chunk *chunk = KEFIR_MALLOC(arena->mem, sizeof(struct kefir_memory_arena_chunk) + CHUNK_CAPACITY);
        REQUIRE(chunk != NULL, NULL);

        chunk->prev = arena->chunk;
        chunk->top = 0;
        arena->chunk = chunk;
    }

    kefir_size_t begin = (arena->chunk->top + alignment - 1) / alignment * alignment;
    kefir_size_t end = begin + size;
    if (end > CHUNK_CAPACITY) {
        struct kefir_memory_arena_chunk *chunk = KEFIR_MALLOC(arena->mem, sizeof(struct kefir_memory_arena_chunk) + CHUNK_CAPACITY);
        REQUIRE(chunk != NULL, NULL);

        chunk->prev = arena->chunk;
        chunk->top = size;
        arena->chunk = chunk;
        return chunk->chunk;
    }

    arena->chunk->top = end;
    return &arena->chunk->chunk[begin];
}
#else
#include "kefir/core/os_error.h"
#include <unistd.h>
#include <sys/mman.h>

#define CHUNK_PAGES 16

kefir_result_t kefir_memory_arena_init(struct kefir_mem *mem, struct kefir_memory_arena *arena) {
    UNUSED(mem);
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to memory arena"));

    long page_size = sysconf(_SC_PAGESIZE);
    REQUIRE(page_size > 0, KEFIR_SET_OS_ERROR("Failed to detect memory page size"));
    REQUIRE(((kefir_size_t) page_size) * CHUNK_PAGES > sizeof(struct kefir_memory_arena_chunk), KEFIR_SET_ERROR(KEFIR_UNKNOWN_ERROR, "Unexpected memory page size"));
    arena->page_size = (kefir_size_t) page_size;
    arena->chunk = NULL;
    arena->special_chunk = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_memory_arena_free(struct kefir_memory_arena *arena) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));

    for (struct kefir_memory_arena_chunk *chunk = arena->chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        int rc = munmap(chunk, chunk->size);
        REQUIRE(!rc, KEFIR_SET_OS_ERROR("Failed to unmap memory arena pages"));
        chunk = prev;
    }

    for (struct kefir_memory_arena_chunk *chunk = arena->special_chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        int rc = munmap(chunk, chunk->size);
        REQUIRE(!rc, KEFIR_SET_OS_ERROR("Failed to unmap memory arena pages"));
        chunk = prev;
    }

    memset(arena, 0, sizeof(struct kefir_memory_arena));
    return KEFIR_OK;
}

kefir_result_t kefir_memory_arena_reset(struct kefir_memory_arena *arena) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    
    for (struct kefir_memory_arena_chunk *chunk = arena->chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        int rc = munmap(chunk, chunk->size);
        REQUIRE(!rc, KEFIR_SET_OS_ERROR("Failed to unmap memory arena pages"));
        chunk = prev;
    }

    for (struct kefir_memory_arena_chunk *chunk = arena->special_chunk; chunk != NULL;) {
        struct kefir_memory_arena_chunk *prev = chunk->prev;
        int rc = munmap(chunk, chunk->size);
        REQUIRE(!rc, KEFIR_SET_OS_ERROR("Failed to unmap memory arena pages"));
        chunk = prev;
    }

    arena->chunk = NULL;
    arena->special_chunk = NULL;
    return KEFIR_OK;
}

void *kefir_memory_arena_alloc(struct kefir_memory_arena *arena, kefir_size_t size, kefir_size_t alignment) {
    REQUIRE(arena != NULL, NULL);

    alignment = MAX(alignment, 1);
    if (size > arena->page_size * CHUNK_PAGES - sizeof(struct kefir_memory_arena_chunk)) {
        kefir_size_t total_size = (sizeof(struct kefir_memory_arena_chunk) + size + arena->page_size - 1) / arena->page_size * arena->page_size;
        void *pages = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        REQUIRE(pages != MAP_FAILED, NULL);

        struct kefir_memory_arena_chunk *chunk = pages;
        chunk->size = total_size;
        chunk->prev = arena->special_chunk;
        chunk->top = size;
        arena->special_chunk = chunk;
        return chunk->chunk;
    }

    if (arena->chunk == NULL) {
        kefir_size_t total_size = arena->page_size * CHUNK_PAGES;
        void *pages = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        REQUIRE(pages != MAP_FAILED, NULL);

        struct kefir_memory_arena_chunk *chunk = pages;
        chunk->size = total_size;
        chunk->prev = arena->chunk;
        chunk->top = 0;
        arena->chunk = chunk;
    }

    kefir_size_t begin = (arena->chunk->top + alignment - 1) / alignment * alignment;
    kefir_size_t end = begin + size;
    if (end > arena->chunk->size - sizeof(struct kefir_memory_arena_chunk)) {
        kefir_size_t total_size = arena->page_size * CHUNK_PAGES;
        void *pages = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        REQUIRE(pages != MAP_FAILED, NULL);

        struct kefir_memory_arena_chunk *chunk = pages;
        chunk->size = total_size;
        chunk->prev = arena->chunk;
        chunk->top = size;
        arena->chunk = chunk;
        return chunk->chunk;
    }

    arena->chunk->top = end;
    return &arena->chunk->chunk[begin];
}

#endif
