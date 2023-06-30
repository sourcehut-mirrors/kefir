#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/codegen/amd64-common.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/static_data.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t translate_module_globals(const struct kefir_ir_module *module,
                                               struct kefir_codegen_opt_amd64 *codegen) {
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

static kefir_result_t translate_module_externals(struct kefir_ir_module *module,
                                                 struct kefir_codegen_opt_amd64 *codegen) {
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

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_SAVE_REGISTERS));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG));
    return KEFIR_OK;
}

static kefir_result_t translate_data_storage(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                             struct kefir_opt_module *module, kefir_ir_data_storage_t storage,
                                             kefir_bool_t defined, const char *section) {
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
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, section));
            first = false;
        }
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_static_data(mem, codegen, data, identifier));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_emulated_tls(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
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
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata"));
            first = false;
        }

        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_T, identifier);
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_static_data(mem, codegen, data, emutls_identifier));
    }

    first = true;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->ir_module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != KEFIR_IR_DATA_THREAD_LOCAL_STORAGE) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".data"));
            first = false;
        }

        char emutls_identifier[1024] = {0};
        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_V, identifier);

        struct kefir_abi_sysv_amd64_type_layout type_layout;
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(data->type, mem, &type_layout));
        kefir_size_t total_size, total_alignment;
        kefir_result_t res =
            kefir_abi_sysv_amd64_calculate_type_properties(data->type, &type_layout, &total_size, &total_alignment);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_sysv_amd64_type_layout_free(mem, &type_layout);
            return res;
        });
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &type_layout));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", emutls_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 4,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], total_alignment),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[2], 0),
            data->defined ? kefir_asm_amd64_xasmgen_operand_label(
                                &codegen->xasmgen_helpers.operands[3],
                                kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_T,
                                                                       identifier))
                          : kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[3], 0)));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_strings(struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_module *module) {

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
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata"));
            first = false;
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSTEM_V_STRING_LITERAL, id));
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

static kefir_result_t translate_data(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_opt_module *module) {
    REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, true, ".data"));
    REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, false, ".bss"));
    if (!codegen->config->emulated_tls) {
        REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, true, ".tdata"));
        REQUIRE_OK(translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, false, ".tbss"));
    } else {
        REQUIRE_OK(translate_emulated_tls(mem, codegen, module));
    }
    REQUIRE_OK(translate_strings(codegen, module));
    return KEFIR_OK;
}

static kefir_result_t translate_global_inline_assembly(struct kefir_codegen_opt_amd64 *codegen,
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
            KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Inline assembly fragment #" KEFIR_ID_FMT, inline_asm->id));
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
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64 *, codegen, cg->data);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_PROLOGUE(&codegen->xasmgen));
    REQUIRE_OK(translate_module_globals(module->ir_module, codegen));
    REQUIRE_OK(translate_module_externals(module->ir_module, codegen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".text"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        const struct kefir_opt_function *func = NULL;
        const struct kefir_opt_code_analysis *func_analysis = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &func));
        REQUIRE_OK(kefir_opt_module_analysis_get_function(analysis, ir_func->declaration->id, &func_analysis));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_translate_function(mem, codegen, module, func, func_analysis));
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
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64 *, codegen, cg->data);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_CLOSE(mem, &codegen->xasmgen));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_init(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                                 FILE *output, const struct kefir_codegen_configuration *config) {
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
    codegen->codegen.translate_optimized = translate_impl;
    codegen->codegen.close = close_impl;
    codegen->codegen.data = codegen;
    codegen->codegen.self = codegen;
    codegen->config = config;
    return KEFIR_OK;
}
