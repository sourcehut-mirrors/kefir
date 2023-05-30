#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/codegen/opt-common/linear_register_allocator.h"

extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[];
extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[];
extern const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters;
extern const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters;

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(struct kefir_mem *, const struct kefir_opt_function *,
                                                                const struct kefir_opt_code_analysis *,
                                                                struct kefir_codegen_opt_linear_register_allocator *);

#endif
