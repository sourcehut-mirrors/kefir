#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/codegen/amd64-common.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t translate_impl(struct kefir_mem *mem, struct kefir_codegen *cg, struct kefir_opt_module *module,
                                     struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64 *, codegen, cg->data);

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {
        const struct kefir_opt_function *func = NULL;
        const struct kefir_opt_code_analysis *func_analysis = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &func));
        REQUIRE_OK(kefir_opt_module_analysis_get_function(analysis, ir_func->declaration->id, &func_analysis));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_translate_function(mem, codegen, module, func, func_analysis));
    }

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
