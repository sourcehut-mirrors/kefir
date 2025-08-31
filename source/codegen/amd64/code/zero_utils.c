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

static kefir_result_t unrolled_zero(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    kefir_asmcmp_virtual_register_index_t target_vreg,
                                    kefir_size_t size) {
    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    if (size % (2 * KEFIR_AMD64_ABI_QWORD) == 0) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        for (kefir_size_t i = 0; i < size; i += 2 * KEFIR_AMD64_ABI_QWORD) {
            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        }
    } else {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_xor(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));
        kefir_size_t i = 0;
        for (; i < ROUND_DOWN_TO(size, KEFIR_AMD64_ABI_QWORD); i += KEFIR_AMD64_ABI_QWORD) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        }

        const kefir_size_t dword_size = KEFIR_AMD64_ABI_QWORD / 2;
        for (; i < ROUND_DOWN_TO(size, dword_size); i += dword_size) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));
        }

        const kefir_size_t word_size = KEFIR_AMD64_ABI_QWORD / 4;
        for (; i < ROUND_DOWN_TO(size, word_size); i += word_size) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(tmp_vreg), NULL));
        }

        for (; i < size; i++) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, i, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
        }
    }

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg, NULL));

    return KEFIR_OK;
}

static kefir_result_t full_zero(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                kefir_asmcmp_virtual_register_index_t target_vreg,
                                kefir_size_t size) {
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
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, count_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         destination_placement_vreg, target_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(source_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(0),
                                      NULL));

    if (size % 8 == 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(count_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_UINT(size / 8), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_stosq_rep(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    } else if (size % 4 == 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(count_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_UINT(size / 4), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_stosl_rep(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    } else if (size % 2 == 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(count_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_UINT(size / 2), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_stosw_rep(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(count_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_UINT(size), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_stosb_rep(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
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

kefir_result_t kefir_codegen_amd64_zero_memory(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                               kefir_asmcmp_virtual_register_index_t target_vreg,
                                               kefir_size_t size) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(target_vreg != KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid target virtual register"));

    if (size <= COPY_UNROLL_LIMIT) {
        REQUIRE_OK(unrolled_zero(mem, function, target_vreg, size));
    } else {
        REQUIRE_OK(full_zero(mem, function, target_vreg, size));
    }
    return KEFIR_OK;
}
