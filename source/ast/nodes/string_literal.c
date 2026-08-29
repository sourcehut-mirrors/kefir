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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_size_t literal_size(kefir_ast_string_literal_type_t type, kefir_size_t length) {
    switch (type) {
        case KEFIR_AST_STRING_LITERAL_MULTIBYTE:
        case KEFIR_AST_STRING_LITERAL_UNICODE8:
            return length;

        case KEFIR_AST_STRING_LITERAL_UNICODE16:
            return sizeof(kefir_char16_t) * length;

        case KEFIR_AST_STRING_LITERAL_UNICODE32:
            return sizeof(kefir_char32_t) * length;

        case KEFIR_AST_STRING_LITERAL_WIDE:
            return sizeof(kefir_wchar_t) * length;
    }
    return 0;
}

NODE_VISIT_IMPL(ast_string_literal_visit, kefir_ast_string_literal, string_literal)

kefir_result_t ast_string_literal_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_string_literal *, node, KEFIR_AST_NODE_SELF(base));
    if (!KEFIR_AST_NODE_IS_ARENA_ALLOCATED(node)) {
        KEFIR_FREE(mem, node->data.literal);
    }
    KEFIR_AST_NODE_ARENA_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_STRING_LITERAL_CLASS = {
    .type = KEFIR_AST_STRING_LITERAL, .visit = ast_string_literal_visit, .free = ast_string_literal_free};

static struct kefir_ast_string_literal *kefir_ast_new_string_literal(struct kefir_mem *mem, struct kefir_memory_arena *arena, const void *literal,
                                                              kefir_size_t length,
                                                              kefir_ast_string_literal_type_t type) {
    REQUIRE(mem != NULL || arena != NULL, NULL);
    REQUIRE(literal != NULL, NULL);

    kefir_size_t sz = literal_size(type, length);
    REQUIRE(sz != 0 || length == 0, NULL);
    void *literal_copy = NULL;
    if (arena != NULL) {
        literal_copy = kefir_memory_arena_alloc(arena, sz, _Alignof(kefir_char32_t));
    } else {
        literal_copy = KEFIR_MALLOC(mem, sz);
    }
    REQUIRE(literal_copy != NULL, NULL);

    struct kefir_ast_string_literal *string_literal = KEFIR_AST_NODE_ARENA_ALLOC(mem, arena, struct kefir_ast_string_literal);
    REQUIRE_ELSE(string_literal != NULL, {
        if (arena == NULL) {
            KEFIR_FREE(mem, literal_copy);
        }
        return NULL;
    });

    string_literal->base.refcount = KEFIR_AST_NODE_ARENA_ALLOCATED(arena, 1);
    string_literal->base.klass = &AST_STRING_LITERAL_CLASS;
    kefir_result_t res = kefir_ast_node_properties_init(&string_literal->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (arena == NULL) {
            KEFIR_FREE(mem, literal_copy);
        }
        return NULL;
    });
    res = kefir_source_location_empty(&string_literal->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (arena == NULL) {
            KEFIR_FREE(mem, literal_copy);
        }
        KEFIR_AST_NODE_ARENA_FREE(mem, string_literal);
        return NULL;
    });

    memcpy(literal_copy, literal, sz);
    string_literal->data.type = type;
    string_literal->data.literal = literal_copy;
    string_literal->data.length = length;
    return string_literal;
}

struct kefir_ast_string_literal *kefir_ast_new_string_literal_multibyte(struct kefir_mem *mem, struct kefir_memory_arena *arena, const char *literal,
                                                                        kefir_size_t length) {
    return kefir_ast_new_string_literal(mem, arena, literal, length, KEFIR_AST_STRING_LITERAL_MULTIBYTE);
}

struct kefir_ast_string_literal *kefir_ast_new_string_literal_unicode8(struct kefir_mem *mem, struct kefir_memory_arena *arena, const char *literal,
                                                                       kefir_size_t length) {
    return kefir_ast_new_string_literal(mem, arena, literal, length, KEFIR_AST_STRING_LITERAL_UNICODE8);
}

struct kefir_ast_string_literal *kefir_ast_new_string_literal_unicode16(struct kefir_mem *mem, struct kefir_memory_arena *arena,
                                                                        const kefir_char16_t *literal,
                                                                        kefir_size_t length) {
    return kefir_ast_new_string_literal(mem, arena, literal, length, KEFIR_AST_STRING_LITERAL_UNICODE16);
}

struct kefir_ast_string_literal *kefir_ast_new_string_literal_unicode32(struct kefir_mem *mem, struct kefir_memory_arena *arena,
                                                                        const kefir_char32_t *literal,
                                                                        kefir_size_t length) {
    return kefir_ast_new_string_literal(mem, arena, literal, length, KEFIR_AST_STRING_LITERAL_UNICODE32);
}

struct kefir_ast_string_literal *kefir_ast_new_string_literal_wide(struct kefir_mem *mem, struct kefir_memory_arena *arena, const kefir_wchar_t *literal,
                                                                   kefir_size_t length) {
    return kefir_ast_new_string_literal(mem, arena, literal, length, KEFIR_AST_STRING_LITERAL_WIDE);
}
