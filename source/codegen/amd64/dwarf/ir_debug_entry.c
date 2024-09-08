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

static kefir_result_t generate_type_immediate(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                              const struct kefir_ir_module *,
                                              struct kefir_codegen_amd64_dwarf_context *, kefir_ir_debug_entry_id_t,
                                              kefir_codegen_amd64_dwarf_entry_id_t *);

static kefir_result_t define_scalar_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.scalar_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.scalar_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.scalar_type,
                                              KEFIR_DWARF(DW_TAG_base_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_encoding), KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_void_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                              struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.void_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.void_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.void_type,
                                              KEFIR_DWARF(DW_TAG_base_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    ;
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_encoding), KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_pointer_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                 struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.pointer_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.pointer_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.pointer_type,
                                              KEFIR_DWARF(DW_TAG_pointer_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_enumeration_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                     struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.enumeration_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.enumeration_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.enumeration_type,
                                              KEFIR_DWARF(DW_TAG_enumeration_type), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_data1)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_enumerator_abbrev(struct kefir_codegen_amd64 *codegen,
                                               struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.enumerator == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.enumerator = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.enumerator,
                                              KEFIR_DWARF(DW_TAG_enumerator), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_const_value),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_array_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                               struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.array_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.array_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.array_type,
                                              KEFIR_DWARF(DW_TAG_array_type), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_subrange_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subrange_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.subrange_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.subrange_type,
                                              KEFIR_DWARF(DW_TAG_subrange_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_count), KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_subrange_nocount_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                          struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subrange_nocount_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.subrange_nocount_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.subrange_nocount_type,
                                              KEFIR_DWARF(DW_TAG_subrange_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_structure_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                   struct kefir_codegen_amd64_dwarf_context *context,
                                                   kefir_codegen_amd64_dwarf_entry_id_t *dwarf_id_ptr,
                                                   kefir_dwarf_tag_t type_tag) {
    REQUIRE(*dwarf_id_ptr == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    *dwarf_id_ptr = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, *dwarf_id_ptr, type_tag, KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_anonymous_structure_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                             struct kefir_codegen_amd64_dwarf_context *context,
                                                             kefir_codegen_amd64_dwarf_entry_id_t *dwarf_id_ptr,
                                                             kefir_dwarf_tag_t type_tag) {
    REQUIRE(*dwarf_id_ptr == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    *dwarf_id_ptr = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, *dwarf_id_ptr, type_tag, KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_incomplete_structure_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                              struct kefir_codegen_amd64_dwarf_context *context,
                                                              kefir_codegen_amd64_dwarf_entry_id_t *dwarf_id_ptr,
                                                              kefir_dwarf_tag_t type_tag) {
    REQUIRE(*dwarf_id_ptr == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    *dwarf_id_ptr = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, *dwarf_id_ptr, type_tag, KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_incomplete_anonymous_structure_type_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context,
    kefir_codegen_amd64_dwarf_entry_id_t *dwarf_id_ptr, kefir_dwarf_tag_t type_tag) {
    REQUIRE(*dwarf_id_ptr == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    *dwarf_id_ptr = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, *dwarf_id_ptr, type_tag, KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_member_abbrev(struct kefir_codegen_amd64 *codegen,
                                           struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.member == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.member = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.member,
                                              KEFIR_DWARF(DW_TAG_member), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_data_member_location),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_anonymous_member_abbrev(struct kefir_codegen_amd64 *codegen,
                                                     struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.anonymous_member == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.anonymous_member = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.anonymous_member,
                                              KEFIR_DWARF(DW_TAG_member), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_byte_size),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_alignment),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_data_member_location),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_bitfield_member_abbrev(struct kefir_codegen_amd64 *codegen,
                                                    struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.bitfield_member == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.bitfield_member = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.bitfield_member,
                                              KEFIR_DWARF(DW_TAG_member), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_data_bit_offset),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_bit_size), KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_anonymous_bitfield_member_abbrev(struct kefir_codegen_amd64 *codegen,
                                                              struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.anonymous_bitfield_member == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.anonymous_bitfield_member = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.anonymous_bitfield_member,
                                              KEFIR_DWARF(DW_TAG_member), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_data_bit_offset),
                                                  KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_bit_size), KEFIR_DWARF(DW_FORM_udata)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_function_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.subroutine_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.subroutine_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.subroutine_type,
                                              KEFIR_DWARF(DW_TAG_subroutine_type), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_prototyped),
                                                  KEFIR_DWARF(DW_FORM_flag)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_formal_parameter_abbrev(struct kefir_codegen_amd64 *codegen,
                                                     struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.formal_parameter == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.formal_parameter = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.formal_parameter,
                                              KEFIR_DWARF(DW_TAG_formal_parameter), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_const_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                               struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.const_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.const_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.const_type,
                                              KEFIR_DWARF(DW_TAG_const_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_volatile_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.volatile_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.volatile_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.volatile_type,
                                              KEFIR_DWARF(DW_TAG_volatile_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_restrict_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.restrict_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.restrict_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.restrict_type,
                                              KEFIR_DWARF(DW_TAG_restrict_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_atomic_type_abbrev(struct kefir_codegen_amd64 *codegen,
                                                struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.atomic_type == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.atomic_type = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.atomic_type,
                                              KEFIR_DWARF(DW_TAG_atomic_type), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_define_unspecified_parameters_abbrev(
    struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.unspecified_paramters == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.unspecified_paramters = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.unspecified_paramters,
                                              KEFIR_DWARF(DW_TAG_unspecified_parameters), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t define_typedef_abbrev(struct kefir_codegen_amd64 *codegen,
                                            struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.type_def == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);

    context->abbrev.entries.type_def = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.type_def,
                                              KEFIR_DWARF(DW_TAG_typedef), KEFIR_DWARF(DW_CHILDREN_no)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_name), KEFIR_DWARF(DW_FORM_string)));
    REQUIRE_OK(
        KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_type), KEFIR_DWARF(DW_FORM_ref4)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_type_immediate_abbrev(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                     const struct kefir_ir_module *ir_module,
                                                     struct kefir_codegen_amd64_dwarf_context *context,
                                                     const struct kefir_ir_debug_entry *ir_debug_entry,
                                                     kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->abbrev.entries.ir_debug_entries,
                                           (kefir_hashtree_key_t) ir_debug_entry->identifier, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(dwarf_entry_id, (kefir_codegen_amd64_dwarf_entry_id_t) node->value);
        return KEFIR_OK;
    }

    const struct kefir_ir_debug_entry_attribute *ir_attr;
    switch (ir_debug_entry->tag) {
        case KEFIR_IR_DEBUG_ENTRY_TYPE_BOOLEAN:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_CHARACTER:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_CHARACTER:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(define_scalar_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.scalar_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.scalar_type);
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_VOID:
            REQUIRE_OK(define_void_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.void_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.void_type);
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER:
            REQUIRE_OK(define_pointer_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.pointer_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.pointer_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ENUMERATION: {
            REQUIRE_OK(define_enumeration_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.enumeration_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.enumeration_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } break;

        case KEFIR_IR_DEBUG_ENTRY_ENUMERATOR:
            REQUIRE_OK(define_enumerator_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.enumerator));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.enumerator);
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY: {
            REQUIRE_OK(define_array_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.array_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.array_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } break;

        case KEFIR_IR_DEBUG_ENTRY_ARRAY_SUBRANGE: {
            kefir_bool_t has_length;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH, &has_length));
            if (has_length) {
                REQUIRE_OK(define_subrange_type_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) context->abbrev.entries.subrange_type));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.subrange_type);
            } else {
                REQUIRE_OK(define_subrange_nocount_type_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(
                    mem, &context->abbrev.entries.ir_debug_entries, (kefir_hashtree_key_t) ir_debug_entry->identifier,
                    (kefir_hashtree_value_t) context->abbrev.entries.subrange_nocount_type));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.subrange_nocount_type);
            }

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNION: {
            kefir_dwarf_tag_t type_tag = KEFIR_DWARF(DW_TAG_structure_type);
            kefir_codegen_amd64_dwarf_entry_id_t *dwarf_type_ptr = &context->abbrev.entries.structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t *dwarf_anonymous_type_ptr =
                &context->abbrev.entries.anonymous_structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t *dwarf_incomplete_type_ptr =
                &context->abbrev.entries.incomplete_structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t *dwarf_incomplete_anonymous_type_ptr =
                &context->abbrev.entries.incomplete_anonymous_structure_type;
            if (ir_debug_entry->tag == KEFIR_IR_DEBUG_ENTRY_TYPE_UNION) {
                type_tag = KEFIR_DWARF(DW_TAG_union_type);
                dwarf_type_ptr = &context->abbrev.entries.union_type;
                dwarf_anonymous_type_ptr = &context->abbrev.entries.anonymous_union_type;
                dwarf_incomplete_type_ptr = &context->abbrev.entries.incomplete_union_type;
                dwarf_incomplete_anonymous_type_ptr = &context->abbrev.entries.incomplete_anonymous_union_type;
            }

            kefir_bool_t has_name, has_size;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &has_size));
            if (has_name && has_size) {
                REQUIRE_OK(define_structure_type_abbrev(codegen, context, dwarf_type_ptr, type_tag));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) *dwarf_type_ptr));
                ASSIGN_PTR(dwarf_entry_id, *dwarf_type_ptr);
            } else if (!has_name && has_size) {
                REQUIRE_OK(
                    define_anonymous_structure_type_abbrev(codegen, context, dwarf_anonymous_type_ptr, type_tag));
                ASSIGN_PTR(dwarf_entry_id, *dwarf_anonymous_type_ptr);
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) *dwarf_anonymous_type_ptr));
            } else if (has_name && !has_size) {
                REQUIRE_OK(
                    define_incomplete_structure_type_abbrev(codegen, context, dwarf_incomplete_type_ptr, type_tag));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) *dwarf_incomplete_type_ptr));
                ASSIGN_PTR(dwarf_entry_id, *dwarf_incomplete_type_ptr);
            } else if (!has_name && !has_size) {
                REQUIRE_OK(define_incomplete_anonymous_structure_type_abbrev(
                    codegen, context, dwarf_incomplete_anonymous_type_ptr, type_tag));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) *dwarf_incomplete_anonymous_type_ptr));
                ASSIGN_PTR(dwarf_entry_id, *dwarf_incomplete_anonymous_type_ptr);
            }

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } break;

        case KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER: {
            kefir_bool_t has_name, has_bitwidth;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH, &has_bitwidth));
            if (has_name && !has_bitwidth) {
                REQUIRE_OK(define_member_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) context->abbrev.entries.member));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.member);
            } else if (!has_name && !has_bitwidth) {
                REQUIRE_OK(define_anonymous_member_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) context->abbrev.entries.anonymous_member));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.anonymous_member);
            } else if (has_name && has_bitwidth) {
                REQUIRE_OK(define_bitfield_member_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                                 (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                                 (kefir_hashtree_value_t) context->abbrev.entries.bitfield_member));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.bitfield_member);
            } else if (!has_name && has_bitwidth) {
                REQUIRE_OK(define_anonymous_bitfield_member_abbrev(codegen, context));
                REQUIRE_OK(kefir_hashtree_insert(
                    mem, &context->abbrev.entries.ir_debug_entries, (kefir_hashtree_key_t) ir_debug_entry->identifier,
                    (kefir_hashtree_value_t) context->abbrev.entries.anonymous_bitfield_member));
                ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.anonymous_bitfield_member);
            }

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_FUNCTION:
            REQUIRE_OK(define_function_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.subroutine_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.subroutine_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_CONST:
            REQUIRE_OK(define_const_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.const_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.const_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_VOLATILE:
            REQUIRE_OK(define_volatile_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.volatile_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.volatile_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_RESTRICT:
            REQUIRE_OK(define_restrict_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.restrict_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.restrict_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ATOMIC:
            REQUIRE_OK(define_atomic_type_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.atomic_type));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.atomic_type);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER:
            REQUIRE_OK(define_formal_parameter_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.formal_parameter));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.formal_parameter);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG:
            REQUIRE_OK(kefir_codegen_amd64_dwarf_define_unspecified_parameters_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.unspecified_paramters));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.unspecified_paramters);
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPEDEF:
            REQUIRE_OK(define_typedef_abbrev(codegen, context));
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->abbrev.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) context->abbrev.entries.type_def));
            ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.type_def);

            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, NULL));
            break;

        case KEFIR_IR_DEBUG_ENTRY_SUBPROGRAM:
        case KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK:
        case KEFIR_IR_DEBUG_ENTRY_VARIABLE:
        case KEFIR_IR_DEBUG_ENTRY_LABEL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "IR debug entries is not a type");
    }

    return KEFIR_OK;
}

static kefir_result_t generate_type_immediate_info(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                   const struct kefir_ir_module *ir_module,
                                                   struct kefir_codegen_amd64_dwarf_context *context,
                                                   const struct kefir_ir_debug_entry *ir_debug_entry,
                                                   kefir_codegen_amd64_dwarf_entry_id_t dwarf_entry_id) {
    const struct kefir_ir_debug_entry_attribute *ir_attr;
    switch (ir_debug_entry->tag) {
#define DEFINE_SCALAR_TYPE_INFO(_encoding)                                                                         \
    do {                                                                                                           \
        REQUIRE_OK(                                                                                                \
            KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.scalar_type)); \
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,  \
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));             \
        REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));                                    \
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,  \
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));             \
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->size));                                      \
        REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,  \
                                                      KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));        \
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->alignment));                                 \
        REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, (_encoding)));                                        \
    } while (0)

        case KEFIR_IR_DEBUG_ENTRY_TYPE_BOOLEAN:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_boolean));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_CHARACTER:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_signed_char));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_CHARACTER:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_unsigned_char));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_signed));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_unsigned));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_float));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT:
            DEFINE_SCALAR_TYPE_INFO(KEFIR_DWARF(DW_ATE_complex_float));
            break;

#undef DEFINE_SCALAR_TYPE_INFO

        case KEFIR_IR_DEBUG_ENTRY_TYPE_VOID:
            REQUIRE_OK(
                KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.void_type));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, KEFIR_DWARF(DW_ATE_void)));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER: {
            kefir_codegen_amd64_dwarf_entry_id_t pointer_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &pointer_type_entry_id));

            REQUIRE_OK(
                KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.pointer_type));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->size));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->alignment));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, pointer_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ENUMERATION: {
            kefir_codegen_amd64_dwarf_entry_id_t underlying_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &underlying_type_entry_id));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                    context->abbrev.entries.enumeration_type));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->size));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->alignment));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, underlying_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
            kefir_result_t res =
                kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                   KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
            } else {
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ""));
            }

            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, KEFIR_DWARF(null)));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_ENUMERATOR:
            REQUIRE_OK(
                KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.enumerator));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CONSTANT_UINT, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->constant_uint));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY: {
            kefir_codegen_amd64_dwarf_entry_id_t element_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &element_type_entry_id));

            REQUIRE_OK(
                KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.array_type));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, element_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            kefir_result_t res;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, KEFIR_DWARF(null)));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_ARRAY_SUBRANGE: {
            kefir_codegen_amd64_dwarf_entry_id_t index_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &index_type_entry_id));

            kefir_bool_t has_length;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH, &has_length));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(
                &codegen->xasmgen, dwarf_entry_id,
                has_length ? context->abbrev.entries.subrange_type : context->abbrev.entries.subrange_nocount_type));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, index_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

            if (has_length) {
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->length));
            }
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNION: {
            kefir_codegen_amd64_dwarf_entry_id_t dwarf_type = context->abbrev.entries.structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t dwarf_anonymous_type =
                context->abbrev.entries.anonymous_structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t dwarf_incomplete_type =
                context->abbrev.entries.incomplete_structure_type;
            kefir_codegen_amd64_dwarf_entry_id_t dwarf_incomplete_anonymous_type =
                context->abbrev.entries.incomplete_anonymous_structure_type;
            if (ir_debug_entry->tag == KEFIR_IR_DEBUG_ENTRY_TYPE_UNION) {
                dwarf_type = context->abbrev.entries.union_type;
                dwarf_anonymous_type = context->abbrev.entries.anonymous_union_type;
                dwarf_incomplete_type = context->abbrev.entries.incomplete_union_type;
                dwarf_incomplete_anonymous_type = context->abbrev.entries.incomplete_anonymous_union_type;
            }

            kefir_bool_t has_name, has_size;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &has_size));

            if (has_name && has_size) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, dwarf_type));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->size));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->alignment));
            } else if (has_name && !has_size) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, dwarf_incomplete_type));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
            } else if (!has_name && has_size) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, dwarf_anonymous_type));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->size));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->alignment));
            } else if (!has_name && !has_size) {
                REQUIRE_OK(
                    KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, dwarf_incomplete_anonymous_type));
            }

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, KEFIR_DWARF(null)));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER: {
            kefir_codegen_amd64_dwarf_entry_id_t member_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &member_type_entry_id));

            kefir_bool_t has_name, has_bitwidth;
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &has_name));
            REQUIRE_OK(kefir_ir_debug_entry_has_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH, &has_bitwidth));

            if (has_name && !has_bitwidth) {
                REQUIRE_OK(
                    KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.member));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, member_type_entry_id)),
                        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                              KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                              KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->size));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->alignment));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->offset));
            } else if (!has_name && !has_bitwidth) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.anonymous_member));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, member_type_entry_id)),
                        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                              KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                              KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->size));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->alignment));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->offset));
            } else if (has_name && has_bitwidth) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.bitfield_member));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, member_type_entry_id)),
                        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                              KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                              KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->bitoffset));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->bitwidth));
            } else if (!has_name && has_bitwidth) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.anonymous_bitfield_member));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_subtract(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, member_type_entry_id)),
                        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                              KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                              KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->bitoffset));
                REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries,
                                                              ir_debug_entry->identifier,
                                                              KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH, &ir_attr));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, ir_attr->bitwidth));
            }
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_FUNCTION: {
            kefir_codegen_amd64_dwarf_entry_id_t return_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &return_type_entry_id));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                    context->abbrev.entries.subroutine_type));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_FUNCTION_PROTOTYPED_FLAG,
                                                          &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen->xasmgen, ir_attr->function_prototyped ? 1 : 0));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, return_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));

            kefir_result_t res;
            struct kefir_ir_debug_entry_child_iterator iter;
            kefir_ir_debug_entry_id_t child_entry_id;
            for (res = kefir_ir_debug_entry_child_iter(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                       &iter, &child_entry_id);
                 res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_entry_id)) {
                REQUIRE_OK(generate_type_immediate(mem, codegen, ir_module, context, child_entry_id, NULL));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen->xasmgen, KEFIR_DWARF(null)));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER: {
            kefir_codegen_amd64_dwarf_entry_id_t parameter_type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id,
                                                      &parameter_type_entry_id));

            REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                    context->abbrev.entries.formal_parameter));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(
                            &codegen->xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, parameter_type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                    context->abbrev.entries.unspecified_paramters));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_CONST:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_VOLATILE:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_RESTRICT:
        case KEFIR_IR_DEBUG_ENTRY_TYPE_ATOMIC: {
            kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(
                kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, &type_entry_id));

            if (ir_debug_entry->tag == KEFIR_IR_DEBUG_ENTRY_TYPE_CONST) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.const_type));
            } else if (ir_debug_entry->tag == KEFIR_IR_DEBUG_ENTRY_TYPE_VOLATILE) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.volatile_type));
            } else if (ir_debug_entry->tag == KEFIR_IR_DEBUG_ENTRY_TYPE_RESTRICT) {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.restrict_type));
            } else {
                REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id,
                                                        context->abbrev.entries.atomic_type));
            }
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_TYPEDEF: {
            kefir_codegen_amd64_dwarf_entry_id_t type_entry_id;
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE, &ir_attr));
            REQUIRE_OK(
                kefir_codegen_amd64_dwarf_type(mem, codegen, ir_module, context, ir_attr->type_id, &type_entry_id));

            REQUIRE_OK(
                KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen->xasmgen, dwarf_entry_id, context->abbrev.entries.type_def));
            REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&ir_module->debug_info.entries, ir_debug_entry->identifier,
                                                          KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME, &ir_attr));
            REQUIRE_OK(KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, ir_attr->name));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, type_entry_id)),
                    kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION))));
        } break;

        case KEFIR_IR_DEBUG_ENTRY_SUBPROGRAM:
        case KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK:
        case KEFIR_IR_DEBUG_ENTRY_VARIABLE:
        case KEFIR_IR_DEBUG_ENTRY_LABEL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "IR debug entries is not a type");
    }

    return KEFIR_OK;
}

static kefir_result_t generate_type_immediate(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                              const struct kefir_ir_module *ir_module,
                                              struct kefir_codegen_amd64_dwarf_context *context,
                                              kefir_ir_debug_entry_id_t ir_entry_id,
                                              kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    const struct kefir_ir_debug_entry *ir_debug_entry;
    REQUIRE_OK(kefir_ir_debug_entry_get(&ir_module->debug_info.entries, ir_entry_id, &ir_debug_entry));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_type_immediate_abbrev(mem, codegen, ir_module, context, ir_debug_entry, dwarf_entry_id));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        struct kefir_hashtree_node *node;
        kefir_result_t res =
            kefir_hashtree_at(&context->info.entries.ir_debug_entries, (kefir_hashtree_key_t) ir_entry_id, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_PTR(dwarf_entry_id, (kefir_ir_debug_entry_id_t) node->value);
        } else {
            const kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->info.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) entry_id));
            REQUIRE_OK(generate_type_immediate_info(mem, codegen, ir_module, context, ir_debug_entry, entry_id));
            ASSIGN_PTR(dwarf_entry_id, entry_id);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_type(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                              const struct kefir_ir_module *ir_module,
                                              struct kefir_codegen_amd64_dwarf_context *context,
                                              kefir_ir_debug_entry_id_t ir_entry_id,
                                              kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    const struct kefir_ir_debug_entry *ir_debug_entry;
    REQUIRE_OK(kefir_ir_debug_entry_get(&ir_module->debug_info.entries, ir_entry_id, &ir_debug_entry));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_type_immediate_abbrev(mem, codegen, ir_module, context, ir_debug_entry, dwarf_entry_id));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        struct kefir_hashtree_node *node;
        kefir_result_t res =
            kefir_hashtree_at(&context->info.entries.ir_debug_entries, (kefir_hashtree_key_t) ir_entry_id, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_PTR(dwarf_entry_id, (kefir_ir_debug_entry_id_t) node->value);
        } else {
            kefir_codegen_amd64_dwarf_entry_id_t entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
            ASSIGN_PTR(dwarf_entry_id, entry_id);
            REQUIRE_OK(kefir_hashtree_insert(mem, &context->info.entries.ir_debug_entries,
                                             (kefir_hashtree_key_t) ir_debug_entry->identifier,
                                             (kefir_hashtree_value_t) entry_id));
            REQUIRE_OK(kefir_list_insert_after(mem, &context->info.entries.pending_ir_debug_type_entries,
                                               kefir_list_tail(&context->info.entries.pending_ir_debug_type_entries),
                                               (void *) (kefir_uptr_t) ir_entry_id));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_types(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                        const struct kefir_ir_module *ir_module,
                                                        struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    for (struct kefir_list_entry *iter = kefir_list_head(&context->info.entries.pending_ir_debug_type_entries);
         iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_ir_debug_entry_id_t, ir_debug_entry_id, (kefir_uptr_t) iter->value);

        struct kefir_hashtree_node *node;
        REQUIRE_OK(kefir_hashtree_at(&context->info.entries.ir_debug_entries, (kefir_hashtree_key_t) ir_debug_entry_id,
                                     &node));
        ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, dwarf_entry_id, node->value);

        const struct kefir_ir_debug_entry *ir_debug_entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(&ir_module->debug_info.entries, ir_debug_entry_id, &ir_debug_entry));

        REQUIRE_OK(generate_type_immediate_info(mem, codegen, ir_module, context, ir_debug_entry, dwarf_entry_id));

        REQUIRE_OK(kefir_list_pop(mem, &context->info.entries.pending_ir_debug_type_entries, iter));
        iter = kefir_list_head(&context->info.entries.pending_ir_debug_type_entries);
    }
    return KEFIR_OK;
}
