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

#define KEFIR_CODEGEN_AMD64_DWARF_INTERNAL
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/optimizer/module_liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t generate_variable_abbrev(struct kefir_mem *mem,
                                               const struct kefir_codegen_amd64_function *codegen_function,
                                               struct kefir_codegen_amd64_dwarf_context *context,
                                               kefir_ir_debug_entry_id_t entry_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));

    REQUIRE(context->abbrev.entries.variable == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.variable = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen_function->codegen->xasmgen, context->abbrev.entries.variable,
                                              KEFIR_DWARF(DW_TAG_variable), KEFIR_DWARF(DW_CHILDREN_no),
                                              codegen_function->codegen->symbol_prefix));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_name),
                                                  KEFIR_DWARF(DW_FORM_strp)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_sec_offset)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_external),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_declaration),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_file),
                                                  KEFIR_DWARF(DW_FORM_strp)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_line),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_column),
                                                  KEFIR_DWARF(DW_FORM_data8)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen_function->codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_varaiable_info(struct kefir_mem *mem,
                                              const struct kefir_codegen_amd64_function *codegen_function,
                                              struct kefir_codegen_amd64_dwarf_context *context,
                                              kefir_ir_debug_entry_id_t variable_entry_id,
                                              kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id_ptr) {
    const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);

    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers;

    kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &type_entry_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, entry_id,
                                            context->abbrev.entries.variable,
                                            codegen_function->codegen->symbol_prefix));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_strp(mem, &codegen_function->codegen->xasmgen, context,
                                                       codegen_function->codegen->symbol_prefix, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY,
                                                       codegen_function->codegen->symbol_prefix, type_entry_id)),
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION,
                                                       codegen_function->codegen->symbol_prefix)))));

    kefir_codegen_amd64_dwarf_entry_id_t loclist_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_LOCLIST_ENTRY_ID(context);
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->loclists.entries.ir_debug_entries,
                                     (kefir_hashtree_key_t) variable_entry_id,
                                     (kefir_hashtree_value_t) loclist_entry_id));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                                   codegen_function->codegen->symbol_prefix, loclist_entry_id))));

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_EXTERNAL, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, attr->external ? 1 : 0));

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_DECLARATION,
                                                  &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, attr->external ? 1 : 0));

    kefir_bool_t has_source_location;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                  &has_source_location));
    if (has_source_location) {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                      &attr));
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_strp(mem, &codegen_function->codegen->xasmgen, context,
                                                           codegen_function->codegen->symbol_prefix,
                                                           attr->source_location));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->line));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->column));
    } else {
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_strp(mem, &codegen_function->codegen->xasmgen, context,
                                                           codegen_function->codegen->symbol_prefix, ""));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
    }
    ASSIGN_PTR(dwarf_entry_id_ptr, entry_id);
    return KEFIR_OK;
}

static kefir_result_t register_to_dwarf_op(kefir_asm_amd64_xasmgen_register_t, kefir_uint8_t *);
static kefir_result_t generate_location_of_virtual_register(struct kefir_codegen_amd64_function *,
                                                            const struct kefir_asmcmp_debug_info_value_location *,
                                                            kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t);

static kefir_result_t generate_local_variable_simple_location_impl(
    struct kefir_codegen_amd64_function *codegen_function, const struct kefir_ir_identifier *ir_identifier,
    kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label,
    kefir_opt_instruction_ref_t allocation_instr_ref) {
    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers2;
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_offset_pair)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       codegen_function->codegen->symbol_prefix, ir_identifier->symbol,
                                                       begin_label)),
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                       codegen_function->codegen->symbol_prefix)))));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       codegen_function->codegen->symbol_prefix, ir_identifier->symbol,
                                                       end_label)),
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                       codegen_function->codegen->symbol_prefix)))));

    kefir_int64_t offset;
    kefir_result_t res = kefir_codegen_amd64_stack_frame_local_variable_offset(
        &codegen_function->stack_frame, (kefir_id_t) allocation_instr_ref, &offset);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);

        offset += codegen_function->stack_frame.offsets.local_area;
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                             1 + kefir_amd64_dwarf_sleb128_length(offset)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
    } else {
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 0));
    }

    return KEFIR_OK;
}

static kefir_result_t generate_local_variable_simple_location(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *codegen_function,
                                                              kefir_ir_debug_entry_id_t variable_entry_id,
                                                              kefir_opt_instruction_ref_t allocation_instr_ref) {
    UNUSED(mem);
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    struct kefir_hashtree fragment_tree;
    REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

    kefir_result_t res = kefir_codegen_amd64_dwarf_generate_range_list_coalesce(mem, codegen_function, code_begin_idx,
                                                                                code_end_idx, &fragment_tree);
    struct kefir_hashtree_node_iterator tree_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&fragment_tree, &tree_iter);
         res == KEFIR_OK && node != NULL; node = kefir_hashtree_next(&tree_iter)) {
        kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
        kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

        REQUIRE_CHAIN(&res, generate_local_variable_simple_location_impl(codegen_function, ir_identifier, begin_label,
                                                                         end_label, allocation_instr_ref));
    }

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &fragment_tree);
        return res;
    });

    REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));

    return KEFIR_OK;
}

#define X87_ST0_DWARF_REGNUM 33
static kefir_result_t register_to_dwarf_op(kefir_asm_amd64_xasmgen_register_t reg, kefir_uint8_t *regnum) {
    switch (reg) {
        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
            *regnum = 0;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
            *regnum = 1;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
            *regnum = 2;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
            *regnum = 3;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
            *regnum = 4;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
            *regnum = 5;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
            *regnum = 6;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
            *regnum = 7;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
            *regnum = 8;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
            *regnum = 9;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
            *regnum = 10;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
            *regnum = 11;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
            *regnum = 12;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
            *regnum = 13;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
            *regnum = 14;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            *regnum = 15;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
            *regnum = 17;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
            *regnum = 18;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
            *regnum = 19;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
            *regnum = 20;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
            *regnum = 21;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
            *regnum = 22;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
            *regnum = 23;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
            *regnum = 24;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
            *regnum = 25;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
            *regnum = 26;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
            *regnum = 27;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
            *regnum = 28;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
            *regnum = 29;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
            *regnum = 30;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
            *regnum = 31;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            *regnum = 32;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to convert register into DWARF register number");
    }
    return KEFIR_OK;
}

static kefir_result_t generate_lle_start_end(struct kefir_codegen_amd64_function *codegen_function,
                                             const struct kefir_ir_identifier *ir_identifier,
                                             kefir_asmcmp_label_index_t location_range_begin_label,
                                             kefir_asmcmp_label_index_t location_range_end_label) {
    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers2;
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_offset_pair)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       codegen_function->codegen->symbol_prefix, ir_identifier->symbol,
                                                       location_range_begin_label)),
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                       codegen_function->codegen->symbol_prefix)))));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       codegen_function->codegen->symbol_prefix, ir_identifier->symbol,
                                                       location_range_end_label)),
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                       codegen_function->codegen->symbol_prefix)))));
    return KEFIR_OK;
}

static kefir_result_t generate_return_space_location(struct kefir_codegen_amd64_function *codegen_function,
                                                     kefir_asmcmp_debug_info_value_location_reference_t location_ref) {
    struct kefir_asmcmp_debug_info_value_location location = {0};
    REQUIRE_OK(kefir_codegen_amd64_function_location_map_get(codegen_function, location_ref, &location));

    switch (location.type) {
        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 0));
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t reg;
            kefir_uint8_t regnum;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(location.reg, &reg));
            REQUIRE_OK(register_to_dwarf_op(reg, &regnum));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 1 + kefir_amd64_dwarf_uleb128_length(regnum) +
                                                     kefir_amd64_dwarf_sleb128_length(location.local_variable.offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_bregx)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, regnum));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, location.local_variable.offset));
        } break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_SPILL_AREA: {
            const kefir_int64_t vreg_offset =
                location.spill_area.index * KEFIR_AMD64_ABI_QWORD + codegen_function->stack_frame.offsets.spill_area;
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 4 + kefir_amd64_dwarf_sleb128_length(vreg_offset) +
                                                     kefir_amd64_dwarf_sleb128_length(location.local_variable.offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, vreg_offset));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_deref)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_consts)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, location.local_variable.offset));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_plus)));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected return space virtual register allocation type");
    }

    return KEFIR_OK;
}

static kefir_result_t generate_return_space_location_with_range(struct kefir_codegen_amd64_function *codegen_function) {
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    kefir_result_t res;
    struct kefir_asmcmp_value_map_fragment_iterator iter;
    const struct kefir_asmcmp_debug_info_value_fragment *fragment;
    for (res = kefir_asmcmp_value_map_fragment_iter(
             &codegen_function->code.context.debug_info.value_map,
             codegen_function->stack_frame.local_variables->return_space_variable_ref, &iter, &fragment);
         res == KEFIR_OK; res = kefir_asmcmp_value_map_fragment_next(&iter, &fragment)) {
        if (fragment->begin_label == fragment->end_label) {
            continue;
        }

        struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers2;
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_offset_pair)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
            kefir_asm_amd64_xasmgen_operand_subtract(
                &codegen_function->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, codegen_function->codegen->symbol_prefix,
                                                           ir_identifier->symbol, fragment->begin_label)),
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                           codegen_function->codegen->symbol_prefix)))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
            kefir_asm_amd64_xasmgen_operand_subtract(
                &codegen_function->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, codegen_function->codegen->symbol_prefix,
                                                           ir_identifier->symbol, fragment->end_label)),
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                           codegen_function->codegen->symbol_prefix)))));

        REQUIRE_OK(generate_return_space_location(codegen_function, fragment->location_ref));
    }
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_OK;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t generate_location_of_virtual_register(
    struct kefir_codegen_amd64_function *codegen_function,
    const struct kefir_asmcmp_debug_info_value_location *location, kefir_asmcmp_label_index_t range_begin_label,
    kefir_asmcmp_label_index_t range_end_label) {
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    switch (location->type) {
        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_IMMEDIATE: {
            REQUIRE_OK(generate_lle_start_end(codegen_function, ir_identifier, range_begin_label, range_end_label));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(
                &codegen_function->codegen->xasmgen,
                1 + kefir_amd64_dwarf_uleb128_length(kefir_amd64_dwarf_sleb128_length(location->immediate)) +
                    kefir_amd64_dwarf_sleb128_length(location->immediate)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_implicit_value)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 kefir_amd64_dwarf_sleb128_length(location->immediate)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, location->immediate));
        } break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_REGISTER: {
            REQUIRE_OK(generate_lle_start_end(codegen_function, ir_identifier, range_begin_label, range_end_label));

            kefir_asm_amd64_xasmgen_register_t reg;
            kefir_uint8_t regnum;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(location->reg, &reg));
            REQUIRE_OK(register_to_dwarf_op(reg, &regnum));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 2));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_regx)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, regnum));
        } break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_SPILL_AREA: {
            REQUIRE_OK(generate_lle_start_end(codegen_function, ir_identifier, range_begin_label, range_end_label));

            const kefir_int64_t offset =
                location->spill_area.index * KEFIR_AMD64_ABI_QWORD + codegen_function->stack_frame.offsets.spill_area;
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 1 + kefir_amd64_dwarf_sleb128_length(offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
        } break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_LOCAL_VARIABLE: {
            if (location->local_variable.identifier ==
                codegen_function->stack_frame.local_variables->return_space_variable_ref) {
                REQUIRE_OK(generate_return_space_location_with_range(codegen_function));
            } else {
                REQUIRE_OK(generate_lle_start_end(codegen_function, ir_identifier, range_begin_label, range_end_label));

                kefir_int64_t offset;
                kefir_result_t res = kefir_codegen_amd64_stack_frame_local_variable_offset(
                    &codegen_function->stack_frame, location->local_variable.identifier, &offset);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    offset += location->local_variable.offset + codegen_function->stack_frame.offsets.local_area;
                    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                         1 + kefir_amd64_dwarf_sleb128_length(offset)));
                    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
                    REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
                }
            }
        } break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_MEMORY_POINTER: {
            REQUIRE_OK(generate_lle_start_end(codegen_function, ir_identifier, range_begin_label, range_end_label));

            kefir_uint8_t regnum;
            REQUIRE_OK(register_to_dwarf_op(location->memory.base_reg, &regnum));

            const kefir_int64_t offset = location->memory.offset;
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(
                &codegen_function->codegen->xasmgen,
                1 + kefir_amd64_dwarf_uleb128_length(regnum) + kefir_amd64_dwarf_sleb128_length(offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_bregx)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, regnum));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t is_vregs_same(const struct kefir_codegen_amd64_function *func,
                                    kefir_asmcmp_debug_info_value_location_reference_t location_ref,
                                    kefir_asmcmp_debug_info_value_location_reference_t other_location_ref,
                                    kefir_bool_t *same) {
    if (location_ref == other_location_ref) {
        *same = true;
        return KEFIR_OK;
    }

    struct kefir_asmcmp_debug_info_value_location location, other_location;
    REQUIRE_OK(kefir_codegen_amd64_function_location_map_get(func, location_ref, &location));
    REQUIRE_OK(kefir_codegen_amd64_function_location_map_get(func, other_location_ref, &other_location));

    if (location.type != other_location.type) {
        *same = false;
        return KEFIR_OK;
    }

    switch (location.type) {
        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE:
            *same = true;
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_IMMEDIATE:
            *same = location.immediate == other_location.immediate;
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_REGISTER:
            *same = location.reg == other_location.reg;
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_SPILL_AREA:
            *same = location.spill_area.index == other_location.spill_area.index &&
                    location.spill_area.length == other_location.spill_area.length;
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_LOCAL_VARIABLE:
            *same = location.local_variable.identifier == other_location.local_variable.identifier &&
                    location.local_variable.offset == other_location.local_variable.offset;
            break;

        case KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_MEMORY_POINTER:
            *same = location.memory.base_reg == other_location.memory.base_reg &&
                    location.memory.offset == other_location.memory.offset;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t coalesce_value_map_fragments(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    const struct kefir_asmcmp_debug_info_value_map *value_map, kefir_opt_instruction_ref_t instr_ref,
    struct kefir_hashtree *fragment_tree,
    kefir_result_t (*do_fragment)(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                  const struct kefir_asmcmp_debug_info_value_location *, kefir_asmcmp_label_index_t,
                                  kefir_asmcmp_label_index_t, void *),
    void *do_fragment_payload) {

    kefir_result_t res;
    struct kefir_asmcmp_value_map_fragment_iterator iter;
    const struct kefir_asmcmp_debug_info_value_fragment *fragment;
    REQUIRE_OK(kefir_hashtree_clean(mem, fragment_tree));
    for (res = kefir_asmcmp_value_map_fragment_iter(value_map, instr_ref, &iter, &fragment); res == KEFIR_OK;
         res = kefir_asmcmp_value_map_fragment_next(&iter, &fragment)) {
        if (fragment->begin_label == fragment->end_label) {
            continue;
        }

        kefir_hashtree_key_t key =
            (((kefir_uint64_t) fragment->begin_label) << 32) | (kefir_uint32_t) fragment->end_label;
        kefir_result_t res =
            kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) fragment->location_ref);
        if (res == KEFIR_ALREADY_EXISTS) {
            continue;
        }
        REQUIRE_OK(res);
    }
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_OK;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_bool_t reached_fixpoint = false;
    for (; !reached_fixpoint;) {
        reached_fixpoint = true;

        struct kefir_hashtree_node_iterator tree_iter;
        struct kefir_hashtree_node *next_node = NULL;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(fragment_tree, &tree_iter); node != NULL;
             node = next_node) {
            next_node = kefir_hashtree_next(&tree_iter);
            kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
            kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;
            ASSIGN_DECL_CAST(kefir_asmcmp_debug_info_value_location_reference_t, location_ref, node->value);

            struct kefir_hashtree_node *other_node;
            kefir_result_t res = kefir_hashtree_lower_bound(
                fragment_tree, (kefir_hashtree_key_t) (((kefir_uint64_t) end_label) << 32), &other_node);
            if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(kefir_hashtree_min(fragment_tree, &other_node));
            } else {
                REQUIRE_OK(res);
                other_node = kefir_hashtree_next_node(fragment_tree, other_node);
            }

            if (other_node == NULL) {
                continue;
            }

            kefir_asmcmp_label_index_t other_begin_label = ((kefir_uint64_t) other_node->key) >> 32;
            kefir_asmcmp_label_index_t other_end_label = (kefir_uint32_t) other_node->key;
            ASSIGN_DECL_CAST(kefir_asmcmp_debug_info_value_location_reference_t, other_location_ref, other_node->value);

            if (other_begin_label != end_label) {
                continue;
            }

            kefir_bool_t vreg_same = false;
            REQUIRE_OK(is_vregs_same(codegen_function, location_ref, other_location_ref, &vreg_same));
            if (vreg_same) {
                reached_fixpoint = false;
                if (next_node != NULL && other_node->key == next_node->key) {
                    next_node = kefir_hashtree_next(&tree_iter);
                }
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, node->key));
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, other_node->key));

                kefir_hashtree_key_t key = (((kefir_uint64_t) begin_label) << 32) | (kefir_uint32_t) other_end_label;
                res = kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) location_ref);
                if (res == KEFIR_ALREADY_EXISTS) {
                    continue;
                }
                REQUIRE_OK(res);
            }
        }
    }

    struct kefir_hashtree_node_iterator tree_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(fragment_tree, &tree_iter); node != NULL;
         node = kefir_hashtree_next(&tree_iter)) {
        kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
        kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;
        ASSIGN_DECL_CAST(kefir_asmcmp_debug_info_value_location_reference_t, location_ref, node->value);

        struct kefir_asmcmp_debug_info_value_location location;
        REQUIRE_OK(kefir_codegen_amd64_function_location_map_get(codegen_function, location_ref, &location));

        REQUIRE_OK(do_fragment(mem, codegen_function, &location, begin_label, end_label, do_fragment_payload));
    }
    return KEFIR_OK;
}

struct generate_x87_stack_locations_of_instruction_do_fragment_param {
    const struct kefir_ir_identifier *ir_identifier;
    kefir_size_t x87_stack_slot;
};

static kefir_result_t generate_x87_stack_locations_of_instruction_do_fragment(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    const struct kefir_asmcmp_debug_info_value_location *location, kefir_asmcmp_label_index_t begin_label,
    kefir_asmcmp_label_index_t end_label, void *payload) {
    UNUSED(mem);
    UNUSED(location);
    struct generate_x87_stack_locations_of_instruction_do_fragment_param *param = payload;

    const kefir_uint8_t regnum = X87_ST0_DWARF_REGNUM + param->x87_stack_slot;

    REQUIRE_OK(generate_lle_start_end(codegen_function, param->ir_identifier, begin_label, end_label));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 2));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_regx)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, regnum));
    return KEFIR_OK;
}

static kefir_result_t generate_x87_stack_locations_of_instruction(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *codegen_function,
                                                                  kefir_opt_instruction_ref_t instr_ref,
                                                                  struct kefir_hashtree *fragment_tree) {
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    kefir_result_t res;
    struct kefir_codegen_amd64_function_x87_locations_iterator x87_iter;
    kefir_opt_instruction_ref_t x87_location_instr_ref;
    kefir_size_t x87_stack_slot;
    for (res = kefir_codegen_amd64_function_x87_locations_iter(codegen_function, instr_ref, &x87_iter,
                                                               &x87_location_instr_ref, &x87_stack_slot);
         res != KEFIR_ITERATOR_END;
         res = kefir_codegen_amd64_function_x87_locations_next(&x87_iter, &x87_location_instr_ref, &x87_stack_slot)) {

        REQUIRE_OK(coalesce_value_map_fragments(
            mem, codegen_function, &codegen_function->code.context.debug_info.value_map, x87_location_instr_ref,
            fragment_tree, generate_x87_stack_locations_of_instruction_do_fragment,
            &(struct generate_x87_stack_locations_of_instruction_do_fragment_param) {.x87_stack_slot = x87_stack_slot,
                                                                                     .ir_identifier = ir_identifier}));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t generate_instruction_location_do_fragment(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    const struct kefir_asmcmp_debug_info_value_location *location, kefir_asmcmp_label_index_t begin_label,
    kefir_asmcmp_label_index_t end_label, void *payload) {
    UNUSED(mem);
    UNUSED(payload);
    REQUIRE_OK(generate_location_of_virtual_register(codegen_function, location, begin_label, end_label));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_instruction_location(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));

    struct kefir_hashtree fragment_tree;
    REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

    kefir_result_t res = generate_x87_stack_locations_of_instruction(mem, codegen_function, instr_ref, &fragment_tree);
    REQUIRE_CHAIN(
        &res, coalesce_value_map_fragments(mem, codegen_function, &codegen_function->code.context.debug_info.value_map,
                                           instr_ref, &fragment_tree, generate_instruction_location_do_fragment, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &fragment_tree);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));
    return KEFIR_OK;
}

static kefir_result_t generate_local_variable_loclists(struct kefir_mem *mem,
                                                       struct kefir_codegen_amd64_function *codegen_function,
                                                       struct kefir_codegen_amd64_dwarf_context *context,
                                                       kefir_ir_debug_entry_id_t variable_entry_id) {
    UNUSED(mem);
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) variable_entry_id,
                                 &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, loclist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                         codegen_function->codegen->symbol_prefix, loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LOCAL_VARIABLE,
                                                  &attr));
    const struct kefir_opt_code_debug_info_local_variable *local_variable_debug_info;
    kefir_result_t res = kefir_opt_code_debug_info_local_variable(
        &codegen_function->function->debug_info, attr->local_variable.variable_id, &local_variable_debug_info);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);

        struct kefir_hashset_iterator ref_iter;
        kefir_hashset_key_t ref_key;
        for (res = kefir_hashset_iter(&local_variable_debug_info->allocations, &ref_iter, &ref_key); res == KEFIR_OK;
             res = kefir_hashset_next(&ref_iter, &ref_key)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, allocation_instr_ref, ref_key);

            const struct kefir_opt_code_debug_info_allocation_placement *allocation_placement;
            res = kefir_opt_code_debug_info_allocation_placement(&codegen_function->function->debug_info,
                                                                 allocation_instr_ref, &allocation_placement);
            if (allocation_instr_ref == codegen_function->stack_frame.local_variables->return_space_variable_ref) {
                REQUIRE_OK(generate_return_space_location_with_range(codegen_function));
            } else if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(generate_local_variable_simple_location(mem, codegen_function, variable_entry_id,
                                                                   allocation_instr_ref));
            } else {
                REQUIRE_OK(res);
                struct kefir_hashset_iterator ref_iter2;
                kefir_hashset_key_t ref_key2;
                for (res = kefir_hashset_iter(&allocation_placement->placement, &ref_iter2, &ref_key2); res == KEFIR_OK;
                     res = kefir_hashset_next(&ref_iter2, &ref_key2)) {
                    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, ref_key2);
                    REQUIRE_OK(
                        kefir_codegen_amd64_dwarf_generate_instruction_location(mem, codegen_function, instr_ref));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));
    return KEFIR_OK;
}

static kefir_result_t generate_global_variable_loclists(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function *codegen_function,
                                                        struct kefir_codegen_amd64_dwarf_context *context,
                                                        const struct kefir_opt_module_liveness *liveness,
                                                        kefir_ir_debug_entry_id_t variable_entry_id) {
    UNUSED(mem);
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) variable_entry_id,
                                 &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, loclist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                         codegen_function->codegen->symbol_prefix, loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_GLOBAL_VARIABLE,
                                                  &attr));
    const char *symbol = kefir_ir_module_get_named_symbol(codegen_function->module->ir_module, attr->global_variable);
    const struct kefir_ir_identifier *variable_identifier;
    kefir_result_t res =
        kefir_ir_module_get_identifier(codegen_function->module->ir_module, symbol, &variable_identifier);

    if (res != KEFIR_NOT_FOUND && kefir_opt_module_is_symbol_alive(liveness, symbol)) {
        REQUIRE_OK(res);

        const struct kefir_ir_identifier *function_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                                  codegen_function->function->ir_func->name, &function_identifier));

        struct kefir_hashtree fragment_tree;
        REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

        kefir_result_t res = kefir_codegen_amd64_dwarf_generate_range_list_coalesce(
            mem, codegen_function, code_begin_idx, code_end_idx, &fragment_tree);
        struct kefir_hashtree_node_iterator tree_iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&fragment_tree, &tree_iter);
             res == KEFIR_OK && node != NULL; node = kefir_hashtree_next(&tree_iter)) {
            kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
            kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

            struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers2;
            REQUIRE_CHAIN(&res,
                          KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_offset_pair)));
            REQUIRE_CHAIN(
                &res,
                KEFIR_AMD64_XASMGEN_DATA(
                    &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen_function->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[1],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                codegen_function->codegen->symbol_prefix, function_identifier->symbol, begin_label)),
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[2],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                                   codegen_function->codegen->symbol_prefix)))));
            REQUIRE_CHAIN(
                &res,
                KEFIR_AMD64_XASMGEN_DATA(
                    &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen_function->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[1],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                codegen_function->codegen->symbol_prefix, function_identifier->symbol, end_label)),
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[2],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                                   codegen_function->codegen->symbol_prefix)))));

            REQUIRE_CHAIN(&res,
                          KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 1 + KEFIR_AMD64_ABI_QWORD));
            REQUIRE_CHAIN(&res, KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_addr)));
            REQUIRE_CHAIN(
                &res, KEFIR_AMD64_XASMGEN_DATA(&codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                               kefir_asm_amd64_xasmgen_operand_label(
                                                   &codegen_function->codegen->xasmgen_helpers.operands[0],
                                                   KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                   variable_identifier->alias != NULL ? variable_identifier->alias
                                                                                      : variable_identifier->symbol)));
        }

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_free(mem, &fragment_tree);
            return res;
        });

        REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));
    }

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));
    return KEFIR_OK;
}

static kefir_result_t generate_thread_local_variable_loclists(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *codegen_function,
                                                              struct kefir_codegen_amd64_dwarf_context *context,
                                                              kefir_ir_debug_entry_id_t variable_entry_id) {
    UNUSED(mem);
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) variable_entry_id,
                                 &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, loclist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                         codegen_function->codegen->symbol_prefix, loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_THREAD_LOCAL_VARIABLE, &attr));
    const struct kefir_ir_identifier *variable_identifier;
    kefir_result_t res = kefir_ir_module_get_identifier(
        codegen_function->module->ir_module,
        kefir_ir_module_get_named_symbol(codegen_function->module->ir_module, attr->global_variable),
        &variable_identifier);

    if (!codegen_function->codegen->config->emulated_tls && res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);

        const struct kefir_ir_identifier *function_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                                  codegen_function->function->ir_func->name, &function_identifier));

        struct kefir_hashtree fragment_tree;
        REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

        kefir_result_t res = kefir_codegen_amd64_dwarf_generate_range_list_coalesce(
            mem, codegen_function, code_begin_idx, code_end_idx, &fragment_tree);
        struct kefir_hashtree_node_iterator tree_iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&fragment_tree, &tree_iter);
             res == KEFIR_OK && node != NULL; node = kefir_hashtree_next(&tree_iter)) {
            kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
            kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

            struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers2;
            REQUIRE_CHAIN(&res,
                          KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_offset_pair)));
            REQUIRE_CHAIN(
                &res,
                KEFIR_AMD64_XASMGEN_DATA(
                    &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen_function->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[1],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                codegen_function->codegen->symbol_prefix, function_identifier->symbol, begin_label)),
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[2],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                                   codegen_function->codegen->symbol_prefix)))));
            REQUIRE_CHAIN(
                &res,
                KEFIR_AMD64_XASMGEN_DATA(
                    &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ULEB128, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen_function->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[1],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                codegen_function->codegen->symbol_prefix, function_identifier->symbol, end_label)),
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[2],
                            KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers2, KEFIR_AMD64_TEXT_SECTION_BEGIN,
                                                                   codegen_function->codegen->symbol_prefix)))));

            REQUIRE_CHAIN(&res,
                          KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 2 + KEFIR_AMD64_ABI_QWORD));
            REQUIRE_CHAIN(&res,
                          KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_const8u)));
            REQUIRE_CHAIN(
                &res, KEFIR_AMD64_XASMGEN_DATA(&codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                               kefir_asm_amd64_xasmgen_operand_label(
                                                   &codegen_function->codegen->xasmgen_helpers.operands[0],
                                                   KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_DTPOFF,
                                                   variable_identifier->alias != NULL ? variable_identifier->alias
                                                                                      : variable_identifier->symbol)));
            REQUIRE_CHAIN(
                &res, KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_form_tls_address)));
        }

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_free(mem, &fragment_tree);
            return res;
        });

        REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));
    }

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_variable(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *codegen_function,
                                                           struct kefir_codegen_amd64_dwarf_context *context,
                                                           const struct kefir_opt_module_liveness *liveness,
                                                           kefir_ir_debug_entry_id_t entry_id,
                                                           kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module liveness"));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_variable_abbrev(mem, codegen_function, context, entry_id));
        ASSIGN_PTR(dwarf_entry_ptr, context->abbrev.entries.variable);
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        REQUIRE_OK(generate_varaiable_info(mem, codegen_function, context, entry_id, dwarf_entry_ptr));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS) {
        kefir_bool_t is_local, is_global;
        REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LOCAL_VARIABLE,
                                                      &is_local));
        REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_GLOBAL_VARIABLE,
                                                      &is_global));
        if (is_local) {
            REQUIRE_OK(generate_local_variable_loclists(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_ptr, KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL);
        } else if (is_global) {
            REQUIRE_OK(generate_global_variable_loclists(mem, codegen_function, context, liveness, entry_id));
            ASSIGN_PTR(dwarf_entry_ptr, KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL);
        } else {
            REQUIRE_OK(generate_thread_local_variable_loclists(mem, codegen_function, context, entry_id));
        }
    }
    return KEFIR_OK;
}
