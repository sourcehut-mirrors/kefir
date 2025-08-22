#ifndef KEFIR_AST_CACHE_H_
#define KEFIR_AST_CACHE_H_

#include "kefir/ast/base.h"
#include "kefir/core/hashtree.h"
#include "kefir/ast/target_environment.h"

typedef struct kefir_ast_context_type_cache {
    const struct kefir_ast_context *context;
    struct kefir_hashtree types;
} kefir_ast_context_type_cache_t;

kefir_result_t kefir_ast_context_type_cache_init(struct kefir_ast_context_type_cache *,
                                                 const struct kefir_ast_context *);
kefir_result_t kefir_ast_context_type_cache_free(struct kefir_mem *, struct kefir_ast_context_type_cache *);

kefir_result_t kefir_ast_context_type_cache_get_type(struct kefir_mem *, struct kefir_ast_context_type_cache *,
                                                     const struct kefir_ast_type *,
                                                     kefir_ast_target_environment_opaque_type_t *,
                                                     const struct kefir_source_location *);

#endif
