#ifndef KEFIR_CODEGEN_TARGET_IR_CONTROL_FLOW_H_
#define KEFIR_CODEGEN_TARGET_IR_CONTROL_FLOW_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/core/hashtreeset.h"
#include "kefir/core/hashset.h"

typedef struct kefir_codegen_target_ir_block_control_flow {
    struct kefir_hashtreeset predecessors;
    struct kefir_hashtreeset successors;
    kefir_codegen_target_ir_block_ref_t immediate_dominator;
    struct kefir_hashset dominance_frontier;
} kefir_codegen_target_ir_block_control_flow_t;

typedef struct kefir_codegen_target_ir_control_flow {
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_codegen_target_ir_block_control_flow *blocks;
    struct kefir_hashtreeset indirect_jump_sources;
    struct kefir_hashtreeset indirect_jump_targets;
    struct kefir_hashtable dominator_tree;
} kefir_codegen_target_ir_control_flow_t;

kefir_result_t kefir_codegen_target_ir_control_flow_init(struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_code *);
kefir_result_t kefir_codegen_target_ir_control_flow_build(struct kefir_mem *, struct kefir_codegen_target_ir_control_flow *);
kefir_result_t kefir_codegen_target_ir_control_flow_free(struct kefir_mem *, struct kefir_codegen_target_ir_control_flow *);
kefir_result_t kefir_codegen_target_ir_control_flow_find_dominators(struct kefir_mem *mem,
                                                        struct kefir_codegen_target_ir_control_flow *);

kefir_result_t kefir_codegen_target_ir_control_flow_is_dominator(const struct kefir_codegen_target_ir_control_flow *,
                                                     kefir_codegen_target_ir_block_ref_t,
                                                     kefir_codegen_target_ir_block_ref_t, kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_control_flow_find_closest_common_dominator(const struct kefir_codegen_target_ir_control_flow *,
                                                       kefir_codegen_target_ir_block_ref_t,
                                                       kefir_codegen_target_ir_block_ref_t,
                                                       kefir_codegen_target_ir_block_ref_t *);

typedef struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator {
    struct kefir_hashset_iterator iter;
} kefir_codegen_target_ir_control_flow_dominator_tree_iterator_t;

kefir_result_t kefir_codegen_target_ir_control_flow_dominator_tree_iter(const struct kefir_codegen_target_ir_control_flow *, struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *, kefir_codegen_target_ir_block_ref_t, kefir_codegen_target_ir_block_ref_t *);
kefir_result_t kefir_codegen_target_ir_control_flow_dominator_tree_next(struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator *, kefir_codegen_target_ir_block_ref_t *);

#endif
