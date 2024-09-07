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

#ifndef KEFIR_CODEGEN_AMD64_DWARF_H_
#define KEFIR_CODEGEN_AMD64_DWARF_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/module.h"
#include "kefir/target/dwarf/dwarf.h"
#include "kefir/target/dwarf/generator.h"

#ifdef KEFIR_CODEGEN_AMD64_DWARF_INTERNAL

typedef kefir_uint64_t kefir_codegen_amd64_dwarf_entry_id_t;
#define KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL ((kefir_codegen_amd64_dwarf_entry_id_t) 0u)

typedef struct kefir_codegen_amd64_dwarf_abbrev_context {
    struct {
        kefir_codegen_amd64_dwarf_entry_id_t next_entry_id;
        kefir_codegen_amd64_dwarf_entry_id_t compile_unit;
        kefir_codegen_amd64_dwarf_entry_id_t void_type;
        kefir_codegen_amd64_dwarf_entry_id_t scalar_type;
        kefir_codegen_amd64_dwarf_entry_id_t pointer_type;
        kefir_codegen_amd64_dwarf_entry_id_t enumeration_type;
        kefir_codegen_amd64_dwarf_entry_id_t enumerator;
        kefir_codegen_amd64_dwarf_entry_id_t array_type;
        kefir_codegen_amd64_dwarf_entry_id_t subrange_type;
        kefir_codegen_amd64_dwarf_entry_id_t subrange_nocount_type;
        kefir_codegen_amd64_dwarf_entry_id_t structure_type;
        kefir_codegen_amd64_dwarf_entry_id_t incomplete_structure_type;
        kefir_codegen_amd64_dwarf_entry_id_t anonymous_structure_type;
        kefir_codegen_amd64_dwarf_entry_id_t incomplete_anonymous_structure_type;
        kefir_codegen_amd64_dwarf_entry_id_t union_type;
        kefir_codegen_amd64_dwarf_entry_id_t incomplete_union_type;
        kefir_codegen_amd64_dwarf_entry_id_t anonymous_union_type;
        kefir_codegen_amd64_dwarf_entry_id_t incomplete_anonymous_union_type;
        kefir_codegen_amd64_dwarf_entry_id_t member;
        kefir_codegen_amd64_dwarf_entry_id_t anonymous_member;
        kefir_codegen_amd64_dwarf_entry_id_t bitfield_member;
        kefir_codegen_amd64_dwarf_entry_id_t anonymous_bitfield_member;
        kefir_codegen_amd64_dwarf_entry_id_t subroutine_type;
        kefir_codegen_amd64_dwarf_entry_id_t formal_parameter;
        kefir_codegen_amd64_dwarf_entry_id_t unspecified_paramters;
        kefir_codegen_amd64_dwarf_entry_id_t type_def;
        kefir_codegen_amd64_dwarf_entry_id_t global_variable;
        kefir_codegen_amd64_dwarf_entry_id_t subprogram;
        kefir_codegen_amd64_dwarf_entry_id_t lexical_block;
        kefir_codegen_amd64_dwarf_entry_id_t local_variable;
        struct kefir_hashtree ir_debug_entries;
    } entries;
} kefir_codegen_amd64_dwarf_abbrev_context_t;

typedef struct kefir_codegen_amd64_dwarf_info_context {
    struct {
        kefir_codegen_amd64_dwarf_entry_id_t next_entry_id;
        kefir_codegen_amd64_dwarf_entry_id_t compile_unit;
        struct kefir_hashtree ir_debug_entries;
        struct kefir_list pending_ir_debug_type_entries;
    } entries;
} kefir_codegen_amd64_dwarf_info_context_t;

typedef struct kefir_codegen_amd64_dwarf_loclists_context {
    struct {
        kefir_codegen_amd64_dwarf_entry_id_t next_entry_id;
        struct kefir_hashtree ir_debug_entries;
    } entries;
} kefir_codegen_amd64_dwarf_loclists_context_t;

typedef struct kefir_codegen_amd64_dwarf_context {
    kefir_dwarf_generator_section_t section;
    struct kefir_codegen_amd64_dwarf_abbrev_context abbrev;
    struct kefir_codegen_amd64_dwarf_info_context info;
    struct kefir_codegen_amd64_dwarf_loclists_context loclists;
} kefir_codegen_amd64_dwarf_context_t;

#define KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(_context) (++(_context)->abbrev.entries.next_entry_id)
#define KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(_context) (++(_context)->info.entries.next_entry_id)
#define KEFIR_CODEGEN_AMD64_DWARF_NEXT_LOCLIST_ENTRY_ID(_context) (++(_context)->loclists.entries.next_entry_id)

kefir_result_t kefir_codegen_amd64_dwarf_context_init(struct kefir_codegen_amd64_dwarf_context *);
kefir_result_t kefir_codegen_amd64_dwarf_context_free(struct kefir_mem *, struct kefir_codegen_amd64_dwarf_context *);

kefir_result_t kefir_codegen_amd64_dwarf_type(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                              const struct kefir_ir_module *,
                                              struct kefir_codegen_amd64_dwarf_context *, kefir_ir_debug_entry_id_t,
                                              kefir_codegen_amd64_dwarf_entry_id_t *);
kefir_result_t kefir_codegen_amd64_dwarf_generate_types(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                                        const struct kefir_ir_module *,
                                                        struct kefir_codegen_amd64_dwarf_context *);
kefir_result_t kefir_codegen_amd64_dwarf_generate_global_identifiers(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                                                     const struct kefir_ir_module *,
                                                                     struct kefir_codegen_amd64_dwarf_context *);
kefir_result_t kefir_codegen_amd64_dwarf_generate_functions(struct kefir_mem *, struct kefir_codegen_amd64_module *,
                                                            struct kefir_codegen_amd64_dwarf_context *);
kefir_result_t kefir_codegen_amd64_dwarf_context_generate_compile_unit(struct kefir_mem *,
                                                                       struct kefir_codegen_amd64_module *,
                                                                       struct kefir_codegen_amd64_dwarf_context *);

kefir_result_t kefir_codegen_amd64_dwarf_generate_lexical_block(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                struct kefir_codegen_amd64_dwarf_context *,
                                                                kefir_ir_debug_entry_id_t,
                                                                kefir_codegen_amd64_dwarf_entry_id_t *);
kefir_result_t kefir_codegen_amd64_dwarf_generate_lexical_block_content(struct kefir_mem *,
                                                                        struct kefir_codegen_amd64_function *,
                                                                        struct kefir_codegen_amd64_dwarf_context *,
                                                                        kefir_ir_debug_entry_id_t);
kefir_result_t kefir_codegen_amd64_dwarf_generate_local_variable(struct kefir_mem *,
                                                                 struct kefir_codegen_amd64_function *,
                                                                 struct kefir_codegen_amd64_dwarf_context *,
                                                                 kefir_ir_debug_entry_id_t,
                                                                 kefir_codegen_amd64_dwarf_entry_id_t *);

#endif

kefir_result_t kefir_codegen_amd64_generate_dwarf_debug_info(struct kefir_mem *, struct kefir_codegen_amd64_module *);

#endif
