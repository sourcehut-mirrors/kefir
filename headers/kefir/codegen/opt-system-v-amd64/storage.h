#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_H_

#include "kefir/core/hashtreeset.h"
#include "kefir/core/list.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"

typedef struct kefir_codegen_opt_sysv_amd64_storage {
    struct kefir_hashtreeset occupied_regs;
    struct kefir_hashtreeset borrowed_regs;
    struct kefir_list borrowed_reg_stack;
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

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_has_borrowed_registers(
    const struct kefir_codegen_opt_sysv_amd64_storage *, kefir_bool_t *);

typedef struct kefir_codegen_opt_sysv_amd64_storage_register {
    kefir_bool_t borrowed;
    kefir_bool_t evicted;
    kefir_asm_amd64_xasmgen_register_t reg;
} kefir_codegen_opt_sysv_amd64_storage_register_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_shared_allocated_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_floating_point_allocated_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, kefir_asm_amd64_xasmgen_register_t,
    struct kefir_codegen_opt_sysv_amd64_storage_register *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(
    struct kefir_amd64_xasmgen *, const struct kefir_codegen_opt_sysv_amd64_storage_register *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_release_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_storage_register *);

typedef enum kefir_codegen_opt_amd64_sysv_storage_location_type {
    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER,
    KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY
} kefir_codegen_opt_amd64_sysv_storage_location_type_t;

typedef struct kefir_codegen_opt_amd64_sysv_storage_location {
    kefir_codegen_opt_amd64_sysv_storage_location_type_t type;

    union {
        kefir_asm_amd64_xasmgen_register_t reg;
        struct {
            kefir_asm_amd64_xasmgen_register_t base_reg;
            kefir_int64_t offset;
        } memory;
    };
} kefir_codegen_opt_amd64_sysv_storage_location_t;

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
    struct kefir_codegen_opt_amd64_sysv_storage_location *, const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *);

const struct kefir_asm_amd64_xasmgen_operand *kefir_codegen_opt_amd64_sysv_storage_location_operand(
    struct kefir_asm_amd64_xasmgen_operand *, struct kefir_codegen_opt_amd64_sysv_storage_location *);

#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER (1ull)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER (1ull << 1)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY (1ull << 2)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY (1ull << 3)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER (1ull << 4)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER_FLAG (1ull << 5)
#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(_reg) \
    ((((kefir_uint64_t) (_reg)) << 48) | KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER_FLAG)
typedef struct kefir_codegen_opt_amd64_sysv_storage_handle {
    struct kefir_codegen_opt_amd64_sysv_storage_location location;
    struct kefir_codegen_opt_sysv_amd64_storage_register storage_reg;
} kefir_codegen_opt_amd64_sysv_storage_handle_t;

typedef kefir_result_t (*kefir_codegen_opt_amd64_sysv_storage_acquire_filter_callback_t)(
    const struct kefir_codegen_opt_amd64_sysv_storage_location *, kefir_bool_t *, void *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_acquire(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *, kefir_uint64_t,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    struct kefir_codegen_opt_amd64_sysv_storage_handle *,
    kefir_codegen_opt_amd64_sysv_storage_acquire_filter_callback_t, void *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_release(struct kefir_mem *, struct kefir_amd64_xasmgen *,
                                                            struct kefir_codegen_opt_sysv_amd64_storage *,
                                                            struct kefir_codegen_opt_amd64_sysv_storage_handle *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_load(
    struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *);
kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_store(
    struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *);
kefir_result_t kefir_codegen_opt_amd64_sysv_storage_handle_restore_evicted(
    struct kefir_amd64_xasmgen *, const struct kefir_codegen_opt_amd64_sysv_storage_handle *);
kefir_result_t kefir_codegen_opt_amd64_sysv_storage_handle_mask_evicted(
    struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_amd64_sysv_storage_handle *);

#define KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(_handle) \
    ((_handle)->location.type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER && (_handle)->storage_reg.evicted)

#endif
