#ifndef KEFIR_CODEGEN_AMD64_SYSTEM_V_INLINE_ASSEMBLY_H_
#define KEFIR_CODEGEN_AMD64_SYSTEM_V_INLINE_ASSEMBLY_H_

#include "kefir/codegen/amd64/system-v/abi/module.h"
#include "kefir/codegen/amd64-sysv.h"

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_invoke(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_sysv_module *,
                                                               struct kefir_codegen_amd64 *,
                                                               const struct kefir_ir_inline_assembly *);

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_embed(struct kefir_mem *,
                                                              struct kefir_codegen_amd64_sysv_module *,
                                                              struct kefir_codegen_amd64 *,
                                                              const struct kefir_ir_inline_assembly *);

#endif
