#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_H_

#include "kefir/core/hashtreeset.h"
#include "kefir/core/list.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"

typedef struct kefir_codegen_opt_sysv_amd64_storage {
    struct kefir_hashtreeset occupied_general_purpose_regs;
    struct kefir_hashtreeset borrowed_general_purpose_regs;
    struct kefir_list borrowed_general_purpose_reg_stack;
} kefir_codegen_opt_sysv_amd64_storage_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_init(struct kefir_codegen_opt_sysv_amd64_storage *);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_free(struct kefir_mem *,
                                                         struct kefir_codegen_opt_sysv_amd64_storage *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_used(struct kefir_mem *,
                                                                       struct kefir_codegen_opt_sysv_amd64_storage *,
                                                                       kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_unused(struct kefir_mem *,
                                                                         struct kefir_codegen_opt_sysv_amd64_storage *,
                                                                         kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_released(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_has_borrowed_registers(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_bool_t *);

typedef struct kefir_codegen_opt_sysv_amd64_storage_temporary_register {
    kefir_bool_t borrow;
    kefir_bool_t evicted;
    kefir_asm_amd64_xasmgen_register_t reg;
} kefir_codegen_opt_sysv_amd64_storage_temporary_register_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_temporary_general_purpose_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    struct kefir_codegen_opt_sysv_amd64_storage_temporary_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_specific_temporary_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, kefir_asm_amd64_xasmgen_register_t,
    struct kefir_codegen_opt_sysv_amd64_storage_temporary_register *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_release_temporary_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_storage_temporary_register *);

#endif
