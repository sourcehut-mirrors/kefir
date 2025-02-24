#ifndef KEFIR_OPTIMIZER_CONFIGURATION_H_
#define KEFIR_OPTIMIZER_CONFIGURATION_H_

#include "kefir/optimizer/pipeline.h"

typedef struct kefir_optimizer_configuration {
    struct kefir_optimizer_pipeline pipeline;

    kefir_size_t max_inline_depth;
    kefir_size_t max_inlines_per_function;
} kefir_optimizer_configuration_t;

kefir_result_t kefir_optimizer_configuration_init(struct kefir_optimizer_configuration *);
kefir_result_t kefir_optimizer_configuration_free(struct kefir_mem *, struct kefir_optimizer_configuration *);

kefir_result_t kefir_optimizer_configuration_add_pipeline_pass(struct kefir_mem *,
                                                               struct kefir_optimizer_configuration *, const char *);

#endif
