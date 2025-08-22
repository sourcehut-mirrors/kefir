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

#include "kefir/ast-translator/cache.h"
#include "kefir/ast-translator/context.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t free_cached_type(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_type *, type, value);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator type"));

    REQUIRE_OK(kefir_ast_translator_type_free(mem, type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_type_cache_init(struct kefir_ast_translator_context_type_cache *cache,
                                                            const struct kefir_ast_translator_context *context) {
    REQUIRE(cache != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST translator context type cache"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    REQUIRE_OK(kefir_hashtree_init(&cache->types, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&cache->types, free_cached_type, NULL));
    cache->context = context;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_type_cache_free(struct kefir_mem *mem,
                                                            struct kefir_ast_translator_context_type_cache *cache) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cache != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context type cache"));

    REQUIRE_OK(kefir_hashtree_free(mem, &cache->types));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_type_cache_get_type(
    struct kefir_mem *mem, struct kefir_ast_translator_context_type_cache *cache, const struct kefir_ast_type *type,
    const struct kefir_ast_translator_type **translator_type_ptr, const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cache != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context type cache"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(translator_type_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST target environment type"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&cache->types, (kefir_hashtree_key_t) type, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *translator_type_ptr = (const struct kefir_ast_translator_type *) node->value;
    } else {
        struct kefir_ast_translator_type *translator_type;
        REQUIRE_OK(kefir_ast_translator_type_new(mem, cache->context->ast_context, cache->context->environment,
                                                 cache->context->module, type, 0, &translator_type, source_location));
        res = kefir_hashtree_insert(mem, &cache->types, (kefir_hashtree_key_t) type,
                                    (kefir_hashtree_value_t) translator_type);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(mem, translator_type);
            return res;
        });
        *translator_type_ptr = translator_type;
    }
    return KEFIR_OK;
}
