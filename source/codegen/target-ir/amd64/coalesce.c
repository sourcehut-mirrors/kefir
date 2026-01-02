#include "kefir/codegen/target-ir/amd64/coalesce.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t extract_coalesce(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_instruction *instruction,
    const struct kefir_codegen_target_ir_coalesce_callback *callback, void *payload) {
    UNUSED(mem);
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(callback != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR coalesce callback"));
    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_amd64_coalesce_class *, klass, payload);
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR amd64 coalesce class"));

    REQUIRE(instruction->operation.opcode != code->klass->phi_opcode &&
        instruction->operation.opcode != code->klass->assign_opcode &&
        instruction->operation.opcode != code->klass->upsilon_opcode &&
        instruction->operation.opcode != code->klass->inline_asm_opcode &&
        instruction->operation.opcode != code->klass->placeholder_opcode, KEFIR_OK);
    
    struct kefir_codegen_target_ir_instruction_destructor_classification classification;
    REQUIRE_OK(klass->destructor_ops->classify_instruction(code, instruction->instr_ref, &classification, klass->destructor_ops->payload));

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        kefir_size_t output_index = 0, parameter_idx = 0;
        switch (classification.operands[i].class) {
            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                parameter_idx++;
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE: {
                kefir_codegen_target_ir_value_ref_t output_value_ref;
                const struct kefir_codegen_target_ir_value_type *output_value_type;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instruction->instr_ref, output_index++, &output_value_ref, &output_value_type));
                if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                    parameter_idx++;
                }
            } break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE: {
                kefir_codegen_target_ir_value_ref_t output_value_ref;
                const struct kefir_codegen_target_ir_value_type *output_value_type;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instruction->instr_ref, output_index++, &output_value_ref, &output_value_type));
                if (instruction->operation.parameters[parameter_idx].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                    REQUIRE_OK(callback->coalesce(mem, instruction->operation.parameters[parameter_idx].direct.value_ref, output_value_ref, callback->payload));
                }
                parameter_idx++;
            } break;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_coalesce_init(struct kefir_codegen_target_ir_amd64_coalesce_class *klass,
    const struct kefir_codegen_target_ir_destructor_ops *ops) {
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR amd64 coalesce class"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR destructor ops"));

    klass->klass.extract_coalescing = extract_coalesce;
    klass->klass.payload = klass;
    klass->destructor_ops = ops;
    return KEFIR_OK;
}
