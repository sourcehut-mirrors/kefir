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
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
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
                                              KEFIR_DWARF(DW_TAG_variable), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_name),
                                                  KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_type),
                                                  KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_location),
                                                  KEFIR_DWARF(DW_FORM_sec_offset)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_external),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_AT_decl_file),
                                                  KEFIR_DWARF(DW_FORM_string)));
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

    kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &attr));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, NULL));
    REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen_function->codegen, codegen_function->module->ir_module,
                                              context, attr->type_id, &type_entry_id));

    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, entry_id, context->abbrev.entries.variable));
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->name));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_subtract(
            &codegen_function->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                       KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, type_entry_id)),
            kefir_asm_amd64_xasmgen_operand_label(&codegen_function->codegen->xasmgen_helpers.operands[2],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                  KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

    kefir_codegen_amd64_dwarf_entry_id_t loclist_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_LOCLIST_ENTRY_ID(context);
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->loclists.entries.ir_debug_entries,
                                     (kefir_hashtree_key_t) variable_entry_id,
                                     (kefir_hashtree_value_t) loclist_entry_id));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY, loclist_entry_id))));

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_EXTERNAL, &attr));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, attr->external ? 1 : 0));

    kefir_bool_t has_source_location;
    REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                  &has_source_location));
    if (has_source_location) {
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION,
                                                      &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, attr->source_location));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_LINE, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->line));
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                      variable_entry_id,
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SOURCE_LOCATION_COLUMN, &attr));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, attr->column));
    } else {
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen_function->codegen->xasmgen, ""));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
        REQUIRE_OK(KEFIR_AMD64_DWARF_QWORD(&codegen_function->codegen->xasmgen, 0));
    }
    ASSIGN_PTR(dwarf_entry_id_ptr, entry_id);
    return KEFIR_OK;
}

static kefir_result_t generate_local_variable_simple_location(
    const struct kefir_codegen_amd64_function *codegen_function, kefir_ir_debug_entry_id_t variable_entry_id,
    kefir_size_t local_variable_id) {
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    kefir_asmcmp_label_index_t range_begin_label, range_end_label;
    REQUIRE_OK(kefir_codegen_amd64_function_find_code_range_labels(codegen_function, code_begin_idx, code_end_idx,
                                                                   &range_begin_label, &range_end_label));

    if (range_begin_label != KEFIR_ASMCMP_INDEX_NONE && range_end_label != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                                  codegen_function->function->ir_func->name, &ir_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       ir_identifier->symbol, range_begin_label))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       ir_identifier->symbol, range_end_label))));

        const struct kefir_abi_amd64_typeentry_layout *entry = NULL;
        REQUIRE_OK(kefir_abi_amd64_type_layout_at(&codegen_function->locals_layout, local_variable_id, &entry));

        const kefir_int64_t offset = entry->relative_offset + codegen_function->stack_frame.offsets.local_area;
        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                             1 + kefir_amd64_dwarf_sleb128_length(offset)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
    }

    return KEFIR_OK;
}

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

kefir_result_t kefir_codegen_amd64_dwarf_generate_instruction_location(
    struct kefir_codegen_amd64_function *codegen_function, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));

    kefir_asmcmp_label_index_t range_begin_label, range_end_label;
    REQUIRE_OK(kefir_codegen_amd64_function_find_instruction_lifetime(codegen_function, instr_ref, &range_begin_label,
                                                                      &range_end_label));
    REQUIRE(range_begin_label != KEFIR_ASMCMP_INDEX_NONE && range_end_label != KEFIR_ASMCMP_INDEX_NONE, KEFIR_OK);

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    kefir_asmcmp_virtual_register_index_t vreg;
    kefir_result_t res = kefir_codegen_amd64_function_vreg_of(codegen_function, instr_ref, &vreg);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_codegen_amd64_register_allocation *reg_allocation;
    REQUIRE_OK(
        kefir_codegen_amd64_register_allocation_of(&codegen_function->register_allocator, vreg, &reg_allocation));

    switch (reg_allocation->type) {
        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
            break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t reg;
            kefir_uint8_t regnum;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg_allocation->direct_reg, &reg));
            REQUIRE_OK(register_to_dwarf_op(reg, &regnum));

            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_begin_label))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_end_label))));
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 2));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_regx)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, regnum));
        } break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT: {
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_begin_label))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_end_label))));

            const kefir_int64_t offset = reg_allocation->spill_area.index * KEFIR_AMD64_ABI_QWORD +
                                         codegen_function->stack_frame.offsets.spill_area;
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 1 + kefir_amd64_dwarf_sleb128_length(offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
        } break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT: {
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_begin_label))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_end_label))));

            const kefir_int64_t offset = reg_allocation->spill_area.index * KEFIR_AMD64_ABI_QWORD +
                                         codegen_function->stack_frame.offsets.spill_area;
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen,
                                                 1 + kefir_amd64_dwarf_sleb128_length(offset)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_fbreg)));
            REQUIRE_OK(KEFIR_AMD64_DWARF_SLEB128(&codegen_function->codegen->xasmgen, offset));
        } break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER: {
            const struct kefir_asmcmp_virtual_register *virtual_reg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&codegen_function->code.context, vreg, &virtual_reg));
            REQUIRE(virtual_reg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register type"));

            kefir_uint8_t regnum;
            REQUIRE_OK(register_to_dwarf_op(virtual_reg->parameters.memory.base_reg, &regnum));

            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_begin_label))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                           KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                           range_end_label))));

            const kefir_int64_t offset = virtual_reg->parameters.memory.offset;
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
                                         loclist_entry_id));

    kefir_bool_t has_refs;
    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LOCAL_VARIABLE,
                                                  &attr));
    REQUIRE_OK(kefir_opt_code_debug_info_local_variable_has_refs(&codegen_function->function->debug_info,
                                                                 attr->local_variable, &has_refs));
    if (!has_refs) {
        REQUIRE_OK(generate_local_variable_simple_location(codegen_function, variable_entry_id, attr->local_variable));
    } else {
        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        struct kefir_opt_code_debug_info_local_variable_ref_iterator iter;
        for (res = kefir_opt_code_debug_info_local_variable_ref_iter(&codegen_function->function->debug_info, &iter,
                                                                     attr->local_variable, &instr_ref);
             res == KEFIR_OK; res = kefir_opt_code_debug_info_local_variable_ref_next(&iter, &instr_ref)) {
            REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_instruction_location(codegen_function, instr_ref));
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
                                                        kefir_ir_debug_entry_id_t variable_entry_id) {
    UNUSED(mem);
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) variable_entry_id,
                                 &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, loclist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_LOCLIST_ENTRY,
                                         loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    kefir_asmcmp_label_index_t range_begin_label, range_end_label;
    REQUIRE_OK(kefir_codegen_amd64_function_find_code_range_labels(codegen_function, code_begin_idx, code_end_idx,
                                                                   &range_begin_label, &range_end_label));

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_GLOBAL_VARIABLE,
                                                  &attr));
    const struct kefir_ir_identifier *variable_identifier;
    kefir_result_t res = kefir_ir_module_get_identifier(
        codegen_function->module->ir_module,
        kefir_ir_module_get_named_symbol(codegen_function->module->ir_module, attr->global_variable),
        &variable_identifier);

    if (res != KEFIR_NOT_FOUND && range_begin_label != KEFIR_ASMCMP_INDEX_NONE &&
        range_end_label != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(res);

        const struct kefir_ir_identifier *function_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                                  codegen_function->function->ir_func->name, &function_identifier));

        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       function_identifier->symbol, range_begin_label))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       function_identifier->symbol, range_end_label))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 1 + KEFIR_AMD64_ABI_QWORD));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_addr)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                variable_identifier->alias != NULL ? variable_identifier->alias : variable_identifier->symbol)));
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
                                         loclist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t code_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id, KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t code_end_idx = attr->code_index;

    kefir_asmcmp_label_index_t range_begin_label, range_end_label;
    REQUIRE_OK(kefir_codegen_amd64_function_find_code_range_labels(codegen_function, code_begin_idx, code_end_idx,
                                                                   &range_begin_label, &range_end_label));

    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries,
                                                  variable_entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_THREAD_LOCAL_VARIABLE, &attr));
    const struct kefir_ir_identifier *variable_identifier;
    kefir_result_t res = kefir_ir_module_get_identifier(
        codegen_function->module->ir_module,
        kefir_ir_module_get_named_symbol(codegen_function->module->ir_module, attr->global_variable),
        &variable_identifier);

    if (!codegen_function->codegen->config->emulated_tls && res != KEFIR_NOT_FOUND &&
        range_begin_label != KEFIR_ASMCMP_INDEX_NONE && range_end_label != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(res);

        const struct kefir_ir_identifier *function_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                                  codegen_function->function->ir_func->name, &function_identifier));

        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_start_end)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       function_identifier->symbol, range_begin_label))));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL,
                                                       function_identifier->symbol, range_end_label))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, 2 + KEFIR_AMD64_ABI_QWORD));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_const8u)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(
                &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_DTPOFF,
                variable_identifier->alias != NULL ? variable_identifier->alias : variable_identifier->symbol)));
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_OP_form_tls_address)));
    }

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_variable(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *codegen_function,
                                                           struct kefir_codegen_amd64_dwarf_context *context,
                                                           kefir_ir_debug_entry_id_t entry_id,
                                                           kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

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
            REQUIRE_OK(generate_global_variable_loclists(mem, codegen_function, context, entry_id));
            ASSIGN_PTR(dwarf_entry_ptr, KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL);
        } else {
            REQUIRE_OK(generate_thread_local_variable_loclists(mem, codegen_function, context, entry_id));
        }
    }
    return KEFIR_OK;
}