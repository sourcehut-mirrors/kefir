#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[] = {
    // Caller-saved
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RCX, KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI, KEFIR_AMD64_XASMGEN_REGISTER_R8,
    KEFIR_AMD64_XASMGEN_REGISTER_R9, KEFIR_AMD64_XASMGEN_REGISTER_R10, KEFIR_AMD64_XASMGEN_REGISTER_R11,
    // Callee-saved
    KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters =
    sizeof(KefirOptSysvAmd64GeneralPurposeRegisters) / sizeof(kefir_asm_amd64_xasmgen_register_t);
const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters =
    sizeof(KefirOptSysvAmd64FloatingPointRegisters) / sizeof(kefir_asm_amd64_xasmgen_register_t);

struct hint_input_output_coalesce_param {
    const struct kefir_opt_function *function;
    const struct kefir_opt_code_analysis *func_analysis;
    struct kefir_opt_code_analysis_instruction_properties *instr_props;
    struct kefir_codegen_opt_linear_register_allocator *allocator;
};

static kefir_result_t hint_input_output_coalesce(kefir_opt_instruction_ref_t input_ref, void *payload) {
    ASSIGN_DECL_CAST(struct hint_input_output_coalesce_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                           "Expected valid System-V AMD64 register allocator hint parameter"));

    struct kefir_opt_instruction_liveness_interval *instr_liveness =
        &param->func_analysis->liveness.intervals[param->instr_props->linear_position];
    struct kefir_opt_code_analysis_instruction_properties *input_instr_props =
        &param->func_analysis->instructions[input_ref];
    struct kefir_opt_instruction_liveness_interval *input_instr_liveness =
        &param->func_analysis->liveness.intervals[input_instr_props->linear_position];
    REQUIRE(input_instr_liveness->range.end <= instr_liveness->range.end + 1, KEFIR_OK);

    const struct kefir_codegen_opt_linear_register_allocation *instr_allocation = NULL;
    const struct kefir_codegen_opt_linear_register_allocation *input_allocation = NULL;
    REQUIRE_OK(
        kefir_codegen_opt_linear_register_allocator_allocation_of(param->allocator, input_ref, &input_allocation));
    REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_allocation_of(
        param->allocator, param->instr_props->instr_ref, &instr_allocation));
    REQUIRE(input_allocation->constraint.type != KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION,
            KEFIR_OK);
    REQUIRE(instr_allocation->constraint.type == input_allocation->constraint.type, KEFIR_OK);

    REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_hint_alias(param->allocator, param->instr_props->instr_ref,
                                                                      input_ref));
    return KEFIR_YIELD;
}

static kefir_result_t hint_phi_coalescing(const struct kefir_opt_function *function,
                                          const struct kefir_opt_instruction *instr,
                                          struct kefir_opt_code_analysis_instruction_properties *instr_props,
                                          const struct kefir_opt_code_analysis *func_analysis,
                                          struct kefir_codegen_opt_linear_register_allocator *allocator) {
    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(&function->code, instr->operation.parameters.phi_ref, &phi));

    struct kefir_opt_phi_node_link_iterator iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    kefir_result_t res;
    for (res = kefir_opt_phi_node_link_iter(phi, &iter, &link_block_id, &link_instr_ref); res == KEFIR_OK;
         res = kefir_opt_phi_node_link_next(&iter, &link_block_id, &link_instr_ref)) {
        struct kefir_opt_code_analysis_instruction_properties *link_instr_props =
            &func_analysis->instructions[link_instr_ref];
        if (link_instr_props->linear_position > instr_props->linear_position) {
            continue;
        }

        const struct kefir_codegen_opt_linear_register_allocation *link_allocation = NULL;
        REQUIRE_OK(
            kefir_codegen_opt_linear_register_allocator_allocation_of(allocator, link_instr_ref, &link_allocation));

        if (link_allocation->constraint.type !=
            KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION) {
            REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_set_type(allocator, instr_props->instr_ref,
                                                                            link_allocation->constraint.type));
            REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_hint_alias(allocator, instr_props->instr_ref,
                                                                              link_instr_ref));
            break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t initialize_allocator(const struct kefir_opt_function *function,
                                           const struct kefir_opt_code_analysis *func_analysis,
                                           struct kefir_codegen_opt_linear_register_allocator *allocator) {
    kefir_result_t res;
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        struct kefir_opt_code_analysis_instruction_properties *instr_props = func_analysis->linearization[instr_idx];
        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

        kefir_bool_t coalesce_input_output = true;
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_PHI:
                REQUIRE_OK(hint_phi_coalescing(function, instr, instr_props, func_analysis, allocator));
                coalesce_input_output = false;
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY:
                return KEFIR_SET_ERROR(
                    KEFIR_NOT_IMPLEMENTED,
                    "Inlint assembly support is not implemented in the System-V AMD64 optimized code generator yet");

            case KEFIR_OPT_OPCODE_JUMP:
            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_RETURN:
            case KEFIR_OPT_OPCODE_VARARG_START:
            case KEFIR_OPT_OPCODE_VARARG_COPY:
            case KEFIR_OPT_OPCODE_VARARG_END:
            case KEFIR_OPT_OPCODE_SCOPE_POP:
                REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_set_type(
                    allocator, instr_props->instr_ref,
                    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION));
                coalesce_input_output = false;
                break;

            case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            case KEFIR_OPT_OPCODE_FLOAT32_ADD:
            case KEFIR_OPT_OPCODE_FLOAT32_SUB:
            case KEFIR_OPT_OPCODE_FLOAT32_MUL:
            case KEFIR_OPT_OPCODE_FLOAT32_DIV:
            case KEFIR_OPT_OPCODE_FLOAT32_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_UINT_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            case KEFIR_OPT_OPCODE_FLOAT64_ADD:
            case KEFIR_OPT_OPCODE_FLOAT64_SUB:
            case KEFIR_OPT_OPCODE_FLOAT64_MUL:
            case KEFIR_OPT_OPCODE_FLOAT64_DIV:
            case KEFIR_OPT_OPCODE_FLOAT64_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_UINT_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64:
                REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_set_type(
                    allocator, instr_props->instr_ref,
                    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT));
                break;

            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE:
                return KEFIR_SET_ERROR(
                    KEFIR_NOT_IMPLEMENTED,
                    "Long double support is not implemented in the System-V AMD64 optimized code generator yet");

            default:
                // Intentionally left blank
                break;
        }

        if (coalesce_input_output) {
            struct hint_input_output_coalesce_param param = {.instr_props = instr_props,
                                                             .allocator = allocator,
                                                             .func_analysis = func_analysis,
                                                             .function = function};
            res = kefir_opt_instruction_extract_inputs(&function->code, instr, hint_input_output_coalesce, &param);
            if (res == KEFIR_YIELD) {
                res = KEFIR_OK;
            }
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(
    struct kefir_mem *mem, const struct kefir_opt_function *function,
    const struct kefir_opt_code_analysis *func_analysis,
    struct kefir_codegen_opt_linear_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to linear register allocator"));

    REQUIRE_OK(kefir_codegen_opt_linear_register_allocator_init(mem, allocator, func_analysis,
                                                                KefirOptSysvAmd64NumOfGeneralPurposeRegisters,
                                                                KefirOptSysvAmd64NumOfFloatingPointRegisters));
    kefir_result_t res = initialize_allocator(function, func_analysis, allocator);
    REQUIRE_CHAIN(&res, kefir_codegen_opt_linear_register_allocator_run(mem, allocator));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_linear_register_allocator_free(mem, allocator);
        return res;
    });
    return KEFIR_OK;
}
