#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(get_argument)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const kefir_size_t param_type_index = kefir_ir_type_child_index(function->function->ir_func->declaration->params,
                                                                    instruction->operation.parameters.index);
    kefir_size_t param_type_slot;
    REQUIRE_OK(
        kefir_ir_type_slot_of(function->function->ir_func->declaration->params, param_type_index, &param_type_slot));

    const struct kefir_abi_amd64_function_parameters *function_parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&function->abi_function_declaration, &function_parameters));

    struct kefir_abi_amd64_function_parameter function_parameter;
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(function_parameters, param_type_slot, &function_parameter));

    kefir_asmcmp_virtual_register_index_t vreg;
    switch (function_parameter.location) {
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
            // Intentionally left blank
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_preallocate_register(mem, &function->code, vreg,
                                                               KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT,
                                                               function_parameter.direct_reg));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
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

    return KEFIR_OK;
}
