#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(return)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    if (instruction->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        kefir_asmcmp_virtual_register_index_t arg_vreg;
        REQUIRE_OK(
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

        const struct kefir_abi_amd64_function_parameters *function_returns;
        REQUIRE_OK(kefir_abi_amd64_function_decl_returns(&function->abi_function_declaration, &function_returns));

        struct kefir_abi_amd64_function_parameter function_return;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at(function_returns, 0, &function_return));

        kefir_asmcmp_virtual_register_index_t vreg;
        switch (function_return.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                // Intentionally left blank
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE, &vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_preallocate_register(
                    mem, &function->code, vreg, KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT,
                    function_return.direct_reg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg, arg_vreg,
                    NULL));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED,
                                       "Non-integral function prameters have not been implemented yet");
        }
    }

    REQUIRE_OK(
        kefir_asmcmp_amd64_ret(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    return KEFIR_OK;
}
