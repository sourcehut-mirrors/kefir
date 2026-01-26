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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t generate_lexical_block_abbrev(struct kefir_codegen_amd64 *codegen,
                                                    struct kefir_codegen_amd64_dwarf_context *context) {
    REQUIRE(context->abbrev.entries.lexical_block == KEFIR_CODEGEN_AMD64_DWARF_ENTRY_NULL, KEFIR_OK);
    context->abbrev.entries.lexical_block = KEFIR_CODEGEN_AMD64_DWARF_NEXT_ABBREV_ENTRY_ID(context);

    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV(&codegen->xasmgen, context->abbrev.entries.lexical_block,
                                              KEFIR_DWARF(DW_TAG_lexical_block), KEFIR_DWARF(DW_CHILDREN_yes)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(&codegen->xasmgen, KEFIR_DWARF(DW_AT_ranges),
                                                  KEFIR_DWARF(DW_FORM_sec_offset)));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_ABBREV_END(&codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t generate_lexical_block_info(struct kefir_mem *mem,
                                                  struct kefir_codegen_amd64_dwarf_context *context,
                                                  struct kefir_codegen_amd64_function *codegen_function,
                                                  const struct kefir_opt_module_liveness *liveness,
                                                  kefir_ir_debug_entry_id_t entry_id,
                                                  kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));

    kefir_codegen_amd64_dwarf_entry_id_t rnglist_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_RNGLIST_ENTRY_ID(context);
    REQUIRE_OK(kefir_hashtree_insert(mem, &context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) entry_id,
                                     (kefir_hashtree_value_t) rnglist_entry_id));

    const kefir_codegen_amd64_dwarf_entry_id_t block_entry_id = KEFIR_CODEGEN_AMD64_DWARF_NEXT_INFO_ENTRY_ID(context);
    ASSIGN_PTR(dwarf_entry_id, block_entry_id);
    REQUIRE_OK(KEFIR_AMD64_DWARF_ENTRY_INFO(&codegen_function->codegen->xasmgen, block_entry_id,
                                            context->abbrev.entries.lexical_block));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen_function->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_DWARF_DEBUG_RNGLIST_ENTRY, rnglist_entry_id))));

    REQUIRE_OK(
        kefir_codegen_amd64_dwarf_generate_lexical_block_content(mem, codegen_function, context, liveness, entry_id));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(&codegen_function->codegen->xasmgen, KEFIR_DWARF(null)));
    return KEFIR_OK;
}

static kefir_result_t generate_lexical_block_ranges(struct kefir_mem *mem,
                                                    struct kefir_codegen_amd64_dwarf_context *context,
                                                    struct kefir_codegen_amd64_function *codegen_function,
                                                    const struct kefir_opt_module_liveness *liveness,
                                                    kefir_ir_debug_entry_id_t entry_id,
                                                    kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    UNUSED(mem);
    UNUSED(liveness);
    UNUSED(dwarf_entry_id);
    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&context->loclists.entries.ir_debug_entries, (kefir_hashtree_key_t) entry_id, &node));
    ASSIGN_DECL_CAST(kefir_codegen_amd64_dwarf_entry_id_t, rnglist_entry_id, node->value);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_function->codegen->xasmgen, KEFIR_AMD64_DWARF_DEBUG_RNGLIST_ENTRY,
                                         rnglist_entry_id));

    const struct kefir_ir_debug_entry_attribute *attr;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN, &attr));
    const kefir_size_t block_begin_idx = attr->code_index;
    REQUIRE_OK(kefir_ir_debug_entry_get_attribute(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                                  KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END, &attr));
    const kefir_size_t block_end_idx = attr->code_index;

    REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_range_list(mem, codegen_function, block_begin_idx, block_end_idx));

    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_LLE_end_of_list)));

    REQUIRE_OK(
        kefir_codegen_amd64_dwarf_generate_lexical_block_content(mem, codegen_function, context, liveness, entry_id));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_lexical_block(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *codegen_function,
                                                                struct kefir_codegen_amd64_dwarf_context *context,
                                                                const struct kefir_opt_module_liveness *liveness,
                                                                kefir_ir_debug_entry_id_t entry_id,
                                                                kefir_codegen_amd64_dwarf_entry_id_t *dwarf_entry_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));

    const struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(kefir_ir_debug_entry_get(&codegen_function->module->ir_module->debug_info.entries, entry_id, &entry));

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_ABBREV) {
        REQUIRE_OK(generate_lexical_block_abbrev(codegen_function->codegen, context));
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block_content(mem, codegen_function, context, liveness,
                                                                            entry_id));
        ASSIGN_PTR(dwarf_entry_id, context->abbrev.entries.lexical_block);
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        REQUIRE_OK(generate_lexical_block_info(mem, context, codegen_function, liveness, entry_id, dwarf_entry_id));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS) {
        REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block_content(mem, codegen_function, context, liveness,
                                                                            entry_id));
    }

    KEFIR_DWARF_GENERATOR_SECTION(context->section, KEFIR_DWARF_GENERATOR_SECTION_RNGLISTS) {
        REQUIRE_OK(generate_lexical_block_ranges(mem, context, codegen_function, liveness, entry_id, dwarf_entry_id));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_lexical_block_content(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    struct kefir_codegen_amd64_dwarf_context *context, const struct kefir_opt_module_liveness *liveness,
    kefir_ir_debug_entry_id_t entry_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen DWARF context"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module liveness"));

    kefir_ir_debug_entry_id_t child_id;
    struct kefir_ir_debug_entry_child_iterator iter;
    kefir_result_t res;
    for (res = kefir_ir_debug_entry_child_iter(&codegen_function->module->ir_module->debug_info.entries, entry_id,
                                               &iter, &child_id);
         res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&iter, &child_id)) {

        const struct kefir_ir_debug_entry *child_entry;
        REQUIRE_OK(
            kefir_ir_debug_entry_get(&codegen_function->module->ir_module->debug_info.entries, child_id, &child_entry));

        switch (child_entry->tag) {
            case KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK:
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_lexical_block(mem, codegen_function, context, liveness,
                                                                            child_id, NULL));
                break;

            case KEFIR_IR_DEBUG_ENTRY_VARIABLE:
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_variable(mem, codegen_function, context, liveness,
                                                                       child_id, NULL));
                break;

            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER:
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_function_parameter(mem, codegen_function, context,
                                                                                 child_id, NULL));
                break;

            case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG:
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_unspecified_function_parameter(mem, codegen_function,
                                                                                             context, child_id, NULL));
                break;

            case KEFIR_IR_DEBUG_ENTRY_LABEL:
                REQUIRE_OK(kefir_codegen_amd64_dwarf_generate_label(mem, codegen_function, context, child_id, NULL));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR debug entry tag");
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
