/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define KEFIR_CODEGEN_AMD64_DWARF_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t kefir_codegen_amd64_dwarf_generate_global_identifier_abbrev(
    struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_ir_module *ir_module,
    struct kefir_codegen_amd64_dwarf_context *context, const struct kefir_ir_identifier *identifier) {
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_ir_debug_type_entry(mem, codegen, ir_module, context,
                                                                      identifier->debug_info.type, NULL));

    REQUIRE(context->abbrev.entries.global_variable == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.global_variable = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.global_variable,
                                              KEFIR_DWARF(DW_TAG_variable), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_external), KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_exprloc)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));

    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_dwarf_generate_global_identifier_info(
    struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_ir_module *ir_module,
    struct kefir_codegen_amd64_dwarf_context *context, const struct kefir_ir_identifier *identifier) {
    const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_ir_debug_type_entry(mem, codegen, ir_module, context,
                                                                      identifier->debug_info.type, &type_entry_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, entry_id, context->abbrev.entries.global_variable));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, identifier->symbol));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, identifier->scope != KEFIR_IR_IDENTIFIER_SCOPE_LOCAL));
    switch (identifier->type) {
        case KEFIR_IR_IDENTIFIER_GLOBAL_DATA:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, 1 + KEFIR_AMD64_ABI_QWORD));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_addr)));
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                         kefir_asm_amd64_xasmgen_operand_label(
                                             &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                             identifier->alias != NULL ? identifier->alias : identifier->symbol)));
            break;

        case KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, 2 + KEFIR_AMD64_ABI_QWORD));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_const8u)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_DTPOFF,
                    identifier->alias != NULL ? identifier->alias : identifier->symbol)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_form_tls_address)));
            break;

        case KEFIR_IR_IDENTIFIER_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected IR identifier type");
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY,
                                                       type_entry_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_global_identifiers(
    struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_ir_module *ir_module,
    struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    struct kefir_hashtree_node_iterator iter;
    const struct kefir_ir_identifier *identifier;
    for (const char *identifier_name = kefir_ir_module_identifiers_iter(ir_module, &iter, &identifier);
         identifier_name != NULL; identifier_name = kefir_ir_module_identifiers_next(&iter, &identifier)) {

        if (identifier->type == KEFIR_IR_IDENTIFIER_FUNCTION || identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_IMPORT ||
            identifier->debug_info.type == KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
            continue;
        }

        KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
            REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_global_identifier_abbrev(mem, codegen, ir_module, context,
                                                                                   identifier));
        }

        KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
            REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_global_identifier_info(mem, codegen, ir_module, context,
                                                                                 identifier));
        }
    }

    return KEFIR_OK;
}
