#include "kefir/target/abi/system-v-amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t function_alloc_return(struct kefir_mem *mem,
                                            struct kefir_abi_amd64_sysv_function_decl *sysv_decl) {
    REQUIRE(kefir_ir_type_children(sysv_decl->decl->result) <= 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected IR function to have return type count less than 2"));
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(sysv_decl->decl->result, mem, &sysv_decl->returns.layout));
    kefir_result_t res = kefir_abi_sysv_amd64_parameter_classify(
        mem, sysv_decl->decl->result, &sysv_decl->returns.layout, &sysv_decl->returns.allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->returns.layout);
        return res;
    });
    sysv_decl->returns.implicit_parameter = false;
    if (kefir_ir_type_children(sysv_decl->decl->result) > 0) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, result,
                         kefir_vector_at(&sysv_decl->returns.allocation, 0));
        sysv_decl->returns.implicit_parameter = result->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY;
    }
    return KEFIR_OK;
}

static kefir_result_t function_alloc_params(struct kefir_mem *mem,
                                            struct kefir_abi_amd64_sysv_function_decl *sysv_decl) {
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(sysv_decl->decl->params, mem, &sysv_decl->parameters.layout));

    kefir_result_t res = kefir_abi_sysv_amd64_parameter_classify(
        mem, sysv_decl->decl->params, &sysv_decl->parameters.layout, &sysv_decl->parameters.allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->parameters.layout);
        return res;
    });
    sysv_decl->parameters.location = (struct kefir_abi_sysv_amd64_parameter_location){0};
    if (sysv_decl->returns.implicit_parameter) {
        sysv_decl->parameters.location.integer_register++;
    }
    res = kefir_abi_sysv_amd64_parameter_allocate(mem, sysv_decl->decl->params, &sysv_decl->parameters.layout,
                                                  &sysv_decl->parameters.allocation, &sysv_decl->parameters.location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_parameter_free(mem, &sysv_decl->parameters.allocation);
        kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->parameters.layout);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_function_decl_alloc(struct kefir_mem *mem,
                                                        const struct kefir_ir_function_decl *decl,
                                                        struct kefir_abi_amd64_sysv_function_decl *sysv_decl) {
    sysv_decl->decl = decl;
    REQUIRE_OK(function_alloc_return(mem, sysv_decl));
    kefir_result_t res = function_alloc_params(mem, sysv_decl);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_parameter_free(mem, &sysv_decl->returns.allocation);
        kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->returns.layout);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_function_decl_free(struct kefir_mem *mem,
                                                       struct kefir_abi_amd64_sysv_function_decl *sysv_decl) {
    REQUIRE_OK(kefir_abi_sysv_amd64_parameter_free(mem, &sysv_decl->returns.allocation));
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->returns.layout));
    REQUIRE_OK(kefir_abi_sysv_amd64_parameter_free(mem, &sysv_decl->parameters.allocation));
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_decl->parameters.layout));
    return KEFIR_OK;
}
