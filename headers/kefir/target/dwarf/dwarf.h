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

#ifndef KEFIR_TARGET_DWARF_DWARF_H_
#define KEFIR_TARGET_DWARF_DWARF_H_

#define KEFIR_DWARF(_id) kefir_dwarf_##_id

#define KEFIR_DWARF_VESION 0x5

enum { KEFIR_DWARF(null) = 0x0 };

typedef enum kefir_dwarf_unit_header_type { KEFIR_DWARF(DW_UT_compile) = 0x01 } kefir_dwarf_unit_header_type_t;

typedef enum kefir_dwarf_children {
    KEFIR_DWARF(DW_CHILDREN_no) = 0,
    KEFIR_DWARF(DW_CHILDREN_yes) = 1
} kefir_dwarf_children_t;

typedef enum kefir_dwarf_tag {
    KEFIR_DWARF(DW_TAG_array_type) = 0x01,
    KEFIR_DWARF(DW_TAG_enumeration_type) = 0x04,
    KEFIR_DWARF(DW_TAG_formal_parameter) = 0x05,
    KEFIR_DWARF(DW_TAG_label) = 0x0a,
    KEFIR_DWARF(DW_TAG_lexical_block) = 0x0b,
    KEFIR_DWARF(DW_TAG_member) = 0x0d,
    KEFIR_DWARF(DW_TAG_pointer_type) = 0x0f,
    KEFIR_DWARF(DW_TAG_compile_unit) = 0x11,
    KEFIR_DWARF(DW_TAG_structure_type) = 0x13,
    KEFIR_DWARF(DW_TAG_subroutine_type) = 0x15,
    KEFIR_DWARF(DW_TAG_typedef) = 0x16,
    KEFIR_DWARF(DW_TAG_union_type) = 0x17,
    KEFIR_DWARF(DW_TAG_unspecified_parameters) = 0x18,
    KEFIR_DWARF(DW_TAG_subrange_type) = 0x21,
    KEFIR_DWARF(DW_TAG_base_type) = 0x24,
    KEFIR_DWARF(DW_TAG_const_type) = 0x26,
    KEFIR_DWARF(DW_TAG_enumerator) = 0x28,
    KEFIR_DWARF(DW_TAG_subprogram) = 0x2e,
    KEFIR_DWARF(DW_TAG_variable) = 0x34,
    KEFIR_DWARF(DW_TAG_volatile_type) = 0x35,
    KEFIR_DWARF(DW_TAG_restrict_type) = 0x37,
    KEFIR_DWARF(DW_TAG_atomic_type) = 0x47
} kefir_dwarf_tag_t;

typedef enum kefir_dwarf_attribute {
    KEFIR_DWARF(DW_AT_end_of_list) = 0x0,
    KEFIR_DWARF(DW_AT_location) = 0x02,
    KEFIR_DWARF(DW_AT_name) = 0x03,
    KEFIR_DWARF(DW_AT_byte_size) = 0x0b,
    KEFIR_DWARF(DW_AT_bit_size) = 0x0d,
    KEFIR_DWARF(DW_AT_stmt_list) = 0x10,
    KEFIR_DWARF(DW_AT_low_pc) = 0x11,
    KEFIR_DWARF(DW_AT_high_pc) = 0x12,
    KEFIR_DWARF(DW_AT_language) = 0x13,
    KEFIR_DWARF(DW_AT_const_value) = 0x1c,
    KEFIR_DWARF(DW_AT_producer) = 0x25,
    KEFIR_DWARF(DW_AT_prototyped) = 0x27,
    KEFIR_DWARF(DW_AT_count) = 0x37,
    KEFIR_DWARF(DW_AT_data_member_location) = 0x38,
    KEFIR_DWARF(DW_AT_decl_column) = 0x39,
    KEFIR_DWARF(DW_AT_decl_file) = 0x3a,
    KEFIR_DWARF(DW_AT_decl_line) = 0x3b,
    KEFIR_DWARF(DW_AT_declaration) = 0x3c,
    KEFIR_DWARF(DW_AT_encoding) = 0x3e,
    KEFIR_DWARF(DW_AT_external) = 0x3f,
    KEFIR_DWARF(DW_AT_frame_base) = 0x40,
    KEFIR_DWARF(DW_AT_type) = 0x49,
    KEFIR_DWARF(DW_AT_data_bit_offset) = 0x6b,
    KEFIR_DWARF(DW_AT_linkage_name) = 0x6e,
    KEFIR_DWARF(DW_AT_alignment) = 0x88
} kefir_dwarf_attribute_t;

typedef enum kefir_dwarf_form {
    KEFIR_DWARF(DW_FORM_end_of_list) = 0x0,
    KEFIR_DWARF(DW_FORM_addr) = 0x01,
    KEFIR_DWARF(DW_FORM_data2) = 0x5,
    KEFIR_DWARF(DW_FORM_data8) = 0x7,
    KEFIR_DWARF(DW_FORM_data1) = 0x0b,
    KEFIR_DWARF(DW_FORM_udata) = 0xf,
    KEFIR_DWARF(DW_FORM_string) = 0x8,
    KEFIR_DWARF(DW_FORM_exprloc) = 0x18,
    KEFIR_DWARF(DW_FORM_flag) = 0x0c,
    KEFIR_DWARF(DW_FORM_ref4) = 0x13,
    KEFIR_DWARF(DW_FORM_sec_offset) = 0x17
} kefir_dwarf_form_t;

typedef enum kefir_dwarf_encoding {
    KEFIR_DWARF(DW_ATE_void) = 0x0,
    KEFIR_DWARF(DW_ATE_boolean) = 0x2,
    KEFIR_DWARF(DW_ATE_complex_float) = 0x3,
    KEFIR_DWARF(DW_ATE_float) = 0x4,
    KEFIR_DWARF(DW_ATE_signed) = 0x5,
    KEFIR_DWARF(DW_ATE_signed_char) = 0x6,
    KEFIR_DWARF(DW_ATE_unsigned) = 0x7,
    KEFIR_DWARF(DW_ATE_unsigned_char) = 0x8
} kefir_dwarf_encoding_t;

typedef enum kefir_dwarf_operation {
    KEFIR_DWARF(DW_OP_addr) = 0x03,
    KEFIR_DWARF(DW_OP_const8u) = 0x0e,
    KEFIR_DWARF(DW_OP_const8s) = 0x0f,
    KEFIR_DWARF(DW_OP_plus) = 0x22,
    KEFIR_DWARF(DW_OP_reg6) = 0x56,
    KEFIR_DWARF(DW_OP_breg6) = 0x76,
    KEFIR_DWARF(DW_OP_regx) = 0x90,
    KEFIR_DWARF(DW_OP_fbreg) = 0x91,
    KEFIR_DWARF(DW_OP_bregx) = 0x92,
    KEFIR_DWARF(DW_OP_piece) = 0x93,
    KEFIR_DWARF(DW_OP_form_tls_address) = 0x9b,
    KEFIR_DWARF(DW_OP_implicit_value) = 0x9e
} kefir_dwarf_operation_t;

typedef enum kefir_dwarf_language { KEFIR_DWARF(DW_LANG_C11) = 0x1d } kefir_dwarf_language_t;

typedef enum kefir_dwarf_loclist {
    KEFIR_DWARF(DW_LLE_end_of_list) = 0x0,
    KEFIR_DWARF(DW_LLE_start_end) = 0x7
} kefir_dwarf_loclist_t;

#define KEFIR_DWARF_AMD64_BREG_RBP KEFIR_DWARF(DW_OP_breg6)

#endif
