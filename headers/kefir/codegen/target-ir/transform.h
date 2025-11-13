#ifndef KEFIR_CODEGEN_TARGET_IR_TRANSFORM_H_
#define KEFIR_CODEGEN_TARGET_IR_TRANSFORM_H_

#include "kefir/codegen/target-ir/amd64/code.h"

kefir_result_t kefir_codegen_target_ir_transform_phi_removal(struct kefir_mem *, struct kefir_codegen_target_ir_code *);

#endif
