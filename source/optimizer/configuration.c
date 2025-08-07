#include "kefir/optimizer/configuration.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_optimizer_configuration_init(struct kefir_optimizer_configuration *conf) {
    REQUIRE(conf != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer configuration"));

    REQUIRE_OK(kefir_optimizer_pipeline_init(&conf->pipeline));
    conf->max_inline_depth = KEFIR_SIZE_MAX;
    conf->max_inlines_per_function = KEFIR_SIZE_MAX;
    conf->target_lowering = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_configuration_free(struct kefir_mem *mem, struct kefir_optimizer_configuration *conf) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(conf != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer configuration"));

    REQUIRE_OK(kefir_optimizer_pipeline_free(mem, &conf->pipeline));
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_configuration_add_pipeline_pass(struct kefir_mem *mem,
                                                               struct kefir_optimizer_configuration *conf,
                                                               const char *spec) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(conf != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer configuration"));
    REQUIRE(spec != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline pass specifier"));

    const struct kefir_optimizer_pass *pass;
    REQUIRE_OK(kefir_optimizer_pass_resolve(spec, &pass));
    REQUIRE_OK(kefir_optimizer_pipeline_add(mem, &conf->pipeline, pass));
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_configuration_copy_from(struct kefir_mem *mem,
                                                       struct kefir_optimizer_configuration *dst_conf,
                                                       const struct kefir_optimizer_configuration *src_conf) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst_conf != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination optimizer configuration"));
    REQUIRE(src_conf != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source optimizer configuration"));

    dst_conf->max_inline_depth = src_conf->max_inline_depth;
    dst_conf->max_inlines_per_function = src_conf->max_inlines_per_function;
    for (const struct kefir_list_entry *iter = kefir_list_head(&src_conf->pipeline.pipeline); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_optimizer_pass *, pass, iter->value);
        REQUIRE_OK(kefir_optimizer_pipeline_add(mem, &dst_conf->pipeline, pass));
    }
    return KEFIR_OK;
}
