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

static kefir_result_t generate_parameter_abbrev(struct kefir_mem *mem,
                                                struct kefir_codegen_amd64_function *codegen_function,
                                                struct kefir_codegen_amd64_dwarf_context *context,
                                                kefir_ir_debug_entry_id_t entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.subprogram_parameter == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.subprogram_parameter = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_function->codegen->xasmgen,
                                              context->abbrev.entries.subprogram_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_name),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_function->codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_anonymous_parameter_abbrev(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *codegen_function,
                                                          struct kefir_codegen_amd64_dwarf_context *context,
                                                          kefir_ir_debug_entry_id_t entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.anonymous_subprogram_parameter == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.anonymous_subprogram_parameter = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_function->codegen->xasmgen,
                                              context->abbrev.entries.anonymous_subprogram_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_function->codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_parameter_with_location_abbrev(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *codegen_function,
                                                              struct kefir_codegen_amd64_dwarf_context *context,
                                                              kefir_ir_debug_entry_id_t entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.subprogram_parameter_location == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.subprogram_parameter_location = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_function->codegen->xasmgen,
                                              context->abbrev.entries.subprogram_parameter_location,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_name),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_sec_offset)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_function->codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_anonymous_parameter_with_location_abbrev(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    struct kefir_codegen_amd64_dwarf_context *context, kefir_ir_debug_entry_id_t entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.anonymous_subprogram_parameter_location == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL,
            KEFIR_OK);
    context->abbrev.entries.anonymous_subprogram_parameter_location =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_function->codegen->xasmgen,
                                              context->abbrev.entries.anonymous_subprogram_parameter_location,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_sec_offset)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_function->codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_parameter_with_location_info(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *codegen_function,
                                                            struct kefir_codegen_amd64_dwarf_context *context,
                                                            kefir_ir_debug_entry_id_t entry_id,
                                                            kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    const kefir_codegen_amd64_dwarf_entry_id_t parameter_entry_id =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, parameter_entry_id);

    kefir_codegen_amd64_dwarf_entry_id_t parameter_type_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &parameter_type_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, parameter_entry_id,
                                            context->abbrev.entries.subprogram_parameter_location));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen_function->codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

    kefir_codegen_amd64_dwarf_entry_id_t loclist_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_LOCLIST_ENTRY_ID(context);
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) entry_id,
                                     (kefir_hashtree_value_t) loclist_entry_id));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY, loclist_entry_id))));
    return KEFIR_OK;
}

static kefir_result_t generate_anonymous_parameter_with_location_info(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    struct kefir_codegen_amd64_dwarf_context *context, kefir_ir_debug_entry_id_t entry_id,
    kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    const kefir_codegen_amd64_dwarf_entry_id_t parameter_entry_id =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, parameter_entry_id);

    kefir_codegen_amd64_dwarf_entry_id_t parameter_type_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &parameter_type_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, parameter_entry_id,
                                            context->abbrev.entries.anonymous_subprogram_parameter_location));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen_function->codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

    kefir_codegen_amd64_dwarf_entry_id_t loclist_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_LOCLIST_ENTRY_ID(context);
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) entry_id,
                                     (kefir_hashtree_value_t) loclist_entry_id));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY, loclist_entry_id))));
    return KEFIR_OK;
}

static kefir_result_t generate_parameter_info(struct kefir_mem *mem,
                                              struct kefir_codegen_amd64_function *codegen_function,
                                              struct kefir_codegen_amd64_dwarf_context *context,
                                              kefir_ir_debug_entry_id_t entry_id,
                                              kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    const kefir_codegen_amd64_dwarf_entry_id_t parameter_entry_id =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, parameter_entry_id);

    kefir_codegen_amd64_dwarf_entry_id_t parameter_type_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &parameter_type_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, parameter_entry_id,
                                            context->abbrev.entries.subprogram_parameter));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen_function->codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
    return KEFIR_OK;
}

static kefir_result_t generate_anonymous_parameter_info(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function *codegen_function,
                                                        struct kefir_codegen_amd64_dwarf_context *context,
                                                        kefir_ir_debug_entry_id_t entry_id,
                                                        kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    const kefir_codegen_amd64_dwarf_entry_id_t parameter_entry_id =
        KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, parameter_entry_id);

    kefir_codegen_amd64_dwarf_entry_id_t parameter_type_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &parameter_type_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, parameter_entry_id,
                                            context->abbrev.entries.anonymous_subprogram_parameter));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen_function->codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
    return KEFIR_OK;
}

static kefir_result_t generate_function_parameter_loclists(struct kefir_codegen_amd64_function *codegen_function,
                                                           struct kefir_codegen_amd64_dwarf_context *context,
                                                           kefir_ir_debug_entry_id_t entry_id) {
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) entry_id, &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, loclist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                         loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_PARAMETER, &attr));
    REQUIRE_OK(
        kefir_hashtree_at(&codegen_function->debug.function_parameters, (kefir_hashtree_key_t) attr->parameter, &node));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_instruction_location(codegen_function,
                                                                       (kefir_opt_instruction_ref_t) node->value));

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_function_parameter(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    struct kefir_codegen_amd64_dwarf_context *context, kefir_ir_debug_entry_id_t entry_id,
    kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_PARAMETER, &attr));

    kefir_bool_t has_type, has_name,
        has_location =
            kefir_hashtree_has(&codegen_function->debug.function_parameters, (kefir_hashtree_key_t) attr->parameter);
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &has_type));
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        if (has_type && has_name && has_location) {
            REQUIRE_OK(generate_parameter_with_location_abbrev(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.subprogram_parameter);
        } else if (has_type && !has_name && has_location) {
            REQUIRE_OK(generate_anonymous_parameter_with_location_abbrev(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.anonymous_subprogram_parameter);
        } else if (has_type && has_name && !has_location) {
            REQUIRE_OK(generate_parameter_abbrev(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.subprogram_parameter);
        } else if (has_type && !has_name && !has_location) {
            REQUIRE_OK(generate_anonymous_parameter_abbrev(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.anonymous_subprogram_parameter);
        }
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        if (has_type && has_name && has_location) {
            REQUIRE_OK(generate_parameter_with_location_info(mem, codegen_function, context, entry_id, dwarf_entry_id));
        } else if (has_type && !has_name && has_location) {
            REQUIRE_OK(generate_anonymous_parameter_with_location_info(mem, codegen_function, context, entry_id,
                                                                       dwarf_entry_id));
        } else if (has_type && has_name && !has_location) {
            REQUIRE_OK(generate_parameter_info(mem, codegen_function, context, entry_id, dwarf_entry_id));
        } else if (has_type && !has_name && !has_location) {
            REQUIRE_OK(generate_anonymous_parameter_info(mem, codegen_function, context, entry_id, dwarf_entry_id));
        }
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS) {
        if (has_location) {
            REQUIRE_OK(generate_function_parameter_loclists(codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_id, KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL);
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_unspecified_function_parameter(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    struct kefir_codegen_amd64_dwarf_context *context, kefir_ir_debug_entry_id_t entry_id,
    kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    UNUSED(entry_id);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(kefir_codegen_amd64_dwarf_define_unspecified_parameters_abbrev(codegen_function->codegen, context));
        ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.unspecified_paramters);
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        kefir_codegen_amd64_dwarf_entry_id_t parameters_entry_id =
            KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
        REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, parameters_entry_id,
                                                context->abbrev.entries.unspecified_paramters));
        ASSIGN_PTR(dwarf_entry_id, parameters_entry_id);
    }

    return KEFIR_OK;
}
