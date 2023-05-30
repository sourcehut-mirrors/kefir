#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_

#include "kefir/codegen/opt-system-v-amd64.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
                                                               struct kefir_opt_module *,
                                                               const struct kefir_opt_function *,
                                                               const struct kefir_opt_code_analysis *);

#endif
