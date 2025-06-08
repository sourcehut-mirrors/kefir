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
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_const)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const struct kefir_bigint *bigint;
    REQUIRE_OK(kefir_ir_module_get_bigint(function->module->ir_module, instruction->operation.parameters.imm.bitint_ref,
                                          &bigint));

    kefir_asmcmp_virtual_register_index_t result_vreg;
    const kefir_size_t qword_bits = KEFIR_AMD64_ABI_QWORD * 8;
    if (bigint->bitwidth <= qword_bits) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

        if (instruction->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST) {
            kefir_int64_t value;
            REQUIRE_OK(kefir_bigint_get_signed(bigint, &value));
            if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            }
        } else {
            kefir_uint64_t value;
            REQUIRE_OK(kefir_bigint_get_unsigned(bigint, &value));
            if (value <= KEFIR_INT32_MAX) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            }
        }
    } else {
        const kefir_size_t qwords = (bigint->bitwidth + qword_bits - 1) / qword_bits;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

        for (kefir_size_t i = 0; i < qwords; i++) {
            kefir_uint64_t part;
            REQUIRE_OK(kefir_bigint_get_bits(bigint, i * qword_bits, qword_bits / 2, &part));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                                       &KEFIR_ASMCMP_MAKE_INT(part), NULL));

            REQUIRE_OK(kefir_bigint_get_bits(bigint, i * qword_bits + qword_bits / 2, qword_bits / 2, &part));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD + KEFIR_AMD64_ABI_QWORD / 2,
                                                    KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_INT(part), NULL));
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
