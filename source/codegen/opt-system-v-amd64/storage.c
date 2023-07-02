#include "kefir/codegen/opt-system-v-amd64/storage.h"
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
#include "kefir/codegen/opt-system-v-amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
    struct kefir_mem *, struct kefir_amd64_xasmgen *, struct kefir_codegen_opt_sysv_amd64_storage *,
    struct kefir_codegen_opt_sysv_amd64_storage_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_init(struct kefir_codegen_opt_sysv_amd64_storage *storage) {
    REQUIRE(storage != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen storage"));

    REQUIRE_OK(kefir_hashtreeset_init(&storage->occupied_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&storage->borrowed_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&storage->borrowed_reg_stack));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_free(struct kefir_mem *mem,
                                                         struct kefir_codegen_opt_sysv_amd64_storage *storage) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_list_free(mem, &storage->borrowed_reg_stack));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &storage->borrowed_regs));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &storage->occupied_regs));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_used(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(!kefir_hashtreeset_has(&storage->occupied_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is already occupied"));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is borrowed at the moment"));

    REQUIRE_OK(kefir_hashtreeset_add(mem, &storage->occupied_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_unused(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(kefir_hashtreeset_has(&storage->occupied_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is not occupied"));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is borrowed at the moment"));

    REQUIRE_OK(kefir_hashtreeset_delete(mem, &storage->occupied_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register has already been borrowed"));

    REQUIRE_OK(kefir_hashtreeset_add(mem, &storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg));
    REQUIRE_OK(kefir_list_insert_after(mem, &storage->borrowed_reg_stack, kefir_list_tail(&storage->borrowed_reg_stack),
                                       (void *) (kefir_uptr_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_released(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(kefir_hashtreeset_has(&storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register has not been borrowed"));

    struct kefir_list_entry *borrow_iter = kefir_list_tail(&storage->borrowed_reg_stack);
    REQUIRE(((kefir_asm_amd64_xasmgen_register_t) (kefir_uptr_t) borrow_iter->value) == reg,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Register is released out of order"));

    REQUIRE_OK(kefir_list_pop(mem, &storage->borrowed_reg_stack, borrow_iter));
    REQUIRE_OK(kefir_hashtreeset_delete(mem, &storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_asm_amd64_xasmgen_register_t reg,
    kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    *result = kefir_hashtreeset_has(&storage->occupied_regs, (kefir_hashtreeset_entry_t) reg);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_asm_amd64_xasmgen_register_t reg,
    kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    *result = kefir_hashtreeset_has(&storage->borrowed_regs, (kefir_hashtreeset_entry_t) reg);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_has_borrowed_registers(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *result = !kefir_hashtreeset_empty(&storage->borrowed_regs);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation, kefir_bool_t exclusive,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    tmp_reg->borrowed = false;
    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {

        kefir_bool_t filter_success = true;
        if (filter_callback != NULL) {
            REQUIRE_OK(filter_callback(reg_allocation->result.reg, &filter_success, filter_payload));
        }

        kefir_bool_t borrowed;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, reg_allocation->result.reg, &borrowed));

        if (filter_success && !borrowed) {
            if (exclusive) {
                tmp_reg->borrowed = true;
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage,
                                                                                       reg_allocation->result.reg));
            }
            tmp_reg->evicted = false;
            tmp_reg->reg = reg_allocation->result.reg;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to acquire allocated register");
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    kefir_result_t res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
        mem, storage, reg_allocation, true, tmp_reg, filter_callback, filter_payload);
    if (res == KEFIR_NO_MATCH) {
        if (reg_allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_FLOATING_POINT) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
                mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
        } else {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
                mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
        }
    }

    REQUIRE_OK(res);
    return KEFIR_OK;
}
kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_shared_allocated_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    kefir_result_t res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
        mem, storage, reg_allocation, false, tmp_reg, filter_callback, filter_payload);
    if (res == KEFIR_NO_MATCH) {
        if (reg_allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_FLOATING_POINT) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
                mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
        } else {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
                mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
        }
    }

    REQUIRE_OK(res);
    return KEFIR_OK;
}

struct filter_floating_point_arg {
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *);
    void *filter_payload;
};

static kefir_result_t filter_floating_point(kefir_asm_amd64_xasmgen_register_t reg, kefir_bool_t *result,
                                            void *payload) {
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct filter_floating_point_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid register filter parameter"));

    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
        *result = false;
    } else {
        *result = true;
        if (arg->filter_callback != NULL) {
            REQUIRE_OK(arg->filter_callback(reg, result, arg->filter_payload));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_floating_point_allocated_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    struct filter_floating_point_arg filter_arg = {.filter_callback = filter_callback,
                                                   .filter_payload = filter_payload};
    kefir_result_t res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
        mem, storage, reg_allocation, true, tmp_reg, filter_floating_point, &filter_arg);
    if (res == KEFIR_NO_MATCH) {
        res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(mem, xasmgen, storage, tmp_reg,
                                                                                       filter_callback, filter_payload);
    }

    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t push_candidate(struct kefir_amd64_xasmgen *xasmgen, kefir_asm_amd64_xasmgen_register_t reg) {
    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
    } else {
        struct kefir_asm_amd64_xasmgen_operand operand;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&operand, KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
            xasmgen,
            kefir_asm_amd64_xasmgen_operand_indirect(
                &operand, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
            kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    tmp_reg->borrowed = true;
    kefir_bool_t filter_success = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64GeneralPurposeRegisters[i];
        kefir_bool_t occupied, borrowed;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(storage, candidate, &occupied));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, candidate, &borrowed));
        if (!occupied && !borrowed) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }

            if (filter_success) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, candidate));
                tmp_reg->evicted = false;
                tmp_reg->reg = candidate;
                return KEFIR_OK;
            }
        }
    }

    tmp_reg->evicted = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64GeneralPurposeRegisters[i];
        kefir_bool_t borrowed;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, candidate, &borrowed));
        if (!borrowed) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }
            if (filter_success) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, candidate));
                REQUIRE_OK(push_candidate(xasmgen, candidate));
                tmp_reg->reg = candidate;
                return KEFIR_OK;
            }
        }
    }

    return KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to obtain a temporary register");
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    tmp_reg->borrowed = true;
    kefir_bool_t filter_success = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfFloatingPointRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64FloatingPointRegisters[i];
        kefir_bool_t occupied, borrowed;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(storage, candidate, &occupied));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, candidate, &borrowed));
        if (!occupied && !borrowed) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }

            if (filter_success) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, candidate));
                tmp_reg->evicted = false;
                tmp_reg->reg = candidate;
                return KEFIR_OK;
            }
        }
    }

    tmp_reg->evicted = true;
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfFloatingPointRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t candidate = KefirOptSysvAmd64FloatingPointRegisters[i];
        kefir_bool_t borrowed;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, candidate, &borrowed));
        if (!borrowed) {
            filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(candidate, &filter_success, filter_payload));
            }
            if (filter_success) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, candidate));
                REQUIRE_OK(push_candidate(xasmgen, candidate));
                tmp_reg->reg = candidate;
                return KEFIR_OK;
            }
        }
    }

    return KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to obtain a temporary register");
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t reg, struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    tmp_reg->borrowed = true;
    if (reg_allocation != NULL &&
        (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
         reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
        reg_allocation->result.reg == reg) {
        kefir_bool_t occupied, borrowed;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(storage, reg_allocation->result.reg, &occupied));
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, reg_allocation->result.reg, &borrowed));

        if (!borrowed) {
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, reg));
            tmp_reg->evicted = occupied;
            tmp_reg->reg = reg_allocation->result.reg;
            if (occupied) {
                REQUIRE_OK(push_candidate(xasmgen, reg_allocation->result.reg));
            }
            return KEFIR_OK;
        }
    }

    kefir_bool_t occupied, borrowed;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(storage, reg, &occupied));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(storage, reg, &borrowed));
    REQUIRE(!borrowed,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested temporary register has already been borrowed"));

    tmp_reg->evicted = occupied;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(mem, storage, reg));
    if (occupied) {
        REQUIRE_OK(push_candidate(xasmgen, reg));
    }
    tmp_reg->reg = reg;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(
    struct kefir_amd64_xasmgen *xasmgen, const struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    kefir_asm_amd64_xasmgen_register_t reg;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(tmp_reg->reg, &reg));

    if (tmp_reg->evicted) {
        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        } else {
            struct kefir_asm_amd64_xasmgen_operand operand;
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &operand, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&operand, KEFIR_AMD64_SYSV_ABI_QWORD)));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_release_register(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    kefir_asm_amd64_xasmgen_register_t reg;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(tmp_reg->reg, &reg));

    if (tmp_reg->borrowed) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_released(mem, storage, reg));
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(xasmgen, tmp_reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
    struct kefir_codegen_opt_amd64_sysv_storage_location *location,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer codegen storage transform location"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                                    "Expected valid pointer to optimizer codegen register allocation"));

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
            location->reg = reg_allocation->result.reg;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY;
            location->memory.base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RBP;
            location->memory.offset =
                stack_frame_map->offset.spill_area + reg_allocation->result.spill.index * KEFIR_AMD64_SYSV_ABI_QWORD;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY;
            location->memory.base_reg = reg_allocation->result.indirect.base_register;
            location->memory.offset = reg_allocation->result.indirect.offset;
            break;
    }
    return KEFIR_OK;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_codegen_opt_amd64_sysv_storage_location_operand(
    struct kefir_asm_amd64_xasmgen_operand *operand, struct kefir_codegen_opt_amd64_sysv_storage_location *location) {
    REQUIRE(operand != NULL, NULL);
    REQUIRE(location != NULL, NULL);

    switch (location->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            return kefir_asm_amd64_xasmgen_operand_reg(location->reg);

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(location->memory.base_reg), location->memory.offset);
    }
    return NULL;
}

struct register_filter_callback_adapter_payload {
    kefir_codegen_opt_amd64_sysv_storage_acquire_filter_callback_t filter_callback;
    void *filter_payload;
};

static kefir_result_t register_filter_callback_adapter(kefir_asm_amd64_xasmgen_register_t reg, kefir_bool_t *success,
                                                       void *payload) {
    REQUIRE(success != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(const struct register_filter_callback_adapter_payload *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid register filter adapter payload"));

    *success = true;
    if (arg->filter_callback != NULL) {
        struct kefir_codegen_opt_amd64_sysv_storage_location location = {
            .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER, .reg = reg};
        REQUIRE_OK(arg->filter_callback(&location, success, arg->filter_payload));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_acquire(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map, kefir_uint64_t flags,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_codegen_opt_amd64_sysv_storage_handle *handle,
    kefir_codegen_opt_amd64_sysv_storage_acquire_filter_callback_t filter_callback, void *filter_payload) {

    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(handle != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage handle"));

    struct register_filter_callback_adapter_payload filter_adapter_payload = {.filter_callback = filter_callback,
                                                                              .filter_payload = filter_payload};

    kefir_result_t res;
#define CHECK_FLAG(_flag) ((flags & (_flag)) != 0)
    if (CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER_FLAG)) {
        res = kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
            mem, xasmgen, storage, reg_allocation, ((kefir_asm_amd64_xasmgen_register_t) (flags >> 48)),
            &handle->storage_reg);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            handle->reg_allocation = reg_allocation;
            handle->location.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
            handle->location.reg = handle->storage_reg.reg;
            return KEFIR_OK;
        }
    }

    const kefir_bool_t can_allocate_in_memory =
        CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY) && reg_allocation != NULL &&
        (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA ||
         reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT);

    if (CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER)) {
        if (reg_allocation != NULL &&
            reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
                mem, storage, reg_allocation, !CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SHARED_REGISTER),
                &handle->storage_reg, register_filter_callback_adapter, &filter_adapter_payload);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                handle->reg_allocation = reg_allocation;
                handle->location.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
                handle->location.reg = handle->storage_reg.reg;
                return KEFIR_OK;
            }
        }

        if ((!can_allocate_in_memory &&
             !CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER))) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_floating_point_register(
                mem, xasmgen, storage, &handle->storage_reg, register_filter_callback_adapter, &filter_adapter_payload);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                handle->reg_allocation = NULL;
                handle->location.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
                handle->location.reg = handle->storage_reg.reg;
                return KEFIR_OK;
            }
        }
    }

    if (CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER)) {
        if (reg_allocation != NULL &&
            reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_register(
                mem, storage, reg_allocation, !CHECK_FLAG(KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SHARED_REGISTER),
                &handle->storage_reg, register_filter_callback_adapter, &filter_adapter_payload);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                handle->reg_allocation = reg_allocation;
                handle->location.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
                handle->location.reg = handle->storage_reg.reg;
                return KEFIR_OK;
            }
        }

        if (!can_allocate_in_memory) {
            res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
                mem, xasmgen, storage, &handle->storage_reg, register_filter_callback_adapter, &filter_adapter_payload);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                handle->reg_allocation = NULL;
                handle->location.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
                handle->location.reg = handle->storage_reg.reg;
                return KEFIR_OK;
            }
        }
    }

    if (can_allocate_in_memory) {
        if (reg_allocation != NULL &&
            (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA ||
             reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT)) {

            struct kefir_codegen_opt_amd64_sysv_storage_location location = {
                .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY,
                .memory = {
                    .base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RBP,
                }};
            if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA) {
                location.memory.offset = stack_frame_map->offset.spill_area +
                                         reg_allocation->result.spill.index * KEFIR_AMD64_SYSV_ABI_QWORD;
            } else {
                location.memory.offset = reg_allocation->result.indirect.offset;
            }

            kefir_bool_t filter_success = true;
            if (filter_callback != NULL) {
                REQUIRE_OK(filter_callback(&location, &filter_success, filter_payload));
            }

            if (filter_success) {
                handle->reg_allocation = reg_allocation;
                handle->location = location;
                return KEFIR_OK;
            }
        }
    }
#undef CHECK_FLAG

    return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to allocate requested storage");
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_release(
    struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    struct kefir_codegen_opt_amd64_sysv_storage_handle *handle) {

    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(handle != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage handle"));

    if (handle->location.type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, xasmgen, storage, &handle->storage_reg));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_load(
    struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *location) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage location"));

    if (location->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY) {
        struct kefir_codegen_opt_amd64_sysv_storage_location reg_location;
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(&reg_location, stack_frame_map,
                                                                                     reg_allocation));
        REQUIRE(reg_location.memory.base_reg == location->memory.base_reg &&
                    reg_location.memory.offset == location->memory.offset,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Register allocation mismatch for in-memory storage"));
        return KEFIR_OK;
    }

    kefir_asm_amd64_xasmgen_register_t target_reg = location->reg;
    REQUIRE(
        !((reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
           reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
          reg_allocation->result.reg == target_reg),
        KEFIR_OK);

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                                                         kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                                                             &operands[0], stack_frame_map, reg_allocation)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                                                          kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                                                              &operands[0], stack_frame_map, reg_allocation)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                                                            kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                                                                &operands[0], stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                                                          kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                                                              &operands[0], stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                                                          kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                                                              &operands[0], stack_frame_map, reg_allocation)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to load floating-point register into 8/16-bit register");
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_location_store(
    struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *location) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage location"));

    if (location->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY) {
        struct kefir_codegen_opt_amd64_sysv_storage_location reg_location;
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(&reg_location, stack_frame_map,
                                                                                     reg_allocation));
        REQUIRE(reg_location.memory.base_reg == location->memory.base_reg &&
                    reg_location.memory.offset == location->memory.offset,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Register allocation mismatch for in-memory storage"));
        return KEFIR_OK;
    }

    kefir_asm_amd64_xasmgen_register_t source_reg = location->reg;
    REQUIRE(
        !((reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
           reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
          reg_allocation->result.reg == source_reg),
        KEFIR_OK);

    struct kefir_asm_amd64_xasmgen_operand operands[1];

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&operands[0], stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&operands[0], stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&operands[0], stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&operands[0], stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&operands[0], stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to store 8/16-bit register into floating-point register");
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_handle_restore_evicted(
    struct kefir_amd64_xasmgen *xasmgen, const struct kefir_codegen_opt_amd64_sysv_storage_handle *handle) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(handle != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage handle"));

    if (handle->location.type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(xasmgen, &handle->storage_reg));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_handle_mask_evicted(
    struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_amd64_sysv_storage_handle *handle) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(handle != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage handle"));

    if (handle->location.type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER) {
        handle->storage_reg.evicted = false;
    }
    return KEFIR_OK;
}
