#include "kefir/codegen/opt-system-v-amd64/storage.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_init(struct kefir_codegen_opt_sysv_amd64_storage *storage) {
    REQUIRE(storage != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen storage"));

    REQUIRE_OK(kefir_hashtreeset_init(&storage->occupied_general_purpose_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&storage->borrowed_general_purpose_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&storage->borrowed_general_purpose_reg_stack));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_free(struct kefir_mem *mem,
                                                         struct kefir_codegen_opt_sysv_amd64_storage *storage) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));

    REQUIRE_OK(kefir_list_free(mem, &storage->borrowed_general_purpose_reg_stack));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &storage->borrowed_general_purpose_regs));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &storage->occupied_general_purpose_regs));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_used(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(!kefir_asm_amd64_xasmgen_register_is_floating_point(reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected general purpose register"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(!kefir_hashtreeset_has(&storage->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is already occupied"));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is borrowed at the moment"));

    REQUIRE_OK(kefir_hashtreeset_add(mem, &storage->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_unused(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(!kefir_asm_amd64_xasmgen_register_is_floating_point(reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected general purpose register"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(kefir_hashtreeset_has(&storage->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is not occupied"));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register is borrowed at the moment"));

    REQUIRE_OK(kefir_hashtreeset_delete(mem, &storage->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_acquired(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(!kefir_asm_amd64_xasmgen_register_is_floating_point(reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected general purpose register"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(!kefir_hashtreeset_has(&storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register has already been borrowed"));

    REQUIRE_OK(kefir_hashtreeset_add(mem, &storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg));
    REQUIRE_OK(kefir_list_insert_after(mem, &storage->borrowed_general_purpose_reg_stack,
                                       kefir_list_tail(&storage->borrowed_general_purpose_reg_stack),
                                       (void *) (kefir_uptr_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_mark_register_released(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_storage *storage,
    kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(!kefir_asm_amd64_xasmgen_register_is_floating_point(reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected general purpose register"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    REQUIRE(kefir_hashtreeset_has(&storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Requested register has not been borrowed"));

    struct kefir_list_entry *borrow_iter = kefir_list_tail(&storage->borrowed_general_purpose_reg_stack);
    REQUIRE(((kefir_asm_amd64_xasmgen_register_t) (kefir_uptr_t) borrow_iter->value) == reg,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Register is released out of order"));

    REQUIRE_OK(kefir_list_pop(mem, &storage->borrowed_general_purpose_reg_stack, borrow_iter));
    REQUIRE_OK(kefir_hashtreeset_delete(mem, &storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_asm_amd64_xasmgen_register_t reg,
    kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    *result = kefir_hashtreeset_has(&storage->occupied_general_purpose_regs, (kefir_hashtreeset_entry_t) reg);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_is_register_borrowed(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_asm_amd64_xasmgen_register_t reg,
    kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
    *result = kefir_hashtreeset_has(&storage->borrowed_general_purpose_regs, (kefir_hashtreeset_entry_t) reg);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_has_borrowed_registers(
    const struct kefir_codegen_opt_sysv_amd64_storage *storage, kefir_bool_t *result) {
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *result = !kefir_hashtreeset_empty(&storage->borrowed_general_purpose_regs);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_general_purpose_register(
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
    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {

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

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_general_purpose_register(
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

    kefir_result_t res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_general_purpose_register(
        mem, storage, reg_allocation, true, tmp_reg, filter_callback, filter_payload);
    if (res == KEFIR_NO_MATCH) {
        res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
            mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
    }

    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_try_acquire_shared_allocated_general_purpose_register(
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

    kefir_result_t res = kefir_codegen_opt_sysv_amd64_storage_acquire_allocated_general_purpose_register(
        mem, storage, reg_allocation, false, tmp_reg, filter_callback, filter_payload);
    if (res == KEFIR_NO_MATCH) {
        res = kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
            mem, xasmgen, storage, tmp_reg, filter_callback, filter_payload);
    }

    REQUIRE_OK(res);
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
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(candidate)));
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
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
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
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                    xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg)));
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
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }
    tmp_reg->reg = reg;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_restore_evicted_register(
    struct kefir_amd64_xasmgen *xasmgen, struct kefir_codegen_opt_sysv_amd64_storage_register *tmp_reg) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(tmp_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                             "Expected valid pointer optimizer codegen storage temporary register"));

    kefir_asm_amd64_xasmgen_register_t reg;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(tmp_reg->reg, &reg));

    if (tmp_reg->evicted) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        tmp_reg->evicted = false;
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

    if (tmp_reg->evicted) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }
    return KEFIR_OK;
}
