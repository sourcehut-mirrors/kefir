#include "kefir/ast-translator/scope/scoped_identifier.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_ast_translator_define_object_scope_lifetime(struct kefir_mem *mem,
                                                                 const struct kefir_ast_identifier_flat_scope *scope,
                                                                 kefir_size_t lifetime_begin,
                                                                 kefir_size_t lifetime_end) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scope != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier flat scope"));
    REQUIRE(lifetime_begin <= lifetime_end,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected object lifetime begin index to precede end index"));

    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(scope, &iter)) {

        switch (iter.value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_id,
                                 iter.value->payload.ptr);

                scoped_id->lifetime.bounded = true;
                scoped_id->lifetime.begin = lifetime_begin;
                scoped_id->lifetime.end = lifetime_end;
            } break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
            case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
                // Intentionally left blank
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected identifier in object scope");
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
