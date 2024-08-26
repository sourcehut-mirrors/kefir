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

#ifndef KEFIR_TARGET_DWARF_GENERATOR_H_
#define KEFIR_TARGET_DWARF_GENERATOR_H_

#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/target/dwarf/dwarf.h"

#define KEFIR_AMD64_DWARF_DEBUG_ABBREV_BEGIN "__kefir_debug_abbrev_section_begin"
#define KEFIR_AMD64_DWARF_DEBUG_ABBREV_ENTRY "__kefir_debug_abbrev_section_entry_%" KEFIR_UINT64_FMT
#define KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION "__kefir_debug_info_section"
#define KEFIR_AMD64_DWARF_DEBUG_INFO_BEGIN "__kefir_debug_info_section_begin"
#define KEFIR_AMD64_DWARF_DEBUG_INFO_END "__kefir_debug_info_section_end"
#define KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY "__kefir_debug_info_section_entry_%" KEFIR_UINT64_FMT
#define KEFIR_AMD64_DWARF_DEBUG_LINES "__kefir_debug_lines_section_begin"

kefir_result_t kefir_amd64_dwarf_byte(struct kefir_amd64_xasmgen *, kefir_uint8_t);
kefir_result_t kefir_amd64_dwarf_word(struct kefir_amd64_xasmgen *, kefir_uint16_t);
kefir_result_t kefir_amd64_dwarf_qword(struct kefir_amd64_xasmgen *, kefir_uint64_t);
kefir_result_t kefir_amd64_dwarf_string(struct kefir_amd64_xasmgen *, const char *);
kefir_result_t kefir_amd64_dwarf_uleb128(struct kefir_amd64_xasmgen *, kefir_uint64_t);

kefir_result_t kefir_amd64_dwarf_attribute_abbrev(struct kefir_amd64_xasmgen *, kefir_dwarf_attribute_t,
                                                  kefir_dwarf_form_t);
kefir_result_t kefir_amd64_dwarf_entry_abbrev(struct kefir_amd64_xasmgen *, kefir_uint64_t, kefir_dwarf_tag_t,
                                              kefir_dwarf_children_t);
kefir_result_t kefir_amd64_dwarf_entry_info_begin(struct kefir_amd64_xasmgen *, kefir_uint64_t, kefir_uint64_t);

typedef enum kefir_dwarf_generator_section {
    KEFIR_DWARF_GENERATOR_SECTION_INIT = 0,
    KEFIR_DWARF_GENERATOR_SECTION_ABBREV = 0,
    KEFIR_DWARF_GENERATOR_SECTION_INFO = 1,
    KEFIR_DWARF_GENERATOR_SECTION_LINES = 2,
    KEFIR_DWARF_GENERATOR_SECTION_COUNT
} kefir_dwarf_generator_section_t;

kefir_result_t kefir_dwarf_generator_section_init(struct kefir_amd64_xasmgen *, kefir_dwarf_generator_section_t);
kefir_result_t kefir_dwarf_generator_section_finalize(struct kefir_amd64_xasmgen *, kefir_dwarf_generator_section_t);

#define KEFIR_AMD64_DWARF_BYTE(_xasmgen, _value) ((kefir_amd64_dwarf_byte((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_WORD(_xasmgen, _value) ((kefir_amd64_dwarf_word((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_QWORD(_xasmgen, _value) ((kefir_amd64_dwarf_qword((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_STRING(_xasmgen, _value) ((kefir_amd64_dwarf_string((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_ULEB128(_xasmgen, _value) ((kefir_amd64_dwarf_uleb128((_xasmgen), (_value))))

#define KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(_xasmgen, _attr, _form) \
    (kefir_amd64_dwarf_attribute_abbrev((_xasmgen), (_attr), (_form)))
#define KEFIR_AMD64_DWARF_ENTRY_ABBREV(_xasmgen, _identifier, _tag, _children) \
    (kefir_amd64_dwarf_entry_abbrev((_xasmgen), (_identifier), (_tag), (_children)))
#define KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(_xasmgen) \
    (KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV((_xasmgen), KEFIR_DWARF(DW_AT_end_of_list), KEFIR_DWARF(DW_FORM_end_of_list)))
#define KEFIR_AMD64_DWARF_ENTRY_INFO(_xasmgen, _identifier, _abbrev_identifier) \
    (kefir_amd64_dwarf_entry_info_begin((_xasmgen), (_identifier), (_abbrev_identifier)))
#define KEFIR_AMD64_DWARF_ENTRY_INFO_CHILDREN_END(_xasmgen) (KEFIR_AMD64_DWARF_ULEB128((_xasmgen), 0))

#define KEFIR_AMD64_DWARF_SECTION_INIT(_xasmgen, _section) (kefir_dwarf_generator_section_init((_xasmgen), (_section)))
#define KEFIR_AMD64_DWARF_SECTION_FINALIZE(_xasmgen, _section) \
    (kefir_dwarf_generator_section_finalize((_xasmgen), (_section)))
#define KEFIR_DWARF_GENERATOR_DEBUG_INFO(_section)                                                            \
    for (*(_section) = KEFIR_DWARF_GENERATOR_SECTION_INIT; *(_section) < KEFIR_DWARF_GENERATOR_SECTION_COUNT; \
         (*(_section))++)

#define KEFIR_DWARF_GENERATOR_SECTION(_section, _desired) if ((_section) == (_desired))

#endif
