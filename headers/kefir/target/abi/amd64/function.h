#ifndef KEFIR_TARGET_ABI_AMD64_FUNCTION_H_
#define KEFIR_TARGET_ABI_AMD64_FUNCTION_H_

#include "kefir/ir/function.h"
#include "kefir/target/abi/amd64/parameters.h"

typedef struct kefir_abi_amd64_function_decl {
    void *payload;
} kefir_abi_amd64_function_decl_t;

kefir_result_t kefir_abi_amd64_function_decl_alloc(struct kefir_mem *, kefir_abi_amd64_variant_t,
                                                   const struct kefir_ir_function_decl *,
                                                   struct kefir_abi_amd64_function_decl *);
kefir_result_t kefir_abi_amd64_function_decl_free(struct kefir_mem *, struct kefir_abi_amd64_function_decl *);

kefir_result_t kefir_abi_amd64_function_decl_ir(const struct kefir_abi_amd64_function_decl *,
                                                const struct kefir_ir_function_decl **);
kefir_result_t kefir_abi_amd64_function_decl_parameters(const struct kefir_abi_amd64_function_decl *,
                                                        const struct kefir_abi_amd64_function_parameters **);
kefir_result_t kefir_abi_amd64_function_decl_parameters_layout(const struct kefir_abi_amd64_function_decl *,
                                                               const struct kefir_abi_amd64_type_layout **);
kefir_result_t kefir_abi_amd64_function_decl_returns(const struct kefir_abi_amd64_function_decl *,
                                                     const struct kefir_abi_amd64_function_parameters **);
kefir_result_t kefir_abi_amd64_function_decl_returns_layout(const struct kefir_abi_amd64_function_decl *,
                                                            const struct kefir_abi_amd64_type_layout **);
kefir_result_t kefir_abi_amd64_function_decl_returns_implicit_parameter(const struct kefir_abi_amd64_function_decl *,
                                                                        kefir_bool_t *,
                                                                        kefir_asm_amd64_xasmgen_register_t *);

#endif
