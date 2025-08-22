#include "kefir/ast/cache.h"
#include "kefir/ast/context.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t free_cached_type(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(kefir_ast_target_environment_opaque_type_t, type, value);
    ASSIGN_DECL_CAST(const struct kefir_ast_context *, context, payload);
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));

    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_type_cache_init(struct kefir_ast_context_type_cache *cache,
                                                 const struct kefir_ast_context *context) {
    REQUIRE(cache != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST context type cache"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));

    REQUIRE_OK(kefir_hashtree_init(&cache->types, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&cache->types, free_cached_type, (void *) context));
    cache->context = context;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_type_cache_free(struct kefir_mem *mem, struct kefir_ast_context_type_cache *cache) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cache != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context type cache"));

    REQUIRE_OK(kefir_hashtree_free(mem, &cache->types));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_type_cache_get_type(struct kefir_mem *mem, struct kefir_ast_context_type_cache *cache,
                                                     const struct kefir_ast_type *type,
                                                     kefir_ast_target_environment_opaque_type_t *opaque_type_ptr,
                                                     const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cache != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context type cache"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(opaque_type_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST target environment type"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&cache->types, (kefir_hashtree_key_t) type, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *opaque_type_ptr = (kefir_ast_target_environment_opaque_type_t) node->value;
    } else {
        kefir_ast_target_environment_opaque_type_t opaque_type;
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, cache->context, cache->context->target_env, type,
                                                         &opaque_type, source_location));
        res = kefir_hashtree_insert(mem, &cache->types, (kefir_hashtree_key_t) type,
                                    (kefir_hashtree_value_t) opaque_type);
        REQUIRE_ELSE(res == KEFIR_OK, {
            REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, cache->context->target_env, opaque_type));
            return res;
        });
        *opaque_type_ptr = opaque_type;
    }
    return KEFIR_OK;
}
