#ifndef KEFIR_CORE_BITSET_H_
#define KEFIR_CORE_BITSET_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

typedef struct kefir_bitset {
    kefir_uint64_t *content;
    kefir_size_t length;
    kefir_size_t capacity;
} kefir_bitset_t;

kefir_result_t kefir_bitset_init(struct kefir_bitset *);
kefir_result_t kefir_bitset_free(struct kefir_mem *, struct kefir_bitset *);

kefir_result_t kefir_bitset_get(const struct kefir_bitset *, kefir_size_t, kefir_bool_t *);
kefir_result_t kefir_bitset_set(const struct kefir_bitset *, kefir_size_t, kefir_bool_t);
kefir_result_t kefir_bitset_find(const struct kefir_bitset *, kefir_bool_t, kefir_size_t, kefir_size_t *);

kefir_result_t kefir_bitset_length(const struct kefir_bitset *, kefir_size_t *);
kefir_result_t kefir_bitset_resize(struct kefir_mem *, struct kefir_bitset *, kefir_size_t);

#endif
