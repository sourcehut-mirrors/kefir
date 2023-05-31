#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/target/abi/system-v-amd64/function.h"

typedef struct kefir_opt_sysv_amd64_function {
    struct kefir_abi_amd64_sysv_function_decl declaration;
    struct kefir_codegen_opt_sysv_amd64_register_allocator register_allocator;
} kefir_opt_sysv_amd64_function_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
                                                               struct kefir_opt_module *,
                                                               const struct kefir_opt_function *,
                                                               const struct kefir_opt_code_analysis *);

#endif
