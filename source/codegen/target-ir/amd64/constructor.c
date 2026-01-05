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

#include "kefir/codegen/target-ir/amd64/constructor.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t is_jump(const struct kefir_asmcmp_context *context, kefir_asmcmp_instruction_index_t instr_idx, kefir_bool_t *is_jump_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(is_jump_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *is_jump_ptr = false;

    struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_idx, &instr));
    if (instr->opcode == KEFIR_ASMCMP_AMD64_OPCODE(inline_assembly)) {
        struct kefir_asmcmp_inline_assembly_fragment_iterator iter;
        kefir_result_t res;
        for (res =
                    kefir_asmcmp_inline_assembly_fragment_iter(context, instr->args[0].inline_asm_idx, &iter);
                res == KEFIR_OK && iter.fragment != NULL; res = kefir_asmcmp_inline_assembly_fragment_next(&iter)) {

            switch (iter.fragment->type) {
                case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intentionally left blank
                    break;

                case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_VALUE:
                    if (iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL ||
                        iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL ||
                        (iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT && iter.fragment->value.indirect.type == KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS)) {
                        *is_jump_ptr = true;
                        return KEFIR_OK;
                    }
                    break;
            }
        }
        REQUIRE_OK(res);
    }

    switch (instr->opcode){
#define DEF_OPCODE0(_opcode, _mnemonic, _variant, _flags) \
        case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
            *is_jump_ptr = ((_flags) & (KEFIR_AMD64_INSTRDB_CONTROL_FLOW_JUMP | KEFIR_AMD64_INSTRDB_CONTROL_FLOW_JUMP_FALLTHROUGH | KEFIR_AMD64_INSTRDB_CONTROL_FLOW_TERMINATE_CONTROL_FLOW)) != 0; \
            break;
#define DEF_OPCODE(_opcode, _mnemonic, _variant, _flags, ...) DEF_OPCODE0(_opcode, _mnemonic, _variant, _flags)
        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE0, DEF_OPCODE, DEF_OPCODE, DEF_OPCODE,)
#undef DEF_OPCODE
#undef DEF_OPCODE0
    }
    return KEFIR_OK;
}

static kefir_result_t classify_instruction(const struct kefir_asmcmp_instruction *instruction,
    struct kefir_codegen_target_ir_asmcmp_instruction_classification *classification, void *payload) {
    UNUSED(payload);
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));

    memset(classification, 0, sizeof(struct kefir_codegen_target_ir_asmcmp_instruction_classification));
    classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SPECIAL_NONE;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE;
        classification->operands[i].implicit = false;
        classification->operands[i].index = 0;
    }
    classification->extra_flags = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_NONE;

    switch (instruction->opcode) {
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_LINK;
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[0].index = 0;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].index = 1;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(produce_virtual_register):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_PRODUCE;
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_TOUCH;
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(data_word):
            classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(data_word);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(weak_touch_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(noop):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_begin):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_end):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SKIP;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(tail_call):
            classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(tail_call);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(rexW):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_ATTRIBUTE;
            classification->attribute = KEFIR_TARGET_IR_AMD64_OPCODE(rexW);
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(data16):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_ATTRIBUTE;
            classification->attribute = KEFIR_TARGET_IR_AMD64_OPCODE(data16);
            return KEFIR_OK;

        case KEFIR_ASMCMP_AMD64_OPCODE(lock):
            classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_ATTRIBUTE;
            classification->attribute = KEFIR_TARGET_IR_AMD64_OPCODE(lock);
            return KEFIR_OK;

        default:
            // Intentionally left blank
            break;
    }

#define CLASSIFY_OP(_op, _index) \
    do { \
        if (((_op) & KEFIR_AMD64_INSTRDB_READ) && ((_op) & KEFIR_AMD64_INSTRDB_WRITE)) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_READ) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_WRITE) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_FPU_STACK) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
            classification->operands[(_index)].index = (_index); \
        } \
    } while (0)

    kefir_bool_t implicit_params = false;
    kefir_size_t num_of_params = 0;
    kefir_bool_t consumes_flags = false;
    kefir_bool_t produces_flags = false;
    switch (instruction->opcode) {
#define INSTR0(_opcode, _mnemonic, _variant, _flags)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        consumes_flags = ((_flags) & KEFIR_AMD64_INSTRDB_CONSUME_FLAGS) != 0; \
        produces_flags = ((_flags) & KEFIR_AMD64_INSTRDB_PRODUCE_FLAGS) != 0; \
        break;

#define INSTR1(_opcode, _mnemonic, _variant, _flags, _op1)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        consumes_flags = ((_flags) & KEFIR_AMD64_INSTRDB_CONSUME_FLAGS) != 0; \
        produces_flags = ((_flags) & KEFIR_AMD64_INSTRDB_PRODUCE_FLAGS) != 0; \
        num_of_params = 1; \
        break;

#define INSTR2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        consumes_flags = ((_flags) & KEFIR_AMD64_INSTRDB_CONSUME_FLAGS) != 0; \
        produces_flags = ((_flags) & KEFIR_AMD64_INSTRDB_PRODUCE_FLAGS) != 0; \
        num_of_params = 2; \
        break;

#define INSTR3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        CLASSIFY_OP(_op2, 2); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        consumes_flags = ((_flags) & KEFIR_AMD64_INSTRDB_CONSUME_FLAGS) != 0; \
        produces_flags = ((_flags) & KEFIR_AMD64_INSTRDB_PRODUCE_FLAGS) != 0; \
        num_of_params = 3; \
        break;

#define INSTR0_VIRT(_opcode, _mnemonic)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        break;

        KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(INSTR0_VIRT, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(INSTR0, INSTR1, INSTR2, INSTR3, )
#undef INSTR0_VIRT
#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
            instruction->args[i].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
            (instruction->args[i].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_8BIT || instruction->args[i].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_16BIT)) {
            classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
        }
    }

    if (consumes_flags) {
        classification->extra_flags = classification->extra_flags | KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_CONSUMES_RESOURCES;
        classification->consumed_resources = 1;
    }
    if (produces_flags) {
        classification->extra_flags = classification->extra_flags | KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_PRODUCES_RESOURCES;
        classification->produced_resources = 1;
    }

    switch (instruction->opcode) {
        case KEFIR_ASMCMP_AMD64_OPCODE(movsb):
        case KEFIR_ASMCMP_AMD64_OPCODE(movsw):
        case KEFIR_ASMCMP_AMD64_OPCODE(movsl):
        case KEFIR_ASMCMP_AMD64_OPCODE(movsq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(stosb):
        case KEFIR_ASMCMP_AMD64_OPCODE(stosw):
        case KEFIR_ASMCMP_AMD64_OPCODE(stosl):
        case KEFIR_ASMCMP_AMD64_OPCODE(stosq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            break;

#define MATCH_VARIANT(_operand, _arg) \
        do { \
            kefir_asmcmp_operand_variant_t native_variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT; \
            if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER) { \
                native_variant = (_arg)->vreg.variant; \
            } else if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT) { \
                native_variant = (_arg)->indirect.variant; \
            } else if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || (_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) { \
                native_variant = (_arg)->rip_indirection.variant; \
            } \
            switch (native_variant) { \
                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE; \
                    break; \
                     \
                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE; \
                    break; \
            } \
        } while (0)
        case KEFIR_ASMCMP_AMD64_OPCODE(cmpxchg): {
            REQUIRE(num_of_params == 2 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[2].implicit = true;
            classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
        } break;

        case KEFIR_ASMCMP_AMD64_OPCODE(mul):
        case KEFIR_ASMCMP_AMD64_OPCODE(imul):
        case KEFIR_ASMCMP_AMD64_OPCODE(imul1):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                MATCH_VARIANT(&classification->operands[1], &instruction->args[0]);
                if ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
                    instruction->args[0].vreg.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                    (instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                    instruction->args[0].indirect.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                    ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) &&
                    instruction->args[0].rip_indirection.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT)) {
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                    MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
                }
            }
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(div):
        case KEFIR_ASMCMP_AMD64_OPCODE(idiv):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                MATCH_VARIANT(&classification->operands[1], &instruction->args[0]);
                if ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
                    instruction->args[0].vreg.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                    (instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                    instruction->args[0].indirect.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                    ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) &&
                    instruction->args[0].rip_indirection.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT)) {
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                    MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
                }
            }
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(cwd):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT;
            break;
            
        case KEFIR_ASMCMP_AMD64_OPCODE(cdq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(cqo):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t compute_vreg_pair_part_spill_offset(kefir_asmcmp_virtual_register_pair_type_t pair_type, kefir_size_t index, kefir_size_t *offset_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(offset_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to offset"));
    REQUIRE(index <= 1, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid index of virtual register pair components"));

    switch (pair_type) {
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERIC:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to calculate spil area offset for component of generic pair");

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE:
            *offset_ptr = index * 4;
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE:
            *offset_ptr = index * 8;
            break;
    }
    return KEFIR_OK;
}

const struct kefir_codegen_target_ir_code_constructor_class KEFIR_TARGET_AMD64_CODE_CONSTRUCTOR_CLASS = {
    .is_jump = is_jump,
    .classify_instruction = classify_instruction,
    .compute_vreg_pair_part_spill_offset = compute_vreg_pair_part_spill_offset,
    .payload = NULL
};
