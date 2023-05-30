#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t generate_code(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                             struct kefir_opt_module *module, const struct kefir_opt_function *function,
                             const struct kefir_opt_code_analysis *func_analysis,
                             struct kefir_codegen_opt_linear_register_allocator *register_allocator) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func_analysis);
    UNUSED(register_allocator);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", function->ir_func->name));
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];
        const struct kefir_codegen_opt_linear_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_allocation_of(register_allocator, instr_props->instr_ref,
                                                                             &reg_allocation));

        switch (reg_allocation->constraint.type) {
            case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE:
                if (reg_allocation->allocation < KefirOptSysvAmd64NumOfGeneralPurposeRegisters) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - GENERAL PURPOSE REGISTER %s", instr_idx,
                        kefir_asm_amd64_xasmgen_register_symbolic_name(
                            KefirOptSysvAmd64GeneralPurposeRegisters[reg_allocation->allocation])));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - GENERAL PURPOSE STORAGE %zu", instr_idx,
                        reg_allocation->allocation - KefirOptSysvAmd64NumOfGeneralPurposeRegisters));
                }
                break;

            case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT:
                if (reg_allocation->allocation < KefirOptSysvAmd64NumOfFloatingPointRegisters) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - FLOATING POINT REGISTER %s", instr_idx,
                        kefir_asm_amd64_xasmgen_register_symbolic_name(
                            KefirOptSysvAmd64FloatingPointRegisters[reg_allocation->allocation])));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                        &codegen->xasmgen, "%zu - FLOATING POINT STORAGE %zu", instr_idx,
                        reg_allocation->allocation - KefirOptSysvAmd64NumOfFloatingPointRegisters));
                }
                break;

            case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION:
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
    struct kefir_codegen_opt_linear_register_allocator register_allocator;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation(mem, function, func_analysis, &register_allocator));

    kefir_result_t res = generate_code(mem, codegen, module, function, func_analysis, &register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_linear_register_allocator_free(mem, &register_allocator);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_free(mem, &register_allocator));
    return KEFIR_OK;
}
