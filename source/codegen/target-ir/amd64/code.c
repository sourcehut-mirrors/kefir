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

#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t amd64_opcode_mnemonic(kefir_codegen_target_ir_opcode_t opcode, const char **mnemonic_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to mnemonic"));

    switch (opcode) {
#define CASE(_opcode, ...)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        *mnemonic_ptr = #_opcode;            \
        break;

        KEFIR_AMD64_INSTRUCTION_DATABASE(CASE, CASE, CASE, CASE, )
        KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(CASE, )
        KEFIR_CODEGEN_TARGET_IR_SPECIAL_OPCODES(CASE, )
#undef CASE

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t register_mnemonic(kefir_codegen_target_ir_physical_register_t reg, const char **mnemonic_ptr,
                                        void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to opcode mnemonic"));

    *mnemonic_ptr = kefir_asm_amd64_xasmgen_register_symbolic_name((kefir_asm_amd64_xasmgen_register_t) reg);
    REQUIRE(*mnemonic_ptr != NULL, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unknown amd64 register"));
    return KEFIR_OK;
}

static kefir_result_t attribute_mnemonic(kefir_codegen_target_ir_native_id_t attr, const char **mnemonic_ptr,
                                        void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to attribute mnemonic"));

    switch (attr) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(rexW):
        case KEFIR_TARGET_IR_AMD64_OPCODE(data16):
        case KEFIR_TARGET_IR_AMD64_OPCODE(lock):
            REQUIRE_OK(amd64_opcode_mnemonic((kefir_target_ir_amd64_opcode_t) attr, mnemonic_ptr, payload));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Invalid amd64 target IR attribute");
    }
    return KEFIR_OK;
}

static kefir_result_t resource_mnemonic(kefir_codegen_target_ir_resource_id_t resource, const char **mnemonic_ptr,
                                        void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to resource mnemonic"));

    switch (resource) {
        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF:
            *mnemonic_ptr = "flag_cf";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF:
            *mnemonic_ptr = "flag_pf";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF:
            *mnemonic_ptr = "flag_zf";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF:
            *mnemonic_ptr = "flag_sf";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF:
            *mnemonic_ptr = "flag_df";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF:
            *mnemonic_ptr = "flag_of";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK:
            *mnemonic_ptr = "x87_stack";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT:
            *mnemonic_ptr = "x87_fpu_environment";
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_MXCSR:
            *mnemonic_ptr = "mxcsr";
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Invalid amd64 target IR resource");
    }
    return KEFIR_OK;
}

static kefir_result_t is_block_terminator(const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instruction, struct kefir_codegen_target_ir_block_terminator_props *props, void *payload) {
    UNUSED(payload);
    REQUIRE(code != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instruction != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(props != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR block terminator props"));

    props->block_terminator = false;
    props->function_terminator = false;
    props->fallthrough = false;
    props->branch = false;
    props->undefined_target = false;
    props->inline_assembly = false;
    props->target_block_refs[0] = KEFIR_ID_NONE;
    props->target_block_refs[1] = KEFIR_ID_NONE;

    if (instruction->operation.opcode == code->klass->inline_asm_opcode) {
        props->inline_assembly = true;
        struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
        const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(code, &iter, instruction->instr_ref, &fragment);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                    if (fragment->operand.type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF ||
                        fragment->operand.type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF ||
                        (fragment->operand.type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT && fragment->operand.indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS)) {
                        props->block_terminator = true;
                        props->undefined_target = true;
                        props->fallthrough = instruction->operation.inline_asm_node.target_block_ref == KEFIR_ID_NONE;
                        return KEFIR_OK;    
                    }
                    break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    switch (instruction->operation.opcode) {
#define DEF_OPCODE_NOOP(...)
#define DEF_OPCODE0(_opcode, _mnemonic, _branch, _flags) \
        case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
            if ((_flags) & KEFIR_AMD64_INSTRDB_CONTROL_FLOW_TERMINATE_CONTROL_FLOW) { \
                props->block_terminator = true; \
                props->function_terminator = true; \
            } \
            break;
#define DEF_OPCODE1(_opcode, _mnemonic, _branch, _flags, ...) CASE_IS_##_branch(_opcode, _flags)
#define CASE_IS_BRANCH(_opcode, _flags) \
        case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
            if (KEFIR_TARGET_IR_AMD64_OPCODE(_opcode) != KEFIR_TARGET_IR_AMD64_OPCODE(call)) { \
                props->block_terminator = true; \
                props->branch = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode) != KEFIR_TARGET_IR_AMD64_OPCODE(jmp); \
                props->fallthrough = true; \
                props->fallthrough = ((_flags) & KEFIR_AMD64_INSTRDB_CONTROL_FLOW_JUMP_FALLTHROUGH) != 0 && \
                    instruction->operation.parameters[1].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF; \
                if (instruction->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF) { \
                    props->target_block_refs[0] = instruction->operation.parameters[0].block_ref; \
                    if (instruction->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF) { \
                        props->target_block_refs[1] = instruction->operation.parameters[1].block_ref; \
                    } \
                } else { \
                    props->undefined_target = true; \
                } \
            } \
            break;
#define CASE_IS_(...)

        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE0, DEF_OPCODE1, DEF_OPCODE_NOOP, DEF_OPCODE_NOOP,)
#undef DEF_OPCODE_NOOP
#undef DEF_OPCODE1
#undef DEF_OPCODE0

        case KEFIR_TARGET_IR_AMD64_OPCODE(tail_call):
            props->block_terminator = true;
            props->function_terminator = true;
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t make_unconditional_jump(kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_codegen_target_ir_operation *operation_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(operation_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR operation"));

    operation_ptr->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jmp);
    operation_ptr->parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF;
    operation_ptr->parameters[0].block_ref = block_ref;
    operation_ptr->parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE;
    operation_ptr->parameters[2].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE;
    operation_ptr->parameters[3].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE;
    return KEFIR_OK;
}

static kefir_result_t finalize_conditional_jump(const struct kefir_codegen_target_ir_operation *operation, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_codegen_target_ir_operation *operation_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(operation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR operation"));
    REQUIRE(operation_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR operation"));

    *operation_ptr = *operation;
    operation_ptr->parameters[2] = operation_ptr->parameters[1];
    operation_ptr->parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF;
    operation_ptr->parameters[1].block_ref = block_ref;
    return KEFIR_OK;
}


static kefir_uint64_t CONSUMED_FLAGS[KEFIR_TARGET_IR_AMD64_OPCODE(num_of_opcodes) + 1] = {
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmova)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovae)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovbe)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmove)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovg)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovge)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovl)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovle)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovne)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovs)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovns)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmovnz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(sete)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setne)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setnp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setg)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setge)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setl)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setle)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(seta)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setae)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setbe)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setnb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(seto)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setno)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(sets)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setns)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(setnc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(adc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(sbb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(pushfq)] = 
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(movsb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(movsw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(movsl)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(movsq)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(stosb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(stosw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(stosl)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(stosq)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(jz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jnz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(je)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jne)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jo)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(js)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jns)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jnc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jno)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jnp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(ja)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jae)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jnb)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jbe)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jg)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jge)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jl)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(jle)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(fld)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fild)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdecstp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fldz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fld1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fistp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fstp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fst)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fabs)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fadd)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fadd2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(faddp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(faddp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubrp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmul1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmul2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdiv2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivr2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivrp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivrp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fchs)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fxam)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fxch)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fucomi)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fucomip)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomi)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomip)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomip2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcmove)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),

    [KEFIR_TARGET_IR_AMD64_OPCODE(fnstenv)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fstcw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fnstcw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fnstsw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),

    [KEFIR_TARGET_IR_AMD64_OPCODE(stmxcsr)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_MXCSR)
};

#define ARITH_FLAGS ((1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) | \
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | \
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | \
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF) | \
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF))
static kefir_uint64_t PRODUCED_FLAGS[KEFIR_TARGET_IR_AMD64_OPCODE(num_of_opcodes) + 1] = {
#define ALL_FLAGS \
    ((1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) | \
    (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF) | \
    (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) | \
    (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) | \
    (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF) | \
    (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF))
    [KEFIR_TARGET_IR_AMD64_OPCODE(popfq)] = ALL_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(tail_call)] = ALL_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(call)] = ALL_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(inline_assembly)] = ALL_FLAGS,
#undef ALL_FLAGS

    [KEFIR_TARGET_IR_AMD64_OPCODE(cld)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(test)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)
        | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
        | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF)
        | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF)
        | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(btc)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmp)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(cmpxchg)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(add)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(xadd)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(adc)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(sub)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(sbb)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(mul)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(imul)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(imul1)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(imul3)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(idiv)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(div)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(shl)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(shld)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(shr)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(shrd)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(sar)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(and)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(or)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(xor)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(neg)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(dec)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) |
        (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF),
    [KEFIR_TARGET_IR_AMD64_OPCODE(bsf)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(bsr)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(rol)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF),

    [KEFIR_TARGET_IR_AMD64_OPCODE(ucomiss)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(ucomisd)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(comiss)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(comisd)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(fucomi)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(fucomip)] = ARITH_FLAGS | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomi)] = ARITH_FLAGS,
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomip)] = ARITH_FLAGS | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcomip2)] = ARITH_FLAGS | (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    
    [KEFIR_TARGET_IR_AMD64_OPCODE(fld)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fild)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdecstp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fldz)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fld1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fistp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fstp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fabs)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fadd)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fadd2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(faddp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(faddp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubrp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fsubp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmul1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmul2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fmulp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdiv2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivr2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivrp1)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fdivrp2)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fchs)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fxch)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fcmove)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK),

    [KEFIR_TARGET_IR_AMD64_OPCODE(fldenv)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fldcw)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),
    [KEFIR_TARGET_IR_AMD64_OPCODE(fnclex)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT),

    [KEFIR_TARGET_IR_AMD64_OPCODE(ldmxcsr)] = (1ull << KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_MXCSR)
};
#undef ARITH_FLAGS

kefir_result_t instruction_resources(kefir_codegen_target_ir_opcode_t opcode, kefir_uint64_t *produced_resources, kefir_uint64_t *consumed_resources, void *payload) {
    UNUSED(payload);
    REQUIRE(opcode < KEFIR_TARGET_IR_AMD64_OPCODE(num_of_opcodes), KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR opcode"));

    ASSIGN_PTR(produced_resources, PRODUCED_FLAGS[opcode]);
    ASSIGN_PTR(consumed_resources, CONSUMED_FLAGS[opcode]);
    return KEFIR_OK;
}

static kefir_result_t classify_instruction(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    struct kefir_codegen_target_ir_instruction_destruction_classification *classification, void *payload) {
    UNUSED(payload);
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction classification"));

    const struct kefir_codegen_target_ir_instruction *instruction;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instruction));

    memset(classification, 0, sizeof(struct kefir_codegen_target_ir_instruction_destruction_classification));
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE;
        classification->operands[i].implicit = false;
    }

    switch (instruction->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(assign):
        case KEFIR_TARGET_IR_AMD64_OPCODE(upsilon):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(placeholder):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(produce_virtual_register);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(touch):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(tail_call):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(tail_call);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(data_word):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(data_word);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(phi):
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to classify target IR phi node");

        default:
            // Intentionally left blank
            break;
    }

    kefir_bool_t implicit_params = false;
    kefir_size_t num_of_params = 0;

#define CLASSIFY_OP(_op, _index) \
    do { \
        if (((_op) & KEFIR_AMD64_INSTRDB_READ) && ((_op) & KEFIR_AMD64_INSTRDB_WRITE)) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE; \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_READ) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_WRITE) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE; \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_FPU_STACK) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
        } \
    } while (0)

    switch (instruction->operation.opcode) {
#define INSTR0(_opcode, _mnemonic, _variant, _flags)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        break;

#define INSTR1(_opcode, _mnemonic, _variant, _flags, _op1)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 1; \
        break;

#define INSTR2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 2; \
        break;

#define INSTR3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        CLASSIFY_OP(_op2, 2); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 3; \
        break;

#define INSTR0_VIRT(_opcode, _mnemonic)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode); \
        break;

        KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(INSTR0_VIRT, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(INSTR0, INSTR1, INSTR2, INSTR3, )
#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }

    kefir_size_t output_index = 0;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr_ref, output_index++, NULL, &value_type));
            if (value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT || value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) {
                classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            }
        }
    }

    switch (instruction->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(stosb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cmpxchg): {
            REQUIRE(num_of_params == 2 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[2].implicit = true;
            classification->operands[2].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
        } break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(mul):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul1):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, instr_ref, 1, &value_ref, NULL);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(div):
        case KEFIR_TARGET_IR_AMD64_OPCODE(idiv):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, instr_ref, 1, &value_ref, NULL);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cwd):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            break;
            
        case KEFIR_TARGET_IR_AMD64_OPCODE(cdq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cqo):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            break;
    }


    classification->produced_resources = PRODUCED_FLAGS[classification->opcode];
    classification->consumed_resources = CONSUMED_FLAGS[classification->opcode];

    return KEFIR_OK;
}

const struct kefir_codegen_target_ir_code_class KEFIR_TARGET_AMD64_CODE_CLASS = {
    .opcode_mnemonic = amd64_opcode_mnemonic,
    .register_mnemonic = register_mnemonic,
    .attribute_mnemonic = attribute_mnemonic,
    .resource_mnemonic = resource_mnemonic,
    .is_block_terminator = is_block_terminator,
    .make_unconditional_jump = make_unconditional_jump,
    .finalize_conditional_jump = finalize_conditional_jump,
    .classify_instruction = classify_instruction,
    .instruction_resources = instruction_resources,
    .assign_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(assign),
    .touch_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(touch),
    .phi_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(phi),
    .upsilon_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(upsilon),
    .placeholder_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(placeholder),
    .inline_asm_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(inline_assembly),
    .payload = NULL
};
