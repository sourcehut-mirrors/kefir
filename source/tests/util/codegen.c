#include "kefir/test/codegen.h"
#include "kefir/test/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t translate_impl(struct kefir_mem *mem, struct kefir_codegen *cg, struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));

    ASSIGN_DECL_CAST(struct kefir_test_codegen *, codegen, cg->data);

    codegen->new_codegen.config = codegen->config;
    REQUIRE_OK(kefir_opt_module_construct(mem, kft_util_get_ir_target_platform(), module));
    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE_OPTIMIZED(mem, &codegen->new_codegen.codegen, module));
    return KEFIR_OK;
}

static kefir_result_t close_impl(struct kefir_mem *mem, struct kefir_codegen *cg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator interface"));
    REQUIRE(cg->data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    ASSIGN_DECL_CAST(struct kefir_test_codegen *, codegen, cg->data);
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen->new_codegen.codegen));
    return KEFIR_OK;
}

kefir_result_t kefir_test_codegen_init(struct kefir_mem *mem, struct kefir_test_codegen *codegen, FILE *output,
                                       const struct kefir_codegen_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen amd64"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid FILE"));

    codegen->config = &KefirCodegenDefaultConfiguration;
    REQUIRE_OK(kefir_codegen_amd64_init(mem, &codegen->new_codegen, output, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, config));

    codegen->iface.translate_optimized = translate_impl;
    codegen->iface.close = close_impl;
    codegen->iface.data = codegen;
    codegen->iface.self = codegen;
    return KEFIR_OK;
}
