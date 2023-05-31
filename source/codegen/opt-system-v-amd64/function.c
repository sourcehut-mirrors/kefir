#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t generate_code(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                             struct kefir_opt_module *module, const struct kefir_opt_function *function,
                             const struct kefir_opt_code_analysis *func_analysis,
                             struct kefir_codegen_opt_sysv_amd64_register_allocator *register_allocator) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func_analysis);
    UNUSED(register_allocator);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", function->ir_func->name));
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(register_allocator, instr_props->instr_ref,
                                                                       &reg_allocation));

        switch (reg_allocation->klass) {
            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE:
                if (!reg_allocation->spilled) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - GENERAL PURPOSE REGISTER %s", instr_idx,
                        kefir_asm_amd64_xasmgen_register_symbolic_name(
                            KefirOptSysvAmd64GeneralPurposeRegisters[reg_allocation->index])));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%zu - STORAGE %zu", instr_idx,
                                                           reg_allocation->index));
                }
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT:
                if (!reg_allocation->spilled) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - FLOATING POINT REGISTER %s", instr_idx,
                        kefir_asm_amd64_xasmgen_register_symbolic_name(
                            KefirOptSysvAmd64FloatingPointRegisters[reg_allocation->index])));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%zu - STORAGE %zu", instr_idx,
                                                           reg_allocation->index));
                }
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%zu - NONE", instr_idx));
                break;
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_RET(&codegen->xasmgen));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *mem,
                                                               struct kefir_codegen_opt_amd64 *codegen,
                                                               struct kefir_opt_module *module,
                                                               const struct kefir_opt_function *function,
                                                               const struct kefir_opt_code_analysis *func_analysis) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func_analysis);

    struct kefir_opt_sysv_amd64_function function_translation_data;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation(mem, function, func_analysis,
                                                                &function_translation_data.register_allocator));
    kefir_result_t res = kefir_abi_amd64_sysv_function_decl_alloc(mem, function->ir_func->declaration,
                                                                  &function_translation_data.declaration);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &function_translation_data.register_allocator);
        return res;
    });

    res = generate_code(mem, codegen, module, function, func_analysis, &function_translation_data.register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &function_translation_data.declaration);
        kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &function_translation_data.register_allocator);
        return res;
    });

    res = kefir_abi_amd64_sysv_function_decl_free(mem, &function_translation_data.declaration);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &function_translation_data.register_allocator);
        return res;
    });
    REQUIRE_OK(
        kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &function_translation_data.register_allocator));
    return KEFIR_OK;
}
