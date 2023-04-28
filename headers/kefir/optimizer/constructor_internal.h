#ifndef KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_H_
#define KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_H_

#ifndef KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_INCLUDE
#error "This optimizer constructor internal header includes shall be specifically marked"
#endif

#include "kefir/optimizer/constructor.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"

typedef struct kefir_opt_constructor_code_block_state {
    kefir_opt_block_id_t block_id;
    struct kefir_list stack;
} kefir_opt_constructor_code_block_state_t;

typedef struct kefir_opt_constructor_state {
    struct kefir_opt_code_container *code;
    struct kefir_hashtree code_blocks;
    struct kefir_opt_constructor_code_block_state *current_block;
    kefir_size_t ir_location;
} kefir_opt_constructor_state;

kefir_result_t kefir_opt_constructor_init(struct kefir_opt_code_container *, struct kefir_opt_constructor_state *);
kefir_result_t kefir_opt_constructor_free(struct kefir_mem *, struct kefir_opt_constructor_state *);

kefir_result_t kefir_opt_constructor_start_code_block_at(struct kefir_mem *, struct kefir_opt_constructor_state *,
                                                         kefir_size_t);
kefir_result_t kefir_opt_constructor_find_code_block_for(const struct kefir_opt_constructor_state *, kefir_size_t,
                                                         struct kefir_opt_constructor_code_block_state **);
kefir_result_t kefir_opt_constructor_update_current_code_block(struct kefir_mem *,
                                                               struct kefir_opt_constructor_state *);

#endif
