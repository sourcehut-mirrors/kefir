#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_IMPL_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_IMPL_H_

#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
#include "kefir/codegen/opt-system-v-amd64/code.h"

#define DEFINE_TRANSLATOR(_id)                                                                           \
    kefir_result_t kefir_codegen_opt_sysv_amd64_translate_##_id(                                         \
        struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_module *module, \
        const struct kefir_opt_function *function, const struct kefir_opt_code_analysis *func_analysis,  \
        struct kefir_opt_sysv_amd64_function *codegen_func, kefir_opt_instruction_ref_t instr_ref)

#define DEFINE_TRANSLATOR_PROLOGUE                                                                                \
    do {                                                                                                          \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));        \
        REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));   \
        REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));     \
        REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function")); \
        REQUIRE(func_analysis != NULL,                                                                            \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));          \
        REQUIRE(codegen_func != NULL,                                                                             \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function"));           \
    } while (false)

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation(
    const struct kefir_codegen_opt_amd64_sysv_storage_location *, kefir_bool_t *, void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_map_registers(struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
                                                          const struct kefir_opt_function *,
                                                          const struct kefir_opt_code_analysis *,
                                                          struct kefir_opt_sysv_amd64_function *, kefir_opt_block_id_t,
                                                          kefir_opt_block_id_t);

#endif
