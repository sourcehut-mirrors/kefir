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
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t is_local_var(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                   kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *res, kefir_int64_t *offset) {
    const struct kefir_opt_instruction *location_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code, instr_ref, &location_instr));
    if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        *res = true;
        REQUIRE_OK(kefir_codegen_amd64_function_local_variable_offset(
            mem, function, location_instr->operation.parameters.type.type_index, true, offset));
    } else if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        *res = true;
        const kefir_int64_t ref_offset = location_instr->operation.parameters.offset;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code,
                                                  location_instr->operation.parameters.refs[0], &location_instr));
        REQUIRE_OK(kefir_codegen_amd64_function_local_variable_offset(
            mem, function, location_instr->operation.parameters.type.type_index, true, offset));
        *offset += ref_offset;
    } else {
        *res = false;
    }
    return KEFIR_OK;
}

#define DEFINE_STORE(_width)                                                                                       \
    do {                                                                                                           \
        kefir_asmcmp_virtual_register_index_t value_vreg;                                                          \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(                                                           \
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_vreg));    \
        const struct kefir_asmcmp_virtual_register *value = NULL;                                                  \
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, value_vreg, &value));                \
                                                                                                                   \
        kefir_bool_t local_var;                                                                                    \
        kefir_int64_t load_offset;                                                                                 \
        REQUIRE_OK(is_local_var(mem, function,                                                                     \
                                instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],      \
                                &local_var, &load_offset));                                                        \
        struct kefir_asmcmp_value target_value;                                                                    \
        if (local_var) {                                                                                           \
            target_value =                                                                                         \
                KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, KEFIR_ASMCMP_OPERAND_VARIANT_##_width##BIT);     \
        } else {                                                                                                   \
            kefir_asmcmp_virtual_register_index_t target_vreg;                                                     \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(                                                       \
                function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],            \
                &target_vreg));                                                                                    \
            target_value =                                                                                         \
                KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_##_width##BIT);    \
        }                                                                                                          \
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,                                                    \
                                          kefir_asmcmp_context_instr_tail(&function->code.context), &target_value, \
                                          &KEFIR_ASMCMP_MAKE_VREG##_width(value_vreg), NULL));                     \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int8_store)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_STORE(8);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int16_store)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_STORE(16);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int32_store)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t value_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
        function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_vreg));
    const struct kefir_asmcmp_virtual_register *value = NULL;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, value_vreg, &value));

    kefir_asmcmp_operand_variant_t target_variant = value->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT
                                                        ? KEFIR_ASMCMP_OPERAND_VARIANT_32BIT
                                                        : KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;

    kefir_bool_t local_var;
    kefir_int64_t load_offset;
    REQUIRE_OK(is_local_var(mem, function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            &local_var, &load_offset));
    struct kefir_asmcmp_value target_value;
    if (local_var) {
        target_value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, target_variant);
    } else {
        kefir_asmcmp_virtual_register_index_t target_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], &target_vreg));
        target_value = KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, target_variant);
    }

    if (value->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context), &target_value,
                                          &KEFIR_ASMCMP_MAKE_VREG32(value_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movd(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context), &target_value,
                                           &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int64_store)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t value_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
        function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_vreg));
    const struct kefir_asmcmp_virtual_register *value = NULL;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, value_vreg, &value));

    kefir_asmcmp_operand_variant_t target_variant = value->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT
                                                        ? KEFIR_ASMCMP_OPERAND_VARIANT_64BIT
                                                        : KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;

    kefir_bool_t local_var;
    kefir_int64_t load_offset;
    REQUIRE_OK(is_local_var(mem, function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            &local_var, &load_offset));
    struct kefir_asmcmp_value target_value;
    if (local_var) {
        target_value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, target_variant);
    } else {
        kefir_asmcmp_virtual_register_index_t target_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], &target_vreg));
        target_value = KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, target_variant);
    }

    if (value->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context), &target_value,
                                          &KEFIR_ASMCMP_MAKE_VREG64(value_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movq(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context), &target_value,
                                           &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_bool_t local_var;
    kefir_int64_t load_offset;
    REQUIRE_OK(is_local_var(mem, function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            &local_var, &load_offset));
    struct kefir_asmcmp_value target_value, target_value2;
    if (local_var) {
        target_value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset + KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_64BIT);
        target_value2 = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT);
    } else {
        kefir_asmcmp_virtual_register_index_t target_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], &target_vreg));
        target_value =
            KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT);
        target_value2 = KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT);
    }

    kefir_asmcmp_virtual_register_index_t value_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
        function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &target_value, &KEFIR_ASMCMP_MAKE_INT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &target_value2, NULL));

    return KEFIR_OK;
}

#define DEFINE_LOAD(_suffix, _width)                                                                               \
    do {                                                                                                           \
        kefir_bool_t local_var;                                                                                    \
        kefir_int64_t load_offset;                                                                                 \
        REQUIRE_OK(is_local_var(mem, function,                                                                     \
                                instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],      \
                                &local_var, &load_offset));                                                        \
        struct kefir_asmcmp_value source_value;                                                                    \
        if (local_var) {                                                                                           \
            source_value =                                                                                         \
                KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, KEFIR_ASMCMP_OPERAND_VARIANT_##_width##BIT);     \
        } else {                                                                                                   \
            kefir_asmcmp_virtual_register_index_t source_vreg;                                                     \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(                                                       \
                function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],            \
                &source_vreg));                                                                                    \
            source_value =                                                                                         \
                KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_##_width##BIT);    \
        }                                                                                                          \
        kefir_asmcmp_virtual_register_index_t value_vreg;                                                          \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                 \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &value_vreg)); \
        REQUIRE_OK(kefir_asmcmp_amd64_mov##_suffix(mem, &function->code,                                           \
                                                   kefir_asmcmp_context_instr_tail(&function->code.context),       \
                                                   &KEFIR_ASMCMP_MAKE_VREG64(value_vreg), &source_value, NULL));   \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, value_vreg));          \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int8_load_signed)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(sx, 8);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int8_load_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(zx, 8);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int16_load_signed)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(sx, 16);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int16_load_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(zx, 16);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int32_load_signed)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(sx, 32);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int32_load_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_bool_t local_var;
    kefir_int64_t load_offset;
    REQUIRE_OK(is_local_var(mem, function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            &local_var, &load_offset));
    struct kefir_asmcmp_value source_value;
    if (local_var) {
        source_value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT);
    } else {
        kefir_asmcmp_virtual_register_index_t source_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], &source_vreg));
        source_value = KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT);
    }

    kefir_asmcmp_virtual_register_index_t value_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &value_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(value_vreg), &source_value, NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, value_vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int64_load)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_LOAD(, 64);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_load)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_bool_t local_var;
    kefir_int64_t load_offset;
    REQUIRE_OK(is_local_var(mem, function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            &local_var, &load_offset));
    struct kefir_asmcmp_value source_value;
    if (local_var) {
        source_value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(load_offset, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT);
    } else {
        kefir_asmcmp_virtual_register_index_t source_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], &source_vreg));
        source_value = KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT);
    }

    kefir_asmcmp_virtual_register_index_t value_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &value_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &source_value, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, value_vreg));

    return KEFIR_OK;
}
