#ifndef KEFIR_CODEGEN_NAIVE_SYSTEM_V_AMD64_INLINE_ASSEMBLY_H_
#define KEFIR_CODEGEN_NAIVE_SYSTEM_V_AMD64_INLINE_ASSEMBLY_H_

#include "kefir/codegen/naive-system-v-amd64/module.h"
#include "kefir/codegen/naive-system-v-amd64/codegen.h"

kefir_result_t kefir_codegen_naive_amd64_sysv_inline_assembly_invoke(struct kefir_mem *,
                                                                     struct kefir_codegen_naive_amd64_sysv_module *,
                                                                     struct kefir_codegen_naive_amd64 *,
                                                                     const struct kefir_ir_inline_assembly *);

kefir_result_t kefir_codegen_naive_amd64_sysv_inline_assembly_embed(struct kefir_mem *,
                                                                    struct kefir_codegen_naive_amd64_sysv_module *,
                                                                    struct kefir_codegen_naive_amd64 *,
                                                                    const struct kefir_ir_inline_assembly *);

#endif
