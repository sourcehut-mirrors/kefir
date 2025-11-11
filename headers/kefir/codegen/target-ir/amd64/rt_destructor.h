#ifndef KEFIR_CODEGEN_TARGET_AMD64_RT_DESTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_AMD64_RT_DESTRUCTOR_H_

#include "kefir/codegen/target-ir/rt_destructor.h"
#include "kefir/codegen/amd64/function.h"

typedef struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops {
    struct kefir_codegen_target_ir_round_trip_destructor_ops ops;
    const struct kefir_codegen_amd64_function *function;
    struct kefir_asmcmp_amd64 *code;
    struct kefir_hashtree constants;
} kefir_codegen_target_ir_round_trip_destructor_amd64_ops_t;

kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_init(const struct kefir_codegen_amd64_function *, struct kefir_asmcmp_amd64 *, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *);
kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_free(struct kefir_mem *, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *);

#endif
