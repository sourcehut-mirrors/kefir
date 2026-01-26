#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_COALESCE_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_COALESCE_H_

#include "kefir/codegen/target-ir/coalesce.h"
#include "kefir/codegen/target-ir/amd64/destructor_ops.h"

typedef struct kefir_codegen_target_ir_amd64_coalesce_class {
    struct kefir_codegen_target_ir_coalesce_class klass;
    const struct kefir_codegen_target_ir_destructor_ops *destructor_ops;
} kefir_codegen_target_ir_amd64_coalesce_class_t;

kefir_result_t kefir_codegen_target_ir_amd64_coalesce_init(struct kefir_codegen_target_ir_amd64_coalesce_class *,
                                                           const struct kefir_codegen_target_ir_destructor_ops *);

#endif
