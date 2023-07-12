#include "kefir/optimizer/configuration.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_optimizer_configuration_init(struct kefir_optimizer_configuration *conf) {
    REQUIRE(conf != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer configuration"));

    REQUIRE_OK(kefir_optimizer_pipeline_init(&conf->pipeline));
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
