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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t generate_subprogram_abbrev(struct kefir_codegen_amd64 *codegen,
                                                 struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subprogram == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.subprogram = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.subprogram,
                                              KEFIR_DWARF(DW_TAG_subprogram), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_low_pc), KEFIR_DWARF(DW_FORM_addr)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_high_pc), KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_frame_base),
                                                  KEFIR_DWARF(DW_FORM_exprloc)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_function_info(struct kefir_codegen_amd64_module *codegen_module,
                                             struct kefir_codegen_amd64_dwarf_context *context,
                                             struct kefir_codegen_amd64_function *codegen_function) {
    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers[2];

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_module->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_module->codegen->xasmgen, entry_id, context->abbrev.entries.subprogram));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_module->codegen->xasmgen, ir_identifier->symbol));
    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_DATA(&codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                 kefir_asm_amd64_xasmgen_operand_label(
                                     &xasmgen_helpers[0].operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                     kefir_asm_amd64_xasmgen_helpers_format(
                                         &xasmgen_helpers[0], KEFIR_AMD64_FUNCTION_BEGIN, ir_identifier->symbol))));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &xasmgen_helpers[0].operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &xasmgen_helpers[0].operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers[0], KEFIR_AMD64_FUNCTION_END,
                                                       ir_identifier->symbol)),
            kefir_asm_amd64_xasmgen_operand_label(
                &xasmgen_helpers[0].operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers[1], KEFIR_AMD64_FUNCTION_BEGIN,
                                                       ir_identifier->symbol)))));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_module->codegen->xasmgen, 2));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_module->codegen->xasmgen, KEFIR_DWARF_AMD64_BREG_RBP));
    REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_module->codegen->xasmgen, 0));
    return KEFIR_OK;
}

static kefir_result_t generate_function(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
                                        struct kefir_codegen_amd64_dwarf_context *context,
                                        const struct kefir_ir_function *function) {
    struct kefir_codegen_amd64_function *codegen_function;
    REQUIRE_OK(kefir_codegen_amd64_module_function(codegen_module, function->name, &codegen_function));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_subprogram_abbrev(codegen_module->codegen, context));
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block_content(
            mem, codegen_function, context, codegen_function->function->ir_func->debug_info.subprogram_id));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        REQUIRE_OK(generate_function_info(codegen_module, context, codegen_function));
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block_content(
            mem, codegen_function, context, codegen_function->function->ir_func->debug_info.subprogram_id));
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_module->codegen->xasmgen, KEFIR_DWARF(null)));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS) {
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block_content(
            mem, codegen_function, context, codegen_function->function->ir_func->debug_info.subprogram_id));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_functions(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_module *codegen_module,
                                                            struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *function =
             kefir_ir_module_function_iter(codegen_module->module->ir_module, &iter);
         function != NULL; function = kefir_ir_module_function_next(&iter)) {

        if (function->debug_info.subprogram_id != KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
            REQUIRE_OK(generate_function(mem, codegen_module, context, function));
        }
    }

    return KEFIR_OK;
}
