#ifndef KEFIR_COMPILER_INCREMENTAL_TOKEN_CURSOR_H_
#define KEFIR_COMPILER_INCREMENTAL_TOKEN_CURSOR_H_

#include "kefir/lexer/lexem.h"
#include "kefir/lexer/buffer.h"
#include "kefir/preprocessor/preprocessor.h"

typedef struct kefir_token_incremental_cursor_handle {
    struct kefir_token_cursor_handle handle;

    struct kefir_token_buffer buffer;
    struct kefir_token_buffer pp_buffer;

    struct kefir_mem *mem;
    struct kefir_preprocessor_state preprocessor_state;
} kefir_token_incremental_cursor_handle_t;

kefir_result_t kefir_token_incremental_cursor_handle_init(struct kefir_mem *, struct kefir_preprocessor *,
                                                          struct kefir_token_allocator *,
                                                          struct kefir_token_incremental_cursor_handle *);
kefir_result_t kefir_token_incremental_cursor_handle_free(struct kefir_token_incremental_cursor_handle *);

kefir_result_t kefir_token_incremental_cursor_handle_flush_pp_tokens(struct kefir_mem *,
                                                                     struct kefir_token_incremental_cursor_handle *);

#endif