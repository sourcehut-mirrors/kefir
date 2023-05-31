#ifndef KEFIR_TARGET_ABI_SYSTEM_V_AMD64_FUNCTION_H_
#define KEFIR_TARGET_ABI_SYSTEM_V_AMD64_FUNCTION_H_

#include "kefir/ir/function.h"
#include "kefir/target/abi/system-v-amd64/parameters.h"
#include "kefir/core/vector.h"

typedef struct kefir_abi_amd64_sysv_function_decl {
    const struct kefir_ir_function_decl *decl;
    struct {
        struct kefir_abi_sysv_amd64_type_layout layout;
        struct kefir_vector allocation;
        struct kefir_abi_sysv_amd64_parameter_location location;
    } parameters;

    struct {
        struct kefir_abi_sysv_amd64_type_layout layout;
        struct kefir_vector allocation;
        bool implicit_parameter;
    } returns;
} kefir_abi_amd64_sysv_function_decl_t;

kefir_result_t kefir_abi_amd64_sysv_function_decl_alloc(struct kefir_mem *, const struct kefir_ir_function_decl *,
                                                        struct kefir_abi_amd64_sysv_function_decl *);
kefir_result_t kefir_abi_amd64_sysv_function_decl_free(struct kefir_mem *, struct kefir_abi_amd64_sysv_function_decl *);

#endif
