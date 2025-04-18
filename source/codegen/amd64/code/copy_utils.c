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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define COPY_UNROLL_LIMIT 64
#define ROUND_DOWN_TO(_val, _factor) ((_val) / (_factor) * (_factor))

static kefir_result_t unrolled_copy(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    kefir_asmcmp_virtual_register_index_t target_vreg,
                                    kefir_asmcmp_virtual_register_index_t source_vreg, kefir_size_t size) {
    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    if (size % (2 * KEFIR_AMD64_ABI_QWORD) == 0) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));
        for (kefir_size_t i = 0; i < size; i += 2 * KEFIR_AMD64_ABI_QWORD) {
            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        }
    } else {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        kefir_size_t i = 0;
        for (; i < ROUND_DOWN_TO(size, KEFIR_AMD64_ABI_QWORD); i += KEFIR_AMD64_ABI_QWORD) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        }

        const kefir_size_t dword_size = KEFIR_AMD64_ABI_QWORD / 2;
        for (; i < ROUND_DOWN_TO(size, dword_size); i += dword_size) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));
        }

        const kefir_size_t word_size = KEFIR_AMD64_ABI_QWORD / 4;
        for (; i < ROUND_DOWN_TO(size, word_size); i += word_size) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg), NULL));
        }

        for (; i < size; i++) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
        }
    }

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), source_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg, NULL));

    return KEFIR_OK;
}

static kefir_result_t full_copy(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                kefir_asmcmp_virtual_register_index_t target_vreg,
                                kefir_asmcmp_virtual_register_index_t source_vreg, kefir_size_t size) {
    kefir_asmcmp_virtual_register_index_t destination_placement_vreg, source_placement_vreg, count_placement_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &destination_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &count_placement_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, destination_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RDI));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RSI));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, count_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         destination_placement_vreg, target_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         source_placement_vreg, source_vreg, NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    const kefir_size_t qword_count = size / KEFIR_AMD64_ABI_QWORD;
    const kefir_size_t qword_remainder = size % KEFIR_AMD64_ABI_QWORD;
    if (qword_count > 0) {
        if (qword_count >= KEFIR_INT32_MAX) {
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(count_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(qword_count), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(count_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(qword_count), NULL));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_movsq_rep(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    }

    switch (qword_remainder) {
        case 0:
            // Intentionally left blank
            break;

        case 2:
        case 6:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(count_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(qword_remainder / 2), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movsw_rep(mem, &function->code,
                                                    kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case 4:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(count_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movsl_rep(mem, &function->code,
                                                    kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        default:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(count_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(qword_remainder), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movsb_rep(mem, &function->code,
                                                    kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;
    }

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         destination_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), source_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), count_placement_vreg, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_copy_memory(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                               kefir_asmcmp_virtual_register_index_t target_vreg,
                                               kefir_asmcmp_virtual_register_index_t source_vreg, kefir_size_t size) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(source_vreg != KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid source virtual register"));
    REQUIRE(target_vreg != KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid target virtual register"));

    if (size <= COPY_UNROLL_LIMIT) {
        REQUIRE_OK(unrolled_copy(mem, function, target_vreg, source_vreg, size));
    } else {
        REQUIRE_OK(full_copy(mem, function, target_vreg, source_vreg, size));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_load_general_purpose_register(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 kefir_asmcmp_instruction_index_t target_vreg,
                                                                 kefir_asmcmp_instruction_index_t location_vreg,
                                                                 kefir_size_t bytes, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));

    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    switch (bytes) {
        case 8:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
            break;

        case 1:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT), NULL));
            break;

        case 2:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT), NULL));
            break;

        case 3:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(target_vreg), &KEFIR_ASMCMP_MAKE_VREG32(target_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 2, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(16), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            break;

        case 4:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));
            break;

        case 5:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            break;

        case 6:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            break;

        case 7:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 6, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(48), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(target_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                   "Unexpected number of bytes to load into general purpose register");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_store_general_purpose_register(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  kefir_asmcmp_instruction_index_t location_vreg,
                                                                  kefir_asmcmp_instruction_index_t source_vreg,
                                                                  kefir_size_t bytes, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));

    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    switch (bytes) {
        case 8:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_VREG64(source_vreg), NULL));
            break;

        case 1:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(source_vreg), NULL));
            break;

        case 2:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(source_vreg), NULL));
            break;

        case 3:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(source_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, source_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(16), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 2, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
            break;

        case 4:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(source_vreg), NULL));
            break;

        case 5:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(source_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, source_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
            break;

        case 6:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(source_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, source_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg), NULL));
            break;

        case 7:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(source_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, source_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 4, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(16), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset + 6, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                   "Unexpected number of bytes to store from general purpose register");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_load_floating_point_register(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                kefir_asmcmp_instruction_index_t target_vreg,
                                                                kefir_asmcmp_instruction_index_t location_vreg,
                                                                kefir_size_t bytes, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));

    switch (bytes) {
        case 4:
            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                NULL));
            break;

        case 8:
            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(target_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                NULL));
            break;

        default: {
            kefir_asmcmp_virtual_register_index_t tmp_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_codegen_amd64_load_general_purpose_register(mem, function, tmp_vreg, location_vreg, bytes,
                                                                         offset));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg, tmp_vreg,
                NULL));
        } break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_store_floating_point_register(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 kefir_asmcmp_instruction_index_t location_vreg,
                                                                 kefir_asmcmp_instruction_index_t source_vreg,
                                                                 kefir_size_t bytes, kefir_int64_t offset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));

    switch (bytes) {
        case 4:
            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(source_vreg), NULL));
            break;

        case 8:
            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(source_vreg), NULL));
            break;

        default: {
            kefir_asmcmp_virtual_register_index_t tmp_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, source_vreg,
                NULL));
            REQUIRE_OK(kefir_codegen_amd64_store_general_purpose_register(mem, function, location_vreg, tmp_vreg, bytes,
                                                                          offset));
        } break;
    }

    return KEFIR_OK;
}
