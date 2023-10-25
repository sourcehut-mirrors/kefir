#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_const)(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_function *function,
                                                               const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_instruction_index_t instr_index;
    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE,
                                                 &result_vreg));

    if (instruction->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
        instruction->operation.parameters.imm.integer <= KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.imm.integer), &instr_index));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movabs(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.imm.integer), &instr_index));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_register_instruction(mem, function, instruction->id, instr_index));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_const)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_instruction_index_t instr_index;
    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE,
                                                 &result_vreg));

    if (instruction->operation.parameters.imm.uinteger <= (kefir_uint64_t) KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.imm.uinteger), &instr_index));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movabs(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.imm.uinteger), &instr_index));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_register_instruction(mem, function, instruction->id, instr_index));
    return KEFIR_OK;
}
