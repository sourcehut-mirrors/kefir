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

#include "kefir/codegen/amd64-common.h"
#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/amd64/static_data.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t translate_module_globals(const struct kefir_ir_module *module,
                                               struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator globals_iter;
    kefir_ir_identifier_type_t global_type;
    for (const char *global = kefir_ir_module_globals_iter(module, &globals_iter, &global_type); global != NULL;
         global = kefir_ir_module_globals_iter_next(&globals_iter, &global_type)) {

        if (!codegen->config->emulated_tls || global_type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, "%s", global));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_V, global));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_T, global));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_module_externals(struct kefir_ir_module *module, struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator externals_iter;
    kefir_ir_identifier_type_t external_type;
    for (const char *external = kefir_ir_module_externals_iter(module, &externals_iter, &external_type);
         external != NULL; external = kefir_ir_module_externals_iter_next(&externals_iter, &external_type)) {

        if (!codegen->config->emulated_tls || external_type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", external));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_V, external));
        }
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_SAVE));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_FLOAT32_TO_UINT));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_FLOAT64_TO_UINT));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_INT));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_UINT));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_EQUALS));
    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_MUL));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_DIV));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_MUL));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_DIV));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_MUL));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_DIV));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_RUNTIME_FENV_UPDATE));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT32_NEG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_FLOAT64_NEG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG));
    return KEFIR_OK;
}

static kefir_result_t translate_module_aliases(struct kefir_ir_module *module, struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator aliases_iter;
    const char *original;
    for (const char *alias = kefir_ir_module_aliases_iter(module, &aliases_iter, &original);
         alias != NULL; alias = kefir_ir_module_aliases_iter_next(&aliases_iter, &original)) {

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIAS(&codegen->xasmgen, alias, original));
    }

    return KEFIR_OK;
}

static kefir_result_t translate_data_storage(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             struct kefir_opt_module *module, kefir_ir_data_storage_t storage,
                                             kefir_bool_t defined, const char *section, kefir_uint64_t section_attr) {
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

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, section, section_attr));
            first = false;
        }

        if (defined || (section_attr & KEFIR_AMD64_XASMGEN_SECTION_TLS) != 0) {
            REQUIRE_OK(kefir_codegen_amd64_static_data(mem, codegen, data, identifier));
        } else {
            REQUIRE_OK(kefir_codegen_amd64_static_data_uninit(mem, codegen, data, identifier));
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

        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_T, identifier);
        REQUIRE_OK(kefir_codegen_amd64_static_data(mem, codegen, data, emutls_identifier));
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

        char emutls_identifier[1024] = {0};
        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_V, identifier);

        struct kefir_abi_amd64_type_layout type_layout;
        REQUIRE_OK(kefir_abi_amd64_type_layout(mem, codegen->abi_variant, data->type, &type_layout));
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
                                                                       identifier))
                          : kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[3], 0)));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_strings(struct kefir_codegen_amd64 *codegen, struct kefir_opt_module *module) {

    kefir_bool_t first = true;
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
        if (!public) {
            continue;
        }
        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            first = false;
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_STRING_LITERAL, id));
        switch (literal_type) {
            case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ASCII, 1,
                                                    kefir_asm_amd64_xasmgen_operand_string_literal(
                                                        &codegen->xasmgen_helpers.operands[0], content, length)));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE16:
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, content, length));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE32:
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, content, length));
                break;
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}

static kefir_result_t translate_data(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                     struct kefir_opt_module *module) {
    REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, true, ".data",
                                      KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, false, ".bss",
                                      KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    if (!codegen->config->emulated_tls) {
        REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, true, ".tdata",
                                          KEFIR_AMD64_XASMGEN_SECTION_TLS));
        REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, false, ".tbss",
                                          KEFIR_AMD64_XASMGEN_SECTION_TLS));
    } else {
        REQUIRE_OK(translate_emulated_tls(mem, codegen, module));
    }
    REQUIRE_OK(translate_strings(codegen, module));
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

static kefir_result_t translate_impl(struct kefir_mem *mem, struct kefir_codegen *cg, struct kefir_opt_module *module,
                                     struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64 *, codegen, cg->data);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_PROLOGUE(&codegen->xasmgen));
    REQUIRE_OK(translate_module_globals(module->ir_module, codegen));
    REQUIRE_OK(translate_module_externals(module->ir_module, codegen));
    REQUIRE_OK(translate_module_aliases(module->ir_module, codegen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".text", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        struct kefir_opt_function *func = NULL;
        const struct kefir_opt_code_analysis *func_analysis = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &func));
        REQUIRE_OK(kefir_opt_module_analysis_get_function(analysis, ir_func->declaration->id, &func_analysis));
        REQUIRE_OK(kefir_codegen_amd64_function_translate(mem, codegen, module, func, func_analysis));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }

    REQUIRE_OK(translate_global_inline_assembly(codegen, module));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(translate_data(mem, codegen, module));

    return KEFIR_OK;
}

static kefir_result_t close_impl(struct kefir_mem *mem, struct kefir_codegen *cg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator interface"));
    REQUIRE(cg->data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64 *, codegen, cg->data);
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
    codegen->codegen.translate_optimized = translate_impl;
    codegen->codegen.close = close_impl;
    codegen->codegen.data = codegen;
    codegen->codegen.self = codegen;
    codegen->config = config;
    codegen->abi_variant = abi_variant;
    REQUIRE_OK(build_pipeline(mem, codegen));
    return KEFIR_OK;
}
