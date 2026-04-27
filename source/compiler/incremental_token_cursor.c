#include "kefir/compiler/incremental_token_cursor.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t get_token(kefir_size_t index, const struct kefir_token **token_ptr,
                                const struct kefir_token_cursor_handle *handle) {
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));
    ASSIGN_DECL_CAST(struct kefir_token_incremental_cursor_handle *, inc_handle, handle->payload[0]);

    REQUIRE_OK(kefir_token_incremental_cursor_handle_flush_pp_tokens(inc_handle->mem, inc_handle));
    while (index >= kefir_token_buffer_length(&inc_handle->buffer)) {
        kefir_bool_t finished = false;
        REQUIRE_OK(
            kefir_preprocessor_state_run(inc_handle->mem, &inc_handle->preprocessor_state, &inc_handle->pp_buffer,
                                         index - kefir_token_buffer_length(&inc_handle->buffer) + 1024, &finished));
        REQUIRE_OK(kefir_token_incremental_cursor_handle_flush_pp_tokens(inc_handle->mem, inc_handle));
        if (finished) {
            break;
        }
    }
    *token_ptr = kefir_token_buffer_at(&inc_handle->buffer, index);
    return KEFIR_OK;
}

kefir_result_t kefir_token_incremental_cursor_handle_init(struct kefir_mem *mem,
                                                          struct kefir_preprocessor *preprocessor,
                                                          struct kefir_token_allocator *token_allocator,
                                                          struct kefir_token_incremental_cursor_handle *handle) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));
    REQUIRE(token_allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator"));
    REQUIRE(handle != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to incremental token cursor handle"));

    handle->mem = mem;
    REQUIRE_OK(kefir_token_buffer_init(&handle->pp_buffer));
    REQUIRE_OK(kefir_token_buffer_init(&handle->buffer));
    REQUIRE_OK(kefir_preprocessor_state_init(mem, preprocessor, token_allocator, NULL, &handle->preprocessor_state));
    handle->handle.get_token = get_token;
    handle->handle.payload[0] = (kefir_uptr_t) handle;
    return KEFIR_OK;
}

kefir_result_t kefir_token_incremental_cursor_handle_free(struct kefir_token_incremental_cursor_handle *handle) {
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid incremental token cursor handle"));

    REQUIRE_OK(kefir_token_buffer_free(handle->mem, &handle->pp_buffer));
    REQUIRE_OK(kefir_token_buffer_free(handle->mem, &handle->buffer));
    return KEFIR_OK;
}

kefir_result_t kefir_token_incremental_cursor_handle_flush_pp_tokens(
    struct kefir_mem *mem, struct kefir_token_incremental_cursor_handle *handle) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid incremental token cursor handle"));

    REQUIRE_OK(kefir_preprocessor_token_convert_buffer(mem, handle->preprocessor_state.preprocessor,
                                                       handle->preprocessor_state.token_allocator, &handle->buffer,
                                                       &handle->pp_buffer));
    REQUIRE_OK(kefir_token_buffer_reset(mem, &handle->pp_buffer));
    return KEFIR_OK;
}
