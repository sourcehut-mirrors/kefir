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

#include "kefir/codegen/amd64-common.h"
#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/amd64/static_data.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/lowering.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t translate_module_identifiers(const struct kefir_ir_module *module,
                                                   struct kefir_codegen_amd64 *codegen,
                                                   struct kefir_opt_module_liveness *liveness) {
    struct kefir_hashtree_node_iterator identifiers_iter;
    const struct kefir_ir_identifier *identifier;
    static const char BUILTIN_PREFIX[] = "__kefir_builtin";
    for (const char *symbol = kefir_ir_module_identifiers_iter(module, &identifiers_iter, &identifier); symbol != NULL;
         symbol = kefir_ir_module_identifiers_next(&identifiers_iter, &identifier)) {

        if (identifier->alias != NULL) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIAS(&codegen->xasmgen, identifier->symbol, identifier->alias));
        }

        kefir_amd64_xasmgen_type_attribute_t type = KEFIR_AMD64_XASMGEN_TYPE_FUNCTION;
        kefir_amd64_xasmgen_visibility_attribute_t visibility = KEFIR_AMD64_XASMGEN_VISIBILITY_DEFAULT;
        switch (identifier->type) {
            case KEFIR_IR_IDENTIFIER_GLOBAL_DATA:
            case KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA:
                type = KEFIR_AMD64_XASMGEN_TYPE_DATA;
                break;

            case KEFIR_IR_IDENTIFIER_FUNCTION:
                // Intentionally left blank
                break;
        }

        switch (identifier->visibility) {
            case KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT:
                // Intentionally left blank
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_HIDDEN:
                visibility = KEFIR_AMD64_XASMGEN_VISIBILITY_HIDDEN;
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_INTERNAL:
                visibility = KEFIR_AMD64_XASMGEN_VISIBILITY_INTERNAL;
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_PROTECTED:
                visibility = KEFIR_AMD64_XASMGEN_VISIBILITY_PROTECTED;
                break;
        }

        switch (identifier->scope) {
            case KEFIR_IR_IDENTIFIER_SCOPE_EXPORT:

                if (!codegen->config->emulated_tls || identifier->type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA) {
                    REQUIRE_OK(
                        KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, type, visibility, "%s", identifier->symbol));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, type, visibility, KEFIR_AMD64_EMUTLS_V,
                                                          identifier->symbol));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, type, visibility, KEFIR_AMD64_EMUTLS_T,
                                                          identifier->symbol));
                }
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK:
                if (!codegen->config->emulated_tls || identifier->type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_WEAK(&codegen->xasmgen, type, visibility, "%s", identifier->symbol));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_WEAK(&codegen->xasmgen, type, visibility, KEFIR_AMD64_EMUTLS_V,
                                                        identifier->symbol));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_WEAK(&codegen->xasmgen, type, visibility, KEFIR_AMD64_EMUTLS_T,
                                                        identifier->symbol));
                }
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_IMPORT:
                if (strncmp(BUILTIN_PREFIX, symbol, sizeof(BUILTIN_PREFIX) - 1) != 0) {
                    if (kefir_opt_module_is_symbol_alive(liveness, symbol)) {
                        if (!codegen->config->emulated_tls ||
                            identifier->type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA) {
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", identifier->symbol));
                        } else {
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_V,
                                                                    identifier->symbol));
                        }
                    }
                }
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_LOCAL:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_data_storage(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             struct kefir_opt_module *module,
                                             struct kefir_opt_module_liveness *liveness,
                                             kefir_ir_data_storage_t storage, kefir_bool_t defined, const char *section,
                                             kefir_uint64_t section_attr) {
    bool first = true;
    struct kefir_hashtree_node_iterator iter;
    const char *identifier = NULL;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->ir_module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != storage) {
            continue;
        }
        if (data->defined != defined) {
            continue;
        }

        if (!kefir_opt_module_is_symbol_alive(liveness, identifier)) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, section, section_attr));
            first = false;
        }

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(module->ir_module, identifier, &ir_identifier));

        if (defined || (section_attr & KEFIR_AMD64_XASMGEN_SECTION_TLS) != 0) {
            REQUIRE_OK(kefir_codegen_amd64_static_data(mem, codegen, module->ir_module, data, ir_identifier->symbol));
        } else {
            REQUIRE_OK(kefir_codegen_amd64_static_data_uninit(mem, codegen, data, ir_identifier->symbol));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_emulated_tls(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             struct kefir_opt_module *module) {
    bool first = true;
    struct kefir_hashtree_node_iterator iter;
    const char *identifier = NULL;
    char emutls_identifier[1024] = {0};
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->ir_module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != KEFIR_IR_DATA_THREAD_LOCAL_STORAGE) {
            continue;
        }

        if (!data->defined) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            first = false;
        }

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(module->ir_module, identifier, &ir_identifier));

        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_T, ir_identifier->symbol);
        REQUIRE_OK(kefir_codegen_amd64_static_data(mem, codegen, module->ir_module, data, emutls_identifier));
    }

    first = true;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->ir_module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != KEFIR_IR_DATA_THREAD_LOCAL_STORAGE) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".data", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            first = false;
        }

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(module->ir_module, identifier, &ir_identifier));

        char emutls_identifier[1024] = {0};
        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_V, ir_identifier->symbol);

        struct kefir_abi_amd64_type_layout type_layout;
        REQUIRE_OK(kefir_abi_amd64_type_layout(mem, codegen->abi_variant, KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GLOBAL,
                                               data->type, &type_layout));
        kefir_size_t total_size, total_alignment;
        kefir_result_t res =
            kefir_abi_amd64_calculate_type_properties(data->type, &type_layout, &total_size, &total_alignment);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_amd64_type_layout_free(mem, &type_layout);
            return res;
        });
        REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", emutls_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 4,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], total_alignment),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[2], 0),
            data->defined ? kefir_asm_amd64_xasmgen_operand_label(
                                &codegen->xasmgen_helpers.operands[3], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_T,
                                                                       ir_identifier->symbol))
                          : kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[3], 0)));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_strings(struct kefir_codegen_amd64 *codegen, struct kefir_opt_module *module,
                                        struct kefir_opt_module_liveness *liveness, kefir_bool_t *has_rodata) {
    struct kefir_hashtree_node_iterator iter;
    kefir_id_t id;
    kefir_ir_string_literal_type_t literal_type;
    kefir_bool_t public;
    const void *content = NULL;
    kefir_size_t length = 0;
    kefir_result_t res = KEFIR_OK;
    for (res = kefir_ir_module_string_literal_iter(module->ir_module, &iter, &id, &literal_type, &public, &content,
                                                   &length);
         res == KEFIR_OK;
         res = kefir_ir_module_string_literal_next(&iter, &id, &literal_type, &public, &content, &length)) {
        if (!public || !kefir_opt_module_is_string_literal_alive(liveness, id)) {
            continue;
        }
        if (!*has_rodata) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            *has_rodata = true;
        }

        switch (literal_type) {
            case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL, id));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ASCII, 1,
                                                    kefir_asm_amd64_xasmgen_operand_string_literal(
                                                        &codegen->xasmgen_helpers.operands[0], content, length)));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE16:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 2));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL, id));
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, content, length));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE32:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL, id));
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, content, length));
                break;
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}

static kefir_result_t translate_constants(struct kefir_codegen_amd64_module *codegen_module, kefir_bool_t *has_rodata) {
#define DECLARE_RODATA                                                                           \
    do {                                                                                         \
        if (!*has_rodata) {                                                                      \
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen_module->codegen->xasmgen, ".rodata", \
                                                   KEFIR_AMD64_XASMGEN_SECTION_NOATTR));         \
            *has_rodata = true;                                                                  \
        }                                                                                        \
    } while (0)

    if (codegen_module->constants.float32_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 4));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1593835520)));
    }

    if (codegen_module->constants.float64_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 8));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 2,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[1], 1138753536)));
    }

    if (codegen_module->constants.long_double_to_uint) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 4));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_LONG_DOUBLE_TO_UINT));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1593835520)));
    }

    if (codegen_module->constants.uint_to_long_double) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 1602224128)));
    }

    if (codegen_module->constants.float32_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT32_NEG));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.float64_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT64_NEG));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 2147483648)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.complex_float32_mul) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_MUL));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float32_div) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_DIV));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float64_mul) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_MUL));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_long_double_div) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_LONG_DOUBLE_DIV));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x3fff)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
    }

    if (codegen_module->constants.complex_float32_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }

    if (codegen_module->constants.complex_float64_neg) {
        DECLARE_RODATA;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s",
                                             KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen_module->codegen->xasmgen_helpers.operands[0], 0x80000000)));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_data(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module) {
    REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module, codegen_module->liveness,
                                      KEFIR_IR_DATA_GLOBAL_STORAGE, true, ".data", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module, codegen_module->liveness,
                                      KEFIR_IR_DATA_GLOBAL_READONLY_STORAGE, true, ".rodata",
                                      KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module, codegen_module->liveness,
                                      KEFIR_IR_DATA_GLOBAL_STORAGE, false, ".bss", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module, codegen_module->liveness,
                                      KEFIR_IR_DATA_GLOBAL_READONLY_STORAGE, false, ".bss",
                                      KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    if (!codegen_module->codegen->config->emulated_tls) {
        REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module,
                                          codegen_module->liveness, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, true, ".tdata",
                                          KEFIR_AMD64_XASMGEN_SECTION_TLS));
        REQUIRE_OK(translate_data_storage(mem, codegen_module->codegen, codegen_module->module,
                                          codegen_module->liveness, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, false, ".tbss",
                                          KEFIR_AMD64_XASMGEN_SECTION_TLS));
    } else {
        REQUIRE_OK(translate_emulated_tls(mem, codegen_module->codegen, codegen_module->module));
    }

    kefir_bool_t has_rodata = false;
    REQUIRE_OK(translate_constants(codegen_module, &has_rodata));
    REQUIRE_OK(
        translate_strings(codegen_module->codegen, codegen_module->module, codegen_module->liveness, &has_rodata));
    return KEFIR_OK;
}

static kefir_result_t translate_global_inline_assembly(struct kefir_codegen_amd64 *codegen,
                                                       struct kefir_opt_module *module) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->ir_module->global_inline_asm, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly *, inline_asm, node->value);
        REQUIRE(kefir_hashtree_empty(&inline_asm->parameters),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Global IR inline assembly cannot have any parameters"));
        REQUIRE(kefir_hashtree_empty(&inline_asm->clobbers),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Global IR inline assembly cannot have any clobbers"));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Inline assembly fragment #%" KEFIR_ID_FMT, inline_asm->id));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&codegen->xasmgen, inline_asm->template));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }
    return KEFIR_OK;
}

static kefir_result_t generate_init_array(struct kefir_codegen_amd64_module *codegen_module) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen_module->codegen->xasmgen, ".init_array",
                                           KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 8));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func =
             kefir_ir_module_function_iter(codegen_module->module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        if (!ir_func->flags.constructor) {
            continue;
        }

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_module->module->ir_module, ir_func->name, &ir_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(&codegen_module->codegen->xasmgen_helpers.operands[0],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, ir_identifier->symbol),
            0));
    }
    return KEFIR_OK;
}

static kefir_result_t generate_fini_array(struct kefir_codegen_amd64_module *codegen_module) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen_module->codegen->xasmgen, ".fini_array",
                                           KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen_module->codegen->xasmgen, 8));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func =
             kefir_ir_module_function_iter(codegen_module->module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        if (!ir_func->flags.destructor) {
            continue;
        }

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(codegen_module->module->ir_module, ir_func->name, &ir_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen_module->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_label(&codegen_module->codegen->xasmgen_helpers.operands[0],
                                                  KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, ir_identifier->symbol),
            0));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module) {
    REQUIRE_OK(kefir_opt_module_liveness_trace(mem, codegen_module->liveness, codegen_module->module));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_PROLOGUE(&codegen_module->codegen->xasmgen));
    REQUIRE_OK(translate_module_identifiers(codegen_module->module->ir_module, codegen_module->codegen,
                                            codegen_module->liveness));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_SECTION(&codegen_module->codegen->xasmgen, ".text", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    if (!codegen_module->codegen->config->runtime_function_generator_mode) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_TEXT_SECTION_BEGIN));
    }

    kefir_bool_t has_constructors = false;
    kefir_bool_t has_destructors = false;
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func =
             kefir_ir_module_function_iter(codegen_module->module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        if (!kefir_opt_module_is_symbol_alive(codegen_module->liveness, (const char *) iter.node->key)) {
            continue;
        }
        struct kefir_opt_function *func = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(codegen_module->module, ir_func->declaration->id, &func));

        struct kefir_codegen_amd64_function *codegen_func;
        REQUIRE_OK(kefir_codegen_amd64_module_insert_function(mem, codegen_module, func, &codegen_func));
        REQUIRE_OK(kefir_codegen_amd64_function_translate(mem, codegen_func));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));

        if (ir_func->flags.constructor) {
            has_constructors = true;
        }
        if (ir_func->flags.destructor) {
            has_destructors = true;
        }
    }

    REQUIRE_OK(translate_global_inline_assembly(codegen_module->codegen, codegen_module->module));
    if (!codegen_module->codegen->config->runtime_function_generator_mode) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen_module->codegen->xasmgen, "%s", KEFIR_AMD64_TEXT_SECTION_END));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));
    REQUIRE_OK(translate_data(mem, codegen_module));

    if (has_constructors) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));
        REQUIRE_OK(generate_init_array(codegen_module));
    }
    if (has_destructors) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));
        REQUIRE_OK(generate_fini_array(codegen_module));
    }

    if (codegen_module->codegen->config->debug_info) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen_module->codegen->xasmgen, 1));
        REQUIRE_OK(kefir_codegen_amd64_generate_dwarf_debug_info(mem, codegen_module));
    }

    if (!codegen_module->codegen->config->runtime_function_generator_mode &&
        !kefir_hashtreeset_empty(&codegen_module->required_runtime_functions)) {
        REQUIRE(codegen_module->codegen->runtime_hooks != NULL &&
                    codegen_module->codegen->runtime_hooks->generate_runtime_functions != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to generate required runtime functions"));
        REQUIRE_OK(codegen_module->codegen->runtime_hooks->generate_runtime_functions(
            mem, kefir_asm_amd64_xasmgen_get_output(&codegen_module->codegen->xasmgen),
            &codegen_module->required_runtime_functions, codegen_module->codegen->runtime_hooks->payload));
    }

    return KEFIR_OK;
}

static kefir_result_t translate_fn(struct kefir_mem *mem, struct kefir_codegen *cg, struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64 *, codegen, cg->data);

    struct kefir_codegen_amd64_module codegen_module;
    struct kefir_opt_module_liveness liveness;
    REQUIRE_OK(kefir_opt_module_liveness_init(&liveness));
    REQUIRE_OK(kefir_codegen_amd64_module_init(&codegen_module, codegen, module, &liveness));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_codegen_amd64_lower_module(mem, &codegen_module, module));
    REQUIRE_CHAIN(&res, translate_impl(mem, &codegen_module));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_module_liveness_free(mem, &liveness);
        kefir_codegen_amd64_module_free(mem, &codegen_module);
        return res;
    });
    res = kefir_opt_module_liveness_free(mem, &liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_amd64_module_free(mem, &codegen_module);
        return res;
    });
    REQUIRE_OK(kefir_codegen_amd64_module_free(mem, &codegen_module));
    return res;
}

static kefir_result_t close_impl(struct kefir_mem *mem, struct kefir_codegen *cg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator interface"));
    REQUIRE(cg->data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64 *, codegen, cg->data);
    if (codegen->debug_info_tracker != NULL) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_FREE_DEBUG_INFO_TRACKER(mem, &codegen->xasmgen, codegen->debug_info_tracker));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_CLOSE(mem, &codegen->xasmgen));
    REQUIRE_OK(kefir_asmcmp_pipeline_free(mem, &codegen->pipeline));
    return KEFIR_OK;
}

static kefir_result_t build_pipeline(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen) {
    REQUIRE(codegen->config->pipeline_spec != NULL, KEFIR_OK);

    const char *spec_iter = codegen->config->pipeline_spec;
    while (spec_iter != NULL && *spec_iter != '\0') {
        char pass_spec[256];

        const char *next_delim = strchr(spec_iter, ',');
        if (next_delim == NULL) {
            snprintf(pass_spec, sizeof(pass_spec), "%s", spec_iter);
            spec_iter = NULL;
        } else {
            snprintf(pass_spec, sizeof(pass_spec), "%.*s", (int) (next_delim - spec_iter), spec_iter);
            spec_iter = next_delim + 1;
        }

        if (*pass_spec == '\0') {
            continue;
        }

        const struct kefir_asmcmp_pipeline_pass *pass;
        REQUIRE_OK(kefir_asmcmp_pipeline_pass_resolve(pass_spec, &pass));
        REQUIRE_OK(kefir_asmcmp_pipeline_add(mem, &codegen->pipeline, pass));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_init(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, FILE *output,
                                        kefir_abi_amd64_variant_t abi_variant,
                                        const struct kefir_codegen_runtime_hooks *runtime_hooks,
                                        const struct kefir_codegen_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen amd64"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid FILE"));
    if (config == NULL) {
        config = &KefirCodegenDefaultConfiguration;
    }

    kefir_asm_amd64_xasmgen_syntax_t syntax = KEFIR_AMD64_XASMGEN_SYNTAX_ATT;
    REQUIRE_OK(kefir_codegen_match_syntax(config->syntax, &syntax));
    REQUIRE_OK(kefir_asm_amd64_xasmgen_init(mem, &codegen->xasmgen, output, syntax));
    REQUIRE_OK(kefir_asmcmp_pipeline_init(&codegen->pipeline));
    codegen->codegen.translate_optimized = translate_fn;
    codegen->codegen.close = close_impl;
    codegen->codegen.data = codegen;
    codegen->codegen.self = codegen;
    codegen->config = config;
    codegen->abi_variant = abi_variant;
    codegen->runtime_hooks = runtime_hooks;
    REQUIRE_OK(build_pipeline(mem, codegen));
    if (config->debug_info) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEW_DEBUG_INFO_TRACKER(mem, &codegen->xasmgen, &codegen->debug_info_tracker));
    } else {
        codegen->debug_info_tracker = NULL;
    }
    return KEFIR_OK;
}
