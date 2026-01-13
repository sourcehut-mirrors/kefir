#include "kefir/codegen/target-ir/amd64/coalesce.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/tie.h"
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
    
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instruction->instr_ref, &classification));

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification.classification.operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[i].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instruction->operation.parameters[classification.operands[i].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
            REQUIRE_OK(callback->coalesce(mem, instruction->operation.parameters[classification.operands[i].read_index].direct.value_ref, classification.operands[i].output, callback->payload));
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
