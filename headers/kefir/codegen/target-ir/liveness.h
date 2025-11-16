#ifndef KEFIR_CODEGEN_TARGET_IR_LIVENESS_H_
#define KEFIR_CODEGEN_TARGET_IR_LIVENESS_H_

#include "kefir/codegen/target-ir/control_flow.h"

typedef struct kefir_codegen_target_ir_liveness_entry {
    kefir_codegen_target_ir_instruction_ref_t *content;
    kefir_size_t length;
    kefir_size_t capacity;
} kefir_codegen_target_ir_liveness_entry_t;

typedef struct kefir_codegen_target_ir_block_liveness {
    struct kefir_codegen_target_ir_liveness_entry live_in;
    struct kefir_codegen_target_ir_liveness_entry live_out;
} kefir_codegen_target_ir_block_liveness_t;

typedef struct kefir_codegen_target_ir_liveness {
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_codegen_target_ir_block_liveness *blocks;
} kefir_codegen_target_ir_liveness_t;

kefir_result_t kefir_codegen_target_ir_liveness_init(struct kefir_codegen_target_ir_liveness *);
kefir_result_t kefir_codegen_target_ir_liveness_free(struct kefir_mem *, struct kefir_codegen_target_ir_liveness *);

kefir_result_t kefir_codegen_target_ir_liveness_build(struct kefir_mem *, const struct kefir_codegen_target_ir_control_flow *, struct kefir_codegen_target_ir_liveness *);

#endif
