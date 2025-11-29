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

#define KEFIR_CODEGEN_AMD64_DWARF_INTERNAL
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t generate_label_abbrev(struct kefir_codegen_amd64 *codegen,
                                            struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.label == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.label = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.label,
                                              KEFIR_DWARF(DW_TAG_label), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_low_pc), KEFIR_DWARF(DW_FORM_addr)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_file),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_line),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_column),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_label_info(struct kefir_codegen_amd64_dwarf_context *context,
                                          struct kefir_codegen_amd64_function *codegen_function,
                                          kefir_ir_debug_entry_id_t entry_id,
                                          kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    const struct kefir_ir_debug_entry_attribute *attr;
    const struct kefir_opt_code_debug_info_code_reference *code_reference;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));


    kefir_asmcmp_label_index_t begin_label = KEFIR_ASMCMP_INDEX_NONE;
    kefir_result_t res = kefir_opt_code_debug_info_code_reference(&codegen_function->function->debug_info, attr->code_index, &code_reference);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t iter_key;
        for (res = kefir_hashset_iter(&code_reference->instructions, &iter, &iter_key); res == KEFIR_OK && begin_label == KEFIR_ASMCMP_INDEX_NONE;
            res = kefir_hashset_next(&iter, &iter_key)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, iter_key);

            struct kefir_asmcmp_code_map_fragment_iterator iter;
            const struct kefir_asmcmp_debug_info_code_fragment *fragment;
            for (res = kefir_asmcmp_code_map_fragment_iter(&codegen_function->code.context.debug_info.code_map, instr_ref, &iter, &fragment);
                res == KEFIR_OK && begin_label == KEFIR_ASMCMP_INDEX_NONE;
                res = kefir_asmcmp_code_map_fragment_next(&iter, &fragment)) {
                begin_label = fragment->begin_label;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    if (begin_label == KEFIR_ASMCMP_INDEX_NONE) {
        ASSIGN_PTR(dwarf_entry_id, KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL);
        return KEFIR_OK;
    }

    const kefir_codegen_amd64_dwarf_entry_id_t label_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, label_entry_id);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, label_entry_id,
                                            context->abbrev.entries.label));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL, codegen_function->codegen->config->symbol_prefix,
                                                   ir_identifier->symbol, begin_label))));

    kefir_bool_t has_source_location;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                  &has_source_location));
    if (has_source_location) {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->source_location));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE,
                                                      &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->line));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN,
                                                      &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->column));
    } else {
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, ""));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_label(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function *codegen_function,
                                                        struct kefir_codegen_amd64_dwarf_context *context,
                                                        kefir_ir_debug_entry_id_t entry_id,
                                                        kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    UNUSED(mem);
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_label_abbrev(codegen_function->codegen, context));
        ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.label);
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        REQUIRE_OK(generate_label_info(context, codegen_function, entry_id, dwarf_entry_id));
    }

    return KEFIR_OK;
}
