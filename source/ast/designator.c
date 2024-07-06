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

#include <string.h>
#include "kefir/ast/designator.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct kefir_ast_designator *kefir_ast_new_member_designator(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                             const char *member, struct kefir_ast_designator *child) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(member != NULL && strlen(member) > 0, NULL);

    if (symbols != NULL) {
        member = kefir_string_pool_insert(mem, symbols, member, NULL);
        REQUIRE(member != NULL, NULL);
    }
    struct kefir_ast_designator *designator = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_designator));
    REQUIRE(designator != NULL, NULL);

    designator->type = KEFIR_AST_DESIGNATOR_MEMBER;
    designator->member = member;
    designator->next = child;
    return designator;
}

struct kefir_ast_designator *kefir_ast_new_index_designator(struct kefir_mem *mem, kefir_size_t index,
                                                            struct kefir_ast_designator *child) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_designator *designator = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_designator));
    REQUIRE(designator != NULL, NULL);

    designator->type = KEFIR_AST_DESIGNATOR_SUBSCRIPT;
    designator->index = index;
    designator->next = child;
    return designator;
}

struct kefir_ast_designator *kefir_ast_new_range_designator(struct kefir_mem *mem, kefir_size_t begin, kefir_size_t end,
                                                            struct kefir_ast_designator *child) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_designator *designator = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_designator));
    REQUIRE(designator != NULL, NULL);

    designator->type = KEFIR_AST_DESIGNATOR_SUBSCRIPT_RANGE;
    designator->range.begin = begin;
    designator->range.end = end;
    designator->next = child;
    return designator;
}


kefir_result_t kefir_ast_temporary_index_designator_from_range(const struct kefir_ast_designator *range_designator, kefir_size_t index, struct kefir_ast_designator *designator) {
    REQUIRE(range_designator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid subscript range designator"));
    REQUIRE(range_designator->type == KEFIR_AST_DESIGNATOR_SUBSCRIPT_RANGE, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid subscript range designator"));
    REQUIRE(index < kefir_ast_designator_elements(range_designator), KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested index is out of subscript designator range"));

    designator->type = KEFIR_AST_DESIGNATOR_SUBSCRIPT;
    designator->index = range_designator->range.begin + index;
    designator->next = range_designator->next;
    return KEFIR_OK;
}

kefir_size_t kefir_ast_designator_elements(const struct kefir_ast_designator *designator) {
    REQUIRE(designator != NULL, 0);

    switch (designator->type) {
        case KEFIR_AST_DESIGNATOR_MEMBER:
        case KEFIR_AST_DESIGNATOR_SUBSCRIPT:
            return 1;

        case KEFIR_AST_DESIGNATOR_SUBSCRIPT_RANGE:
            return designator->range.end - designator->range.begin + 1;
    }
    return 0;
}

kefir_result_t kefir_ast_designator_free(struct kefir_mem *mem, struct kefir_ast_designator *designator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(designator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST designator"));

    if (designator->next != NULL) {
        REQUIRE_OK(kefir_ast_designator_free(mem, designator->next));
        designator->next = NULL;
    }
    designator->member = NULL;
    KEFIR_FREE(mem, designator);
    return KEFIR_OK;
}

struct kefir_ast_designator *kefir_ast_designator_clone(struct kefir_mem *mem, const struct kefir_ast_designator *src) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(src != NULL, NULL);

    struct kefir_ast_designator *dst = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_designator));
    REQUIRE(dst != NULL, NULL);
    dst->type = src->type;
    switch (src->type) {
        case KEFIR_AST_DESIGNATOR_MEMBER:
            dst->member = src->member;
            break;

        case KEFIR_AST_DESIGNATOR_SUBSCRIPT:
            dst->index = src->index;
            break;

        case KEFIR_AST_DESIGNATOR_SUBSCRIPT_RANGE:
            dst->range.begin = src->range.begin;
            dst->range.end = src->range.end;
            break;
    }
    dst->next = kefir_ast_designator_clone(mem, src->next);
    if (src->next != NULL) {
        REQUIRE_ELSE(dst->next != NULL, {
            KEFIR_FREE(mem, dst);
            return NULL;
        });
    }
    return dst;
}

static kefir_result_t kefir_ast_designator_unroll_impl(struct kefir_ast_designator *root_designator, struct kefir_ast_designator *prev_designator, struct kefir_ast_designator *designator, kefir_result_t (*callback)(struct kefir_ast_designator *, void *), void *payload) {
    if (designator == NULL) {
        REQUIRE_OK(callback(root_designator, payload));
        return KEFIR_OK;
    }

    struct kefir_ast_designator designator_iter = *designator;
    if (root_designator == NULL) {
        root_designator = &designator_iter;
    }
    if (prev_designator != NULL) {
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
#endif
        prev_designator->next = &designator_iter;
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
#endif
    }

    switch (designator_iter.type) {
        case KEFIR_AST_DESIGNATOR_MEMBER:
        case KEFIR_AST_DESIGNATOR_SUBSCRIPT:
            REQUIRE_OK(kefir_ast_designator_unroll_impl(root_designator, &designator_iter, designator->next, callback, payload));
            break;

        case KEFIR_AST_DESIGNATOR_SUBSCRIPT_RANGE:
            designator_iter.type = KEFIR_AST_DESIGNATOR_SUBSCRIPT;
            for (kefir_size_t i = designator->range.begin; i <= designator->range.end; i++) {
                designator_iter.index = i;
                REQUIRE_OK(kefir_ast_designator_unroll_impl(root_designator, &designator_iter, designator->next, callback, payload));
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_designator_unroll(struct kefir_ast_designator *designator, kefir_result_t (*callback)(struct kefir_ast_designator *, void *), void *payload) {
    REQUIRE(callback != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST designator unroll callback"));

    if (designator != NULL) {
        REQUIRE_OK(kefir_ast_designator_unroll_impl(NULL, NULL, designator, callback, payload));
    } else {
        REQUIRE_OK(callback(designator, payload));
    }
    return KEFIR_OK;
}
