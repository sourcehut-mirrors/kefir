/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/system-v/instr.h"
#include "kefir/codegen/amd64/system-v/abi/vararg.h"
#include "kefir/codegen/amd64/labels.h"
#include "kefir/codegen/amd64/opcodes.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/core/error.h"

static kefir_result_t cg_symbolic_opcode(kefir_iropcode_t opcode, const char **symbolic) {
    REQUIRE(symbolic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to symbolic opcode"));
    *symbolic = kefir_amd64_iropcode_handler(opcode);
    REQUIRE(*symbolic != NULL, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find symbolic opcode representation"));
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_sysv_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                            struct kefir_amd64_sysv_function *sysv_func,
                                            struct kefir_codegen_amd64_sysv_module *sysv_module,
                                            const struct kefir_irblock *block, kefir_size_t instr_index,
                                            kefir_size_t *instr_width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(sysv_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V function"));
    REQUIRE(sysv_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V module"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block"));
    REQUIRE(instr_width != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to instruction width"));

    const struct kefir_irinstr *instr = kefir_irblock_at(&sysv_func->func->body, instr_index);
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to fetch instruction from IR block"));
    *instr_width = 1;
    switch (instr->opcode) {
        case KEFIR_IROPCODE_JMP:
        case KEFIR_IROPCODE_BRANCH: {
            const char *opcode_symbol = NULL;
            REQUIRE(instr->arg.u64 <= kefir_irblock_length(&sysv_func->func->body),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Jump offset is out of IR block bounds"));
            REQUIRE_OK(cg_symbolic_opcode(instr->opcode, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_offset(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[2],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL, sysv_func->func->name)),
                    2 * KEFIR_AMD64_SYSV_ABI_QWORD * instr->arg.u64)));
        } break;

        case KEFIR_IROPCODE_RET: {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_helpers_format(
                        &codegen->xasmgen_helpers, KEFIR_AMD64_SYSV_PROCEDURE_EPILOGUE_LABEL, sysv_func->func->name)),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_INVOKE: {
            kefir_id_t id = (kefir_id_t) instr->arg.u64;
            REQUIRE(kefir_codegen_amd64_sysv_module_function_decl(mem, sysv_module, id, false),
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                    "Failed to allocate AMD64 System-V IR module function decaration"));

            const struct kefir_ir_function_decl *decl = kefir_ir_module_get_declaration(sysv_module->module, id);
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0],
                    decl->name == NULL || decl->vararg
                        ? kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                                 KEFIR_AMD64_SYSV_FUNCTION_GATE_ID_LABEL, id)
                        : kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                                 KEFIR_AMD64_SYSV_FUNCTION_GATE_NAMED_LABEL, decl->name,
                                                                 decl->id)),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_INVOKEV: {
            kefir_id_t id = (kefir_id_t) instr->arg.u64;
            REQUIRE(kefir_codegen_amd64_sysv_module_function_decl(mem, sysv_module, id, true),
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                    "Failed to allocate AMD64 System-V IR module function decaration"));

            const struct kefir_ir_function_decl *decl = kefir_ir_module_get_declaration(sysv_module->module, id);
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0],
                    decl->name == NULL || decl->vararg
                        ? kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                                 KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_ID_LABEL, id)
                        : kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                                 KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_NAMED_LABEL,
                                                                 decl->name)),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_PUSHSTRING: {
            const kefir_id_t identifier = (kefir_id_t) instr->arg.u64;

            kefir_ir_string_literal_type_t type;
            kefir_bool_t public;
            const void *content;
            kefir_size_t length;
            REQUIRE_OK(
                kefir_ir_module_get_string_literal(sysv_module->module, identifier, &type, &public, &content, &length));
            REQUIRE(public, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot push non-public string literal"));

            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL, identifier))));
        } break;

        case KEFIR_IROPCODE_BZERO:
        case KEFIR_IROPCODE_BCOPY: {
            const kefir_id_t type_id = (kefir_id_t) instr->arg.u32[0];
            const kefir_size_t type_index = (kefir_size_t) instr->arg.u32[1];
            struct kefir_abi_sysv_amd64_type_layout *layout =
                kefir_codegen_amd64_sysv_module_type_layout(mem, sysv_module, type_id);
            REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named type"));
            const struct kefir_abi_sysv_amd64_typeentry_layout *entry = NULL;
            REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(layout, type_index, &entry));

            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(instr->opcode, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], entry->size)));
        } break;

        case KEFIR_IROPCODE_EXTUBITS:
        case KEFIR_IROPCODE_EXTSBITS:
        case KEFIR_IROPCODE_INSERTBITS: {
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(instr->opcode, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], instr->arg.u32[0]),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[2], instr->arg.u32[1])));
        } break;

        case KEFIR_IROPCODE_GETLOCAL: {
            const kefir_id_t type_id = (kefir_id_t) instr->arg.u32[0];
            const kefir_size_t type_index = (kefir_size_t) instr->arg.u32[1];
            REQUIRE(type_id == sysv_func->func->locals_type_id,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Requested local variable type does not match actual"));
            struct kefir_abi_sysv_amd64_type_layout *layout =
                kefir_codegen_amd64_sysv_module_type_layout(mem, sysv_module, type_id);
            REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named type"));
            const struct kefir_abi_sysv_amd64_typeentry_layout *entry = NULL;
            REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(layout, type_index, &entry));
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(instr->opcode, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1],
                                                    sysv_func->frame.base.locals + entry->relative_offset)));
        } break;

        case KEFIR_IROPCODE_GETGLOBAL: {
            const kefir_id_t named_symbol = (kefir_id_t) instr->arg.u64;
            const char *symbol = kefir_ir_module_get_named_symbol(sysv_module->module, named_symbol);
            REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to find named symbol"));

            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[1], symbol)));
        } break;

        case KEFIR_IROPCODE_GETTHRLOCAL: {
            const kefir_id_t named_symbol = (kefir_id_t) instr->arg.u64;
            const char *symbol = kefir_ir_module_get_named_symbol(sysv_module->module, named_symbol);
            REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to find named symbol"));

            REQUIRE_OK(kefir_codegen_amd64_sysv_module_declare_tls(mem, sysv_module, symbol));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_SYSV_FUNCTION_TLS_ENTRY, symbol)),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_VARARG_START:
        case KEFIR_IROPCODE_VARARG_COPY:
        case KEFIR_IROPCODE_VARARG_GET:
        case KEFIR_IROPCODE_VARARG_END:
            return kefir_codegen_amd64_sysv_vararg_instruction(mem, codegen, sysv_module, sysv_func, instr);

        case KEFIR_IROPCODE_PUSHF32: {
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol)));
            union {
                kefir_float32_t fp32;
                kefir_uint32_t uint32;
            } value = {.fp32 = instr->arg.f32[0]};
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.uint32),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_PUSHF64: {
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            union {
                kefir_float64_t fp64;
                kefir_uint64_t uint64;
            } value = {.fp64 = instr->arg.f64};
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], value.uint64)));
        } break;

        case KEFIR_IROPCODE_PUSHU64: {
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], instr->arg.u64)));
        } break;

        case KEFIR_IROPCODE_PUSHLABEL: {
            const char *opcode_symbol = NULL;
            REQUIRE(instr->arg.u64 <= kefir_irblock_length(&sysv_func->func->body),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Label offset is out of IR block bounds"));
            REQUIRE_OK(cg_symbolic_opcode(KEFIR_IROPCODE_PUSHI64, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_offset(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[2],
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL, sysv_func->func->name)),
                    2 * KEFIR_AMD64_SYSV_ABI_QWORD * instr->arg.u64)));
        } break;

        case KEFIR_IROPCODE_INLINEASM: {
            kefir_id_t asm_id = instr->arg.u64;
            const struct kefir_ir_inline_assembly *inline_asm =
                kefir_ir_module_get_inline_assembly(sysv_module->module, asm_id);
            REQUIRE(inline_asm != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Invalid IR inline assembly fragment identifier"));
            REQUIRE_OK(kefir_codegen_amd64_sysv_inline_assembly_invoke(mem, sysv_module, codegen, inline_asm));
        } break;

        default: {
            const char *opcode_symbol = NULL;
            REQUIRE_OK(cg_symbolic_opcode(instr->opcode, &opcode_symbol));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], opcode_symbol),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], instr->arg.i64)));
        } break;
    }
    return KEFIR_OK;
}
