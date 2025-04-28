#ifndef KEFIR_CODEGEN_VARIABLE_ALLOCATOR_H_
#define KEFIR_CODEGEN_VARIABLE_ALLOCATOR_H_

#include "kefir/optimizer/local_variables.h"

typedef struct kefir_codegen_local_variable_allocator_hooks {
    kefir_result_t (*type_layout)(kefir_id_t, kefir_size_t, kefir_size_t *, kefir_size_t *, void *);
    void *payload;
} kefir_codegen_local_variable_allocator_hooks_t;

typedef struct kefir_codegen_local_variable_allocator {
    struct kefir_bucketset alive_variables;
    struct kefir_hashtree variable_locations;
    kefir_size_t total_size;
    kefir_size_t total_alignment;
    kefir_bool_t all_global;
} kefir_codegen_local_variable_allocator_t;

kefir_result_t kefir_codegen_local_variable_allocator_init(struct kefir_codegen_local_variable_allocator *);
kefir_result_t kefir_codegen_local_variable_allocator_free(struct kefir_mem *, struct kefir_codegen_local_variable_allocator *);

kefir_result_t kefir_codegen_local_variable_allocator_mark_alive(struct kefir_mem *, struct kefir_codegen_local_variable_allocator *, kefir_opt_instruction_ref_t, kefir_id_t *);
kefir_result_t kefir_codegen_local_variable_allocator_mark_all_global(struct kefir_codegen_local_variable_allocator *);
kefir_result_t kefir_codegen_local_variable_allocator_run(struct kefir_mem *, struct kefir_codegen_local_variable_allocator *, const struct kefir_opt_code_container *, const struct kefir_codegen_local_variable_allocator_hooks *, const struct kefir_opt_code_variable_conflicts *);
kefir_result_t kefir_codegen_local_variable_allocation_of(const struct kefir_codegen_local_variable_allocator *, kefir_opt_instruction_ref_t, kefir_int64_t *);

#endif
