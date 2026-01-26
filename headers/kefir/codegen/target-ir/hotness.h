#ifndef KEFIR_CODEGEN_TARGET_IR_HOTNESS_H_
#define KEFIR_CODEGEN_TARGET_IR_HOTNESS_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"

#define KEFIR_CODEGEN_TARGET_IR_HOTNESS_MAX \
    ((struct kefir_codegen_target_ir_value_hotness_fragment) {.uses = (kefir_uint32_t) ~0ull, .fragment_length = 0})
#define KEFIR_CODEGEN_TARGET_IR_HOTNESS_IS_MAX(_fragment) \
    ((_fragment)->uses == (kefir_uint32_t) ~0ull && (_fragment)->fragment_length == 0)

typedef struct kefir_codegen_target_ir_value_hotness_fragment {
    union {
        kefir_uint64_t hotness;
        struct {
            kefir_uint32_t uses;
            kefir_uint32_t fragment_length;
        };
    };
} kefir_codegen_target_ir_value_hotness_fragment_t;

_Static_assert(sizeof(struct kefir_codegen_target_ir_value_hotness_fragment) == sizeof(kefir_uint64_t),
               "Unexpected size of target IR value hotness fragment");

typedef struct kefir_codegen_target_ir_hotness {
    struct kefir_hashtable global_hotness;
} kefir_codegen_target_ir_hotness_t;

kefir_int_t kefir_codegen_target_ir_value_hotness_compare(struct kefir_codegen_target_ir_value_hotness_fragment,
                                                          struct kefir_codegen_target_ir_value_hotness_fragment);

kefir_result_t kefir_codegen_target_ir_hotness_init(struct kefir_codegen_target_ir_hotness *);
kefir_result_t kefir_codegen_target_ir_hotness_free(struct kefir_mem *, struct kefir_codegen_target_ir_hotness *);

kefir_result_t kefir_codegen_target_ir_hotness_build(struct kefir_mem *, struct kefir_codegen_target_ir_hotness *,
                                                     const struct kefir_codegen_target_ir_control_flow *,
                                                     const struct kefir_codegen_target_ir_liveness *);

kefir_result_t kefir_codegen_target_ir_hotness_get_global(const struct kefir_codegen_target_ir_hotness *,
                                                          kefir_codegen_target_ir_value_ref_t,
                                                          struct kefir_codegen_target_ir_value_hotness_fragment *);

#endif
