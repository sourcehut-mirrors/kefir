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
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/version.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_dwarf_context_generate_compile_unit(
    struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
    struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        context->abbrev.entries.compile_unit = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
        REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_module->codegen->xasmgen,
                                                  context->abbrev.entries.compile_unit,
                                                  KEFIR_DWARF(DW_TAG_compile_unit), KEFIR_DWARF(DW_CHILDREN_yes)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_AT_language),
                                                      KEFIR_DWARF(DW_FORM_data2)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_AT_producer),
                                                      KEFIR_DWARF(DW_FORM_string)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_AT_low_pc),
                                                      KEFIR_DWARF(DW_FORM_addr)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_AT_high_pc),
                                                      KEFIR_DWARF(DW_FORM_data8)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_AT_stmt_list),
                                                      KEFIR_DWARF(DW_FORM_sec_offset)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_module->codegen->xasmgen));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        context->info.entries.compile_unit = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
        REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_module->codegen->xasmgen, context->info.entries.compile_unit,
                                                context->abbrev.entries.compile_unit));
        REQUIRE_OK(KEFIR_AMD64_DWARF_WORD(&codegen_module->codegen->xasmgen, KEFIR_DWARF(DW_LANG_C11)));
        REQUIRE_OK(
            KEFIR_AMD64_DWARF_STRING(&codegen_module->codegen->xasmgen, "Kefir C compiler " KEFIR_VERSION_SHORT));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                            kefir_asm_amd64_xasmgen_operand_label(
                                                &codegen_module->codegen->xasmgen_helpers.operands[0],
                                                KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, KEFIR_AMD64_TEXT_SECTION_BEGIN)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_subtract(
                &codegen_module->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_label(&codegen_module->codegen->xasmgen_helpers.operands[1],
                                                      KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                      KEFIR_AMD64_TEXT_SECTION_END),
                kefir_asm_amd64_xasmgen_operand_label(&codegen_module->codegen->xasmgen_helpers.operands[2],
                                                      KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                      KEFIR_AMD64_TEXT_SECTION_BEGIN))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_label(&codegen_module->codegen->xasmgen_helpers.operands[0],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, KEFIR_AMD64_DWARF_DEBUG_LINES)));
    }

    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_global_identifiers(mem, codegen_module->codegen,
                                                                     codegen_module->module->ir_module, context));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_functions(mem, codegen_module, context));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_types(mem, codegen_module->codegen, codegen_module->module->ir_module,
                                                        context));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_module->codegen->xasmgen, 0));
    }

    return KEFIR_OK;
}
