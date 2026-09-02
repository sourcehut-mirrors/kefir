/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/constants.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_generate_local_constants(const struct kefir_codegen_amd64_function_translator_state *translator, const struct kefir_opt_module *module, const struct kefir_opt_function *function, struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator iter;
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&translator->constants, &iter);
    REQUIRE(node != NULL, KEFIR_OK);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    for (; node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, constant_label, node->key);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(
            kefir_ir_module_get_identifier(module->ir_module, function->ir_func->name, &ir_identifier));
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_FLOAT32_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));
                union {
                    kefir_uint32_t u32;
                    kefir_float32_t f32;
                } value = {.f32 = instr->operation.parameters.imm.float32};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u32)));
            } break;

            case KEFIR_OPT_OPCODE_FLOAT64_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));
                union {
                    kefir_uint64_t u64;
                    kefir_float64_t f64;
                } value = {.f64 = instr->operation.parameters.imm.float64};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64)));
            } break;

            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));
                volatile union {
                    kefir_uint64_t u64[2];
                    kefir_long_double_t long_double;
                } value = {.u64 = {0, 0}};

                value.long_double = KEFIR_OPT_PARAMETERS_IMM_GET_LONG_DOUBLE(&instr->operation.parameters);

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64[0])));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64[1])));
            } break;

            case KEFIR_OPT_OPCODE_DECIMAL32_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                         instr->operation.parameters.imm.decimal32.uvalue)));
            } break;

            case KEFIR_OPT_OPCODE_DECIMAL64_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                         instr->operation.parameters.imm.decimal64.uvalue)));
            } break;

            case KEFIR_OPT_OPCODE_DECIMAL128_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 16));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     codegen->symbol_prefix, ir_identifier->symbol,
                                                     constant_label));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                         instr->operation.parameters.imm.decimal128.uvalue[0]),
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1],
                                                         instr->operation.parameters.imm.decimal128.uvalue[1])));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".text", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_generate_global_constants(const struct kefir_codegen_amd64_module *codegen_module, kefir_bool_t *has_rodata) {
    REQUIRE(codegen_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen module"));
    REQUIRE(has_rodata != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    
#define DECLARE_RODATA                                                                           \
    do {                                                                                         \
        if (!*has_rodata) {                                                                      \
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen_module->codegen->xasmgen, ".rodata", \
                                                   KEFIR_AMD64_XASMGEN_SECTION_NOATTR));         \
            *has_rodata = true;                                                                  \
        }                                                                                        \
    } while (0)

    if (codegen_module->constants.float32_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 4));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1593835520)));
    }

    if (codegen_module->constants.float64_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 8));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 2,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[1], 1138753536)));
    }

    if (codegen_module->constants.long_double_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 4));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_LONG_DOUBLE_TO_UINT,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1593835520)));
    }

    if (codegen_module->constants.uint_to_long_double) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1602224128)));
    }

    if (codegen_module->constants.float32_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_FLOAT32_NEG,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.float64_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_FLOAT64_NEG,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.complex_float32_mul) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_MUL,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float32_div) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_DIV,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float64_mul) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_MUL,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_long_double_div) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_LONG_DOUBLE_DIV,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x3fff)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.complex_float32_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float64_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen,
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.copysign_float32) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_COPYSIGNF,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.copysign_float64) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_COPYSIGN,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.isfinite_float32) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_ISFINITEF32_MASK,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2147483647ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_ISFINITEF32_CMP,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2139095039ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.isfinite_float64) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_ISFINITEF64_MASK,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], ~0ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2147483647ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_ISFINITEF64_CMP,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], ~0ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                            kefir_asm_amd64_xasmgen_operand_immu(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0], 2146435071ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.isfinite_long_double) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, KEFIR_AMD64_CONSTANT_ISFINITEL_CMP,
                                             codegen_module->codegen->symbol_prefix));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], ~0ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], ~0ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 32766ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_generate_strings(struct kefir_codegen_amd64 *codegen, struct kefir_opt_module *module,
                                        struct kefir_opt_module_liveness *liveness, kefir_bool_t *has_rodata) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module liveness"));
    REQUIRE(has_rodata != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_ir_module_string_literal_iterator iter;
    kefir_id_t id;
    kefir_ir_string_literal_type_t literal_type;
    kefir_bool_t public;
    const void *content = NULL;
    kefir_size_t length = 0;
    kefir_result_t res = KEFIR_OK;
    for (res = kefir_ir_module_string_literal_iter(module->ir_module, &iter, &id, &literal_type, &public, &content,
                                                   &length);
         res == KEFIR_OK;
         res = kefir_ir_module_string_literal_next(&iter, &id, &literal_type, &public, &content, &length)) {
        if (!public || !kefir_opt_module_is_string_literal_alive(liveness, id)) {
            continue;
        }
        if (!*has_rodata) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            *has_rodata = true;
        }

        switch (literal_type) {
            case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL,
                                                     codegen->symbol_prefix, id));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ASCII, 1,
                                                    kefir_asm_amd64_xasmgen_operand_string_literal(
                                                        &codegen->xasmgen_helpers.operands[0], content, length)));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE16:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 2));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL,
                                                     codegen->symbol_prefix, id));
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, content, length));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE32:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL,
                                                     codegen->symbol_prefix, id));
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, content, length));
                break;
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}
