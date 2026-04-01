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
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_amd64_dwarf_context_init(struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen DWARF context"));

    memset(context, 0, sizeof(struct kefir_codegen_amd64_dwarf_context));
    REQUIRE_OK(kefir_hashtree_init(&context->abbrev.entries.ir_debug_entries, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&context->info.entries.ir_debug_entries, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&context->info.entries.pending_ir_debug_type_entries));
    REQUIRE_OK(kefir_hashtree_init(&context->loclists.entries.ir_debug_entries, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&context->strings.entries.strings, &kefir_hashtree_str_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_context_free(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    REQUIRE_OK(kefir_hashtree_free(mem, &context->strings.entries.strings));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->loclists.entries.ir_debug_entries));
    REQUIRE_OK(kefir_list_free(mem, &context->info.entries.pending_ir_debug_type_entries));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->info.entries.ir_debug_entries));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->abbrev.entries.ir_debug_entries));
    memset(context, 0, sizeof(struct kefir_codegen_amd64_dwarf_context));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_generate_dwarf_debug_info(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_module *codegen_module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));

    struct kefir_codegen_amd64_dwarf_context context;
    REQUIRE_OK(kefir_codegen_amd64_dwarf_context_init(&context));
    KEFIR_DWARF_GENERATOR_DEBUG_INFO(&context.section) {
        kefir_result_t res = KEFIR_AMD64_DWARF_SECTION_INIT(&codegen_module->codegen->xasmgen, context.section,
                                                            codegen_module->codegen->symbol_prefix);
        REQUIRE_CHAIN(&res, kefir_codegen_amd64_dwarf_context_generate_compile_unit(mem, codegen_module, &context));
        REQUIRE_CHAIN(&res, KEFIR_AMD64_DWARF_SECTION_FINALIZE(&codegen_module->codegen->xasmgen, context.section,
                                                               codegen_module->codegen->symbol_prefix));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_codegen_amd64_dwarf_context_free(mem, &context);
            return res;
        });
    }
    REQUIRE_OK(kefir_codegen_amd64_dwarf_context_free(mem, &context));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_insert_string(struct kefir_mem *mem,
                                                       struct kefir_codegen_amd64_dwarf_context *context,
                                                       const char *string,
                                                       kefir_codegen_amd64_dwarf_entry_id_t *entry_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 dwarf context"));
    REQUIRE(string != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));
    REQUIRE(entry_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to DWARF entry identifier"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->strings.entries.strings, (kefir_hashtree_key_t) string, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *entry_id_ptr = (kefir_asmcmp_label_index_t) node->value;
        return KEFIR_OK;
    }

    kefir_codegen_amd64_dwarf_entry_id_t entry_id = context->strings.entries.next_string_id++;
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->strings.entries.strings, (kefir_hashtree_key_t) string,
                                     (kefir_hashtree_value_t) entry_id));
    *entry_id_ptr = entry_id;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_strp(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                                       struct kefir_codegen_amd64_dwarf_context *context,
                                                       const char *symbol_prefix, const char *string) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 xasmgen"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 dwarf context"));
    REQUIRE(string != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));

    kefir_codegen_amd64_dwarf_entry_id_t entry_id;
    REQUIRE_OK(kefir_codegen_amd64_dwarf_insert_string(mem, context, string, &entry_id));

    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&xasmgen_helpers, KEFIR_AMD64_DWARF_DEBUG_STR_ENTRY, symbol_prefix,
                                                   entry_id))));
    return KEFIR_OK;
}