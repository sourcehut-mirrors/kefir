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

#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

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

const struct kefir_codegen_target_ir_code_class KEFIR_TARGET_AMD64_CODE_CLASS = {
    .opcode_mnemonic = amd64_opcode_mnemonic,
    .register_mnemonic = register_mnemonic,
    .attribute_mnemonic = attribute_mnemonic,
    .resource_mnemonic = resource_mnemonic,
    .is_block_terminator = is_block_terminator,
    .make_unconditional_jump = make_unconditional_jump,
    .finalize_conditional_jump = finalize_conditional_jump,
    .assign_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(assign),
    .touch_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(touch),
    .phi_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(phi),
    .upsilon_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(upsilon),
    .placeholder_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(placeholder),
    .inline_asm_opcode = KEFIR_TARGET_IR_AMD64_OPCODE(inline_assembly),
    .payload = NULL
};
