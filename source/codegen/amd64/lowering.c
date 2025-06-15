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
#include "kefir/codegen/amd64/lowering.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define QWORD_BITS (KEFIR_AMD64_ABI_QWORD * 8)

struct lowering_param {
    struct {
        kefir_id_t bigint_set_signed;
        kefir_id_t bigint_set_unsigned;
        kefir_id_t bigint_cast_signed;
        kefir_id_t bigint_cast_unsigned;
        kefir_id_t bigint_signed_to_float;
        kefir_id_t bigint_unsigned_to_float;
        kefir_id_t bigint_signed_to_double;
        kefir_id_t bigint_unsigned_to_double;
        kefir_id_t bigint_signed_to_long_double;
        kefir_id_t bigint_unsigned_to_long_double;
        kefir_id_t bigint_signed_from_float;
        kefir_id_t bigint_unsigned_from_float;
        kefir_id_t bigint_signed_from_double;
        kefir_id_t bigint_unsigned_from_double;
        kefir_id_t bigint_signed_from_long_double;
        kefir_id_t bigint_unsigned_from_long_double;
        kefir_id_t bigint_is_zero;
    } runtime_fn;
};

#define BIGINT_RUNTIME_FN_IDENTIFIER(_name)                                                                \
    (struct kefir_ir_identifier) {                                                                         \
        .symbol = (_name), .type = KEFIR_IR_IDENTIFIER_FUNCTION, .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL, \
        .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT, .alias = NULL, .debug_info = {               \
            .entry = KEFIR_IR_DEBUG_ENTRY_ID_NONE                                                          \
        }                                                                                                  \
    }

#define DECL_BIGINT_RUNTIME_FN(_id, _name, _params, _returns, _init)                                               \
    static kefir_result_t get_##_id##_function_decl_id(                                                            \
        struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module, struct kefir_opt_module *module, \
        struct lowering_param *param, kefir_id_t *func_decl_id) {                                                  \
        if (param->runtime_fn._id != KEFIR_ID_NONE) {                                                              \
            *func_decl_id = param->runtime_fn._id;                                                                 \
            return KEFIR_OK;                                                                                       \
        }                                                                                                          \
                                                                                                                   \
        kefir_id_t parameters_type_id, returns_type_id;                                                            \
        struct kefir_ir_type *parameters_type =                                                                    \
            kefir_ir_module_new_type(mem, module->ir_module, (_params), &parameters_type_id);                      \
        struct kefir_ir_type *returns_type =                                                                       \
            kefir_ir_module_new_type(mem, module->ir_module, (_returns), &returns_type_id);                        \
        REQUIRE(parameters_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));   \
        REQUIRE(returns_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));      \
        _init                                                                                                      \
                                                                                                                   \
            struct kefir_ir_function_decl *func_decl = kefir_ir_module_new_function_declaration(                   \
                mem, module->ir_module, (_name), parameters_type_id, false, returns_type_id);                      \
        REQUIRE(func_decl != NULL,                                                                                 \
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR function declaration"));            \
                                                                                                                   \
        REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module->ir_module, func_decl->name,                     \
                                                      &BIGINT_RUNTIME_FN_IDENTIFIER(func_decl->name)));            \
                                                                                                                   \
        REQUIRE_OK(kefir_codegen_amd64_module_require_runtime(mem, codegen_module, func_decl->name));              \
                                                                                                                   \
        param->runtime_fn._id = func_decl->id;                                                                     \
        *func_decl_id = func_decl->id;                                                                             \
        return KEFIR_OK;                                                                                           \
    }

DECL_BIGINT_RUNTIME_FN(bigint_set_signed, BIGINT_GET_SET_SIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_set_unsigned, BIGINT_GET_SET_UNSIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_cast_signed, BIGINT_CAST_SIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_cast_unsigned, BIGINT_CAST_UNSIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_float, BIGINT_SIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_float, BIGINT_UNSIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_double, BIGINT_SIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_double, BIGINT_UNSIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_long_double, BIGINT_SIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_long_double, BIGINT_UNSIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_float, BIGINT_SIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_float, BIGINT_UNSIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_double, BIGINT_SIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_double, BIGINT_UNSIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_long_double, BIGINT_SIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_long_double, BIGINT_UNSIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_is_zero, BIGINT_IS_ZERO_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT8, 0, 0));
})

static kefir_result_t new_bitint_type(struct kefir_mem *mem, struct kefir_opt_module *module, kefir_size_t width,
                                      struct kefir_ir_type **type_ptr, kefir_id_t *type_id_ptr) {
    struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, type_id_ptr);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_BITINT, 0, width));
    ASSIGN_PTR(type_ptr, type);
    return KEFIR_OK;
}

static kefir_result_t lower_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
                                        struct kefir_opt_module *module, struct kefir_opt_function *func,
                                        struct lowering_param *param, const struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func);
    UNUSED(instr);
    UNUSED(replacement_ref);

    const kefir_opt_block_id_t block_id = instr->block_id;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST: {
            const struct kefir_bigint *bigint;
            REQUIRE_OK(
                kefir_ir_module_get_bigint(module->ir_module, instr->operation.parameters.imm.bitint_ref, &bigint));
            if (bigint->bitwidth <= QWORD_BITS) {
                kefir_int64_t value;
                REQUIRE_OK(kefir_bigint_get_signed(bigint, &value));
                REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, value, replacement_ref));
            } else {
                // Intentionally left blank
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST: {
            const struct kefir_bigint *bigint;
            REQUIRE_OK(
                kefir_ir_module_get_bigint(module->ir_module, instr->operation.parameters.imm.bitint_ref, &bigint));
            if (bigint->bitwidth <= QWORD_BITS) {
                kefir_uint64_t value;
                REQUIRE_OK(kefir_bigint_get_unsigned(bigint, &value));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, value, replacement_ref));
            } else {
                // Intentionally left blank
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_GET_SIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0],
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_GET_UNSIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0],
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_FROM_SIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
                const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_set_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_FROM_UNSIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
                const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_set_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED:
        case KEFIR_OPT_OPCODE_BITINT_CAST_UNSIGNED: {
            const kefir_bool_t signed_cast = instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED;

            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
            const kefir_size_t src_bitwidth = instr->operation.parameters.src_bitwidth;
            if (bitwidth <= QWORD_BITS && src_bitwidth <= QWORD_BITS) {
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                        mem, &func->code, block_id, arg_ref, 0, MIN(bitwidth, src_bitwidth), replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                        mem, &func->code, block_id, arg_ref, 0, MIN(bitwidth, src_bitwidth), replacement_ref));
                }
            } else if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, arg_ref,
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    &value_ref));
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, value_ref, 0,
                                                                          bitwidth, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, value_ref, 0,
                                                                            bitwidth, replacement_ref));
                }
            } else if (src_bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t casted_arg_ref;
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          src_bitwidth, &casted_arg_ref));
                    REQUIRE_OK(
                        get_bigint_set_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            src_bitwidth, &casted_arg_ref));
                    REQUIRE_OK(
                        get_bigint_set_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                }

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, casted_arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            } else {
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                if (signed_cast) {
                    REQUIRE_OK(
                        get_bigint_cast_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(
                        get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                }

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, MIN(bitwidth, src_bitwidth), NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref, src_bitwidth_ref,
                    init_value_ref, init_value_pair_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, src_bitwidth, &src_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, init_value_ref,
                                                       &init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, init_value_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, src_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_float32(mem, &func->code, block_id, value_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_float32(mem, &func->code, block_id, value_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_float64(mem, &func->code, block_id, value_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_float64(mem, &func->code, block_id, value_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_long_double(mem, &func->code, block_id, value_ref,
                                                                             replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_long_double(mem, &func->code, block_id, value_ref,
                                                                              replacement_ref));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }
            } else {
                kefir_opt_instruction_ref_t value_copy_ref, tmp_ref, init_value_copy_ref, init_value_copy_pair_ref,
                    bitwidth_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &value_copy_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &tmp_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_copy_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_copy_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_copy_ref, init_value_copy_ref,
                                                       &init_value_copy_pair_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
                        REQUIRE_OK(get_bigint_signed_to_float_function_decl_id(mem, codegen_module, module, param,
                                                                               &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                        REQUIRE_OK(get_bigint_unsigned_to_float_function_decl_id(mem, codegen_module, module, param,
                                                                                 &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_signed_to_double_function_decl_id(mem, codegen_module, module, param,
                                                                                &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_unsigned_to_double_function_decl_id(mem, codegen_module, module, param,
                                                                                  &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(get_bigint_signed_to_long_double_function_decl_id(mem, codegen_module, module, param,
                                                                                     &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(get_bigint_unsigned_to_long_double_function_decl_id(mem, codegen_module, module,
                                                                                       param, &func_decl_id));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0,
                                                                      init_value_copy_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, tmp_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
        case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float32_to_int(mem, &func->code, block_id, arg_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float32_to_uint(mem, &func->code, block_id, arg_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float64_to_int(mem, &func->code, block_id, arg_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float64_to_uint(mem, &func->code, block_id, arg_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_long_double_to_int(mem, &func->code, block_id, arg_ref,
                                                                             replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_long_double_to_uint(mem, &func->code, block_id, arg_ref,
                                                                              replacement_ref));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }
            } else {
                kefir_opt_instruction_ref_t value_ref, bitwidth_ref, call_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_float_function_decl_id(mem, codegen_module, module, param,
                                                                                 &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_float_function_decl_id(mem, codegen_module, module, param,
                                                                                   &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_double_function_decl_id(mem, codegen_module, module, param,
                                                                                  &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_double_function_decl_id(mem, codegen_module, module, param,
                                                                                    &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_long_double_function_decl_id(mem, codegen_module, module,
                                                                                       param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_long_double_function_decl_id(mem, codegen_module, module,
                                                                                         param, &func_decl_id));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_TO_BOOL: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            kefir_opt_instruction_ref_t value_ref;
            if (bitwidth <= 8) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_int8_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= 16) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int16_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= 32) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int32_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int64_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else {
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_is_zero_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t call_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_int8_bool_not(mem, &func->code, block_id, call_ref, replacement_ref));
            }
        } break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t lower_function(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
                                     struct kefir_opt_module *module, struct kefir_opt_function *func,
                                     struct lowering_param *param) {
    UNUSED(module);

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {
            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&func->debug_info, instr_id));
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));

            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            REQUIRE_OK(lower_instruction(mem, codegen_module, module, func, param, instr, &replacement_ref));

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr->id));
                REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, instr->id,
                                                                            replacement_ref));
                if (instr->control_flow.prev != KEFIR_ID_NONE || instr->control_flow.next != KEFIR_ID_NONE) {
                    const struct kefir_opt_instruction *replacement_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, replacement_ref, &replacement_instr));
                    if (replacement_instr->control_flow.prev == KEFIR_ID_NONE &&
                        replacement_instr->control_flow.next == KEFIR_ID_NONE) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block->id, instr_id, replacement_ref));
                    }
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                }
                kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
            }

            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_lower_module(struct kefir_mem *mem,
                                                struct kefir_codegen_amd64_module *codegen_module,
                                                struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid memory allocator"));
    REQUIRE(codegen_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid amd64 code generator module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer module"));

    struct lowering_param param = {.runtime_fn = {.bigint_set_signed = KEFIR_ID_NONE,
                                                  .bigint_set_unsigned = KEFIR_ID_NONE,
                                                  .bigint_cast_signed = KEFIR_ID_NONE,
                                                  .bigint_cast_unsigned = KEFIR_ID_NONE,
                                                  .bigint_signed_to_float = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_float = KEFIR_ID_NONE,
                                                  .bigint_signed_to_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_double = KEFIR_ID_NONE,
                                                  .bigint_signed_to_long_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_long_double = KEFIR_ID_NONE,
                                                  .bigint_signed_from_float = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_float = KEFIR_ID_NONE,
                                                  .bigint_signed_from_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_double = KEFIR_ID_NONE,
                                                  .bigint_signed_from_long_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_long_double = KEFIR_ID_NONE,
                                                  .bigint_is_zero = KEFIR_ID_NONE}};
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {

        struct kefir_opt_function *func;
        REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &func));
        REQUIRE_OK(lower_function(mem, codegen_module, module, func, &param));
    }
    return KEFIR_OK;
}
