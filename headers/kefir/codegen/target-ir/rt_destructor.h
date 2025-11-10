#ifndef KEFIR_CODEGEN_TARGET_IR_RT_DESTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_IR_RT_DESTRUCTOR_H_

#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/codegen/asmcmp/context.h"

typedef struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification {
    kefir_asmcmp_instruction_opcode_t opcode;
    union {
        struct {
            struct kefir_codegen_target_ir_asmcmp_operand_classification operands[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
        };
    };
} kefir_codegen_target_ir_target_ir_instruction_destructor_classification_t;

typedef struct kefir_codegen_target_ir_round_trip_destructor_parameter {
    kefir_result_t (*touch_virtual_register)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_asmcmp_virtual_register_index_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*link_virtual_register)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_asmcmp_virtual_register_index_t, kefir_asmcmp_virtual_register_index_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*classify_instruction)(const struct kefir_codegen_target_ir_instruction *, struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification *, void *);
    void *payload;
} kefir_codegen_target_ir_round_trip_destructor_parameter_t;

kefir_result_t kefir_codegen_target_ir_round_trip_destruct(struct kefir_mem *, const struct kefir_codegen_target_ir_code *, struct kefir_asmcmp_context *, const struct kefir_codegen_target_ir_round_trip_destructor_parameter *);

#endif
