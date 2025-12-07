/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/sort.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t decode_constraint(const struct kefir_codegen_target_ir_value_type *value_type,
    kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr,
    void *payload) {
    UNUSED(payload);
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));
    REQUIRE(value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Expected requirement type target IR value constraint"));

    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
            return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Expected requirement type target IR value constraint");

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_GP(value_type->constraint.physical_register);
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_SSE(value_type->constraint.physical_register);
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t do_allocate(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_value_type *value_type,
    const struct kefir_hashset *conflicts,
    const struct kefir_hashset *reserved,
    kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr,
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));
    REQUIRE(conflicts != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator conflict set"));
    REQUIRE(reserved != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator reserved set"));
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));
    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_amd64_regalloc_class *, klass,
        payload);
    UNUSED(klass);

    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_NA;
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                kefir_codegen_target_ir_regalloc_allocation_t alloc = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_GP(value_type->constraint.physical_register);
                // TODO
                // REQUIRE(!kefir_hashset_has(conflicts, (kefir_hashset_key_t) alloc),
                //     KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Conflict in target IR amd64 register allocation constraints"));
                *allocation_ptr = alloc;
                return KEFIR_OK;
            }
            for (kefir_size_t i = 0; i < klass->num_of_gp_registers; i++) {
                kefir_codegen_target_ir_regalloc_allocation_t alloc = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_GP(klass->gp_registers[i]);
                if (!kefir_hashset_has(conflicts, (kefir_hashset_key_t) alloc) && !kefir_hashset_has(reserved, (kefir_hashset_key_t) alloc)) {
                    *allocation_ptr = alloc;
                    return KEFIR_OK;
                }
            }
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_NA; // TODO
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                kefir_codegen_target_ir_regalloc_allocation_t alloc = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_SSE(value_type->constraint.physical_register);
                // TODO
                // REQUIRE(!kefir_hashset_has(conflicts, (kefir_hashset_key_t) alloc),
                //     KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Conflict in target IR amd64 register allocation constraints"));
                *allocation_ptr = alloc;
                return KEFIR_OK;
            }
            for (kefir_size_t i = 0; i < klass->num_of_sse_registers; i++) {
                kefir_codegen_target_ir_regalloc_allocation_t alloc = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_SSE(klass->sse_registers[i]);
                if (!kefir_hashset_has(conflicts, (kefir_hashset_key_t) alloc) && !kefir_hashset_has(reserved, (kefir_hashset_key_t) alloc)) {
                    *allocation_ptr = alloc;
                    return KEFIR_OK;
                }
            }
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_NA; // TODO
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
            *allocation_ptr = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_NA; // TODO
            break;
    }
    *allocation_ptr = 0;
    return KEFIR_OK;
}

static kefir_result_t is_callee_preserved_reg(kefir_abi_amd64_variant_t variant, kefir_asm_amd64_xasmgen_register_t reg,
                                              kefir_bool_t *preserved) {
    const kefir_size_t num_of_regs = kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(variant);
    kefir_asm_amd64_xasmgen_register_t preserved_reg;
    for (kefir_size_t i = 0; i < num_of_regs; i++) {
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(variant, i, &preserved_reg));
        if (preserved_reg == reg) {
            *preserved = true;
            return KEFIR_OK;
        }
    }

    *preserved = false;
    return KEFIR_OK;
}

static kefir_result_t abi_register_comparator(void *ptr1, void *ptr2, kefir_int_t *cmp, void *payload) {
    UNUSED(payload);
    REQUIRE(ptr1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid first register pointer"));
    REQUIRE(ptr2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid second register pointer"));
    REQUIRE(cmp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid comparator payload"));
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg1, ptr1);
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg2, ptr2);
    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_amd64_regalloc_class *, klass,
        payload);

    kefir_bool_t preserved1, preserved2;
    REQUIRE_OK(is_callee_preserved_reg(klass->abi_variant, *reg1, &preserved1));
    REQUIRE_OK(is_callee_preserved_reg(klass->abi_variant, *reg2, &preserved2));

    if (!preserved1 && preserved2) {
        *cmp = -1;
    } else if (preserved1 && !preserved2) {
        *cmp = 1;
    } else if (*reg1 < *reg2) {
        *cmp = -1;
    } else if (*reg1 == *reg2) {
        *cmp = 0;
    } else {
        *cmp = 1;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_regalloc_class_init(struct kefir_mem *mem, struct kefir_codegen_target_ir_amd64_regalloc_class *klass, kefir_abi_amd64_variant_t abi_variant) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR amd64 register allocator class"));

    static const kefir_asm_amd64_xasmgen_register_t AMD64_GENERAL_PURPOSE_REGS[] = {
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_RCX,
        KEFIR_AMD64_XASMGEN_REGISTER_RDX, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI,
        KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9,  KEFIR_AMD64_XASMGEN_REGISTER_R10,
        KEFIR_AMD64_XASMGEN_REGISTER_R11, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
        KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

    static const kefir_size_t NUM_OF_AMD64_GENERAL_PURPOSE_REGS =
        sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]);

    static const kefir_asm_amd64_xasmgen_register_t AMD64_FLOATING_POINT_REGS[] = {
        KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

    static const kefir_size_t NUM_OF_AMD64_FLOATING_POINT_REGS =
        sizeof(AMD64_FLOATING_POINT_REGS) / sizeof(AMD64_FLOATING_POINT_REGS[0]);

    _Static_assert(sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]) <= KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS, "Mismatch betwee number of available amd64 registers");
    _Static_assert(sizeof(AMD64_FLOATING_POINT_REGS) / sizeof(AMD64_FLOATING_POINT_REGS[0]) <= KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS, "Mismatch betwee number of available amd64 registers");

    klass->abi_variant = abi_variant;
    memcpy(klass->gp_registers, AMD64_GENERAL_PURPOSE_REGS, sizeof(kefir_asm_amd64_xasmgen_register_t)* NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    memcpy(klass->sse_registers, AMD64_FLOATING_POINT_REGS, sizeof(kefir_asm_amd64_xasmgen_register_t)* NUM_OF_AMD64_FLOATING_POINT_REGS);
    klass->num_of_gp_registers = NUM_OF_AMD64_GENERAL_PURPOSE_REGS;
    klass->num_of_sse_registers = NUM_OF_AMD64_FLOATING_POINT_REGS;

    REQUIRE_OK(kefir_mergesort(
        mem, klass->gp_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        klass->num_of_gp_registers, abi_register_comparator, klass));
    REQUIRE_OK(kefir_mergesort(
        mem, klass->sse_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        klass->num_of_sse_registers, abi_register_comparator, klass));

    klass->klass.do_allocate = do_allocate;
    klass->klass.decode_constraint = decode_constraint;
    klass->klass.payload = klass;
    return KEFIR_OK;
}
