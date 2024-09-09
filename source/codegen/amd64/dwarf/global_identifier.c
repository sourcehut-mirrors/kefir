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
    kefir_codegen_amd64_dwarf_entry_id_t variable_entry_id = identifier->debug_info.entry;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.global_variable == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.global_variable = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.global_variable,
                                              KEFIR_DWARF(DW_TAG_variable), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_external), KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_declaration),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_exprloc)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_linkage_name),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_file),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_line),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    ;
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_column),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));

    return KEFIR_OK;
}

static kefir_result_t define_subprogram_decl_formal_parameter_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subprogram_declaration_formal_parameter == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL,
            KEFIR_OK);

    context->abbrev.entries.subprogram_declaration_formal_parameter =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen,
                                              context->abbrev.entries.subprogram_declaration_formal_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_subprogram_decl_untyped_formal_parameter_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(
        context->abbrev.entries.subprogram_declaration_untyped_formal_parameter == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL,
        KEFIR_OK);

    context->abbrev.entries.subprogram_declaration_untyped_formal_parameter =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen,
                                              context->abbrev.entries.subprogram_declaration_untyped_formal_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_subprogram_decl_anonymous_formal_parameter_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subprogram_declaration_anonymous_formal_parameter ==
                KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL,
            KEFIR_OK);

    context->abbrev.entries.subprogram_declaration_anonymous_formal_parameter =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen,
                                              context->abbrev.entries.subprogram_declaration_anonymous_formal_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_subprogram_decl_unspecified_parameters_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.unspecified_paramters == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.unspecified_paramters = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.unspecified_paramters,
                                              KEFIR_DWARF(DW_TAG_unspecified_parameters), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_subprogram_declaration_abbrev(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                             const struct kefir_ir_module *ir_module,
                                                             struct kefir_codegen_amd64_dwarf_context *context,
                                                             kefir_ir_debug_entry_id_t subprogram_entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id, NULL));

    kefir_ir_debug_entry_id_t child_id;
    struct kefir_ir_debug_entry_child_iterator iter;
    kefir_result_t res;
    for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, subprogram_entry_id, &iter, &child_id);
         res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_id)) {

        const struct kefir_ir_debug_entry *child_entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(&ir_module->debug_info.entries, child_id, &child_entry));

        switch (child_entry->tag) {
            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER: {
                kefir_bool_t has_type = false;
                kefir_result_t res = kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, child_id,
                                                                        KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id, NULL));
                    has_type = true;
                }

                kefir_bool_t has_name;
                REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, child_id,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));

                if (has_type && has_name) {
                    REQUIRE_OK(define_subprogram_decl_formal_parameter_abbrev(codegen, context));
                } else if (has_type && !has_name) {
                    REQUIRE_OK(define_subprogram_decl_anonymous_formal_parameter_abbrev(codegen, context));
                } else if (!has_type && has_name) {
                    REQUIRE_OK(define_subprogram_decl_untyped_formal_parameter_abbrev(codegen, context));
                }
            } break;

            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG:
                REQUIRE_OK(define_subprogram_decl_unspecified_parameters_abbrev(codegen, context));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR debug entry");
        }
    }

    REQUIRE(context->abbrev.entries.subprogram_declaration == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.subprogram_declaration = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.subprogram_declaration,
                                              KEFIR_DWARF(DW_TAG_subprogram), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_prototyped),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_external), KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_declaration),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_linkage_name),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_file),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_line),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_column),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_dwarf_generate_global_identifier_info(
    struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_ir_module *ir_module,
    struct kefir_codegen_amd64_dwarf_context *context, const struct kefir_ir_identifier *identifier) {
    kefir_bool_t function_local_variable;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, identifier->debug_info.entry,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &function_local_variable));
    REQUIRE(!function_local_variable, KEFIR_OK);

    const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    kefir_codegen_amd64_dwarf_entry_id_t variable_entry_id = identifier->debug_info.entry;

    kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id, &type_entry_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, entry_id, context->abbrev.entries.global_variable));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, attr->name));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_EXTERNAL, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, attr->external ? 1 : 0));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_DECLARATION, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, attr->declaration ? 1 : 0));

    kefir_result_t res = kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                            KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_GLOBAL_VARIABLE, &attr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, 1 + KEFIR_AMD64_ABI_QWORD));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_addr)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  identifier->alias != NULL ? identifier->alias : identifier->symbol)));
    } else {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_THREAD_LOCAL_VARIABLE, &attr));
        if (codegen->config->emulated_tls) {
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, 0));
        } else {
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, 2 + KEFIR_AMD64_ABI_QWORD));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_const8u)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_DTPOFF,
                    identifier->alias != NULL ? identifier->alias : identifier->symbol)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_OP_form_tls_address)));
        }
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

    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen,
                                        identifier->alias != NULL ? identifier->alias : identifier->symbol));

    kefir_bool_t has_source_location;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                  &has_source_location));
    if (has_source_location) {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, attr->source_location));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, attr->line));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, attr->column));
    } else {
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ""));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, 0));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, 0));
    }

    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_dwarf_generate_function_info(
    struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_ir_module *ir_module,
    struct kefir_codegen_amd64_dwarf_context *context, const struct kefir_ir_identifier *identifier) {
    const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    kefir_codegen_amd64_dwarf_entry_id_t subprogram_decl_entry_id = identifier->debug_info.entry;

    kefir_codegen_amd64_dwarf_entry_id_t return_type_entry_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id, &return_type_entry_id));

    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, entry_id, context->abbrev.entries.subprogram_declaration));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY,
                                                       return_type_entry_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_FUNCTION_PROTOTYPED_FLAG, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, attr->function_prototyped ? 1 : 0));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_EXTERNAL, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, attr->external ? 1 : 0));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_DECLARATION, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, attr->declaration ? 1 : 0));

    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen,
                                        identifier->alias != NULL ? identifier->alias : identifier->symbol));

    kefir_bool_t has_source_location;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                  &has_source_location));
    if (has_source_location) {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, attr->source_location));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, attr->line));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, subprogram_decl_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, attr->column));
    } else {
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ""));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, 0));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen->xasmgen, 0));
    }

    kefir_ir_debug_entry_id_t child_id;
    struct kefir_ir_debug_entry_child_iterator iter;
    kefir_result_t res;
    for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, subprogram_decl_entry_id, &iter,
                                               &child_id);
         res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_id)) {

        const struct kefir_ir_debug_entry *child_entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(&ir_module->debug_info.entries, child_id, &child_entry));

        switch (child_entry->tag) {
            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER: {
                kefir_codegen_amd64_dwarf_entry_id_t parameter_type_id;
                const char *name = NULL;
                kefir_bool_t has_type = false, has_name = false;
                kefir_result_t res = kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, child_id,
                                                                        KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, attr->type_id,
                                                              &parameter_type_id));
                    has_type = true;
                }
                res = kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, child_id,
                                                         KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    name = attr->name;
                    has_name = true;
                }

                const kefir_codegen_amd64_dwarf_entry_id_t entry_id =
                    KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
                if (has_type && has_name) {
                    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(
                        &codegen->xasmgen, entry_id, context->abbrev.entries.subprogram_declaration_formal_parameter));
                } else if (has_type && !has_name) {
                    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(
                        &codegen->xasmgen, entry_id,
                        context->abbrev.entries.subprogram_declaration_anonymous_formal_parameter));
                } else if (!has_type && has_name) {
                    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(
                        &codegen->xasmgen, entry_id,
                        context->abbrev.entries.subprogram_declaration_untyped_formal_parameter));
                }

                if (has_name) {
                    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, name));
                }

                if (has_type) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                        kefir_asm_amd64_xasmgen_operand_subtract(
                            &codegen->xasmgen_helpers.operands[0],
                            kefir_asm_amd64_xasmgen_operand_label(
                                &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                kefir_asm_amd64_xasmgen_helpers_format(
                                    &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_id)),
                            kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
                }
            } break;

            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG: {
                const kefir_codegen_amd64_dwarf_entry_id_t entry_id =
                    KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, entry_id,
                                                        context->abbrev.entries.unspecified_paramters));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR debug entry");
        }
    }

    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, KEFIR_DWARF(null)));

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

        if (identifier->debug_info.entry == KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
            continue;
        }

        KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
            if (identifier->type == KEFIR_IR_IDENTIFIER_FUNCTION) {
                REQUIRE_OK(generate_subprogram_declaration_abbrev(mem, codegen, ir_module, context,
                                                                  identifier->debug_info.entry));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_global_identifier_abbrev(mem, codegen, ir_module, context,
                                                                                       identifier));
            }
        }

        KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
            if (identifier->type == KEFIR_IR_IDENTIFIER_FUNCTION) {
                REQUIRE_OK(
                    kefir_codegen_amd64_dwarf_generate_function_info(mem, codegen, ir_module, context, identifier));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_global_identifier_info(mem, codegen, ir_module, context,
                                                                                     identifier));
            }
        }
    }

    return KEFIR_OK;
}
