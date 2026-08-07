/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/codegen/amd64/target_ir.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/amd64/constructor.h"
#include "kefir/codegen/target-ir/amd64/destructor_ops.h"
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/late_transform.h"
#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/codegen/target-ir/amd64/destructor.h"
#include "kefir/codegen/target-ir/amd64/coalesce.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/error.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_function_target_ir_init(struct kefir_mem *mem, struct kefir_codegen_amd64_function_target_ir *target_ir, kefir_abi_amd64_variant_t abi_variant) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target_ir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to codegen amd64 function target IR"));

    REQUIRE_OK(kefir_codegen_target_ir_code_init(&target_ir->code, &KEFIR_TARGET_AMD64_CODE_CLASS));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&target_ir->control_flow, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_init(&target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_interference_init(&target_ir->interference));
    REQUIRE_OK(kefir_codegen_target_ir_coalesce_init(&target_ir->coalesce));
    REQUIRE_OK(kefir_codegen_target_ir_amd64_regalloc_class_init(mem, &target_ir->regalloc_class,
                                                                 abi_variant));
    REQUIRE_OK(kefir_codegen_target_ir_regalloc_init(&target_ir->regalloc, &target_ir->regalloc_class.klass));
    REQUIRE_OK(kefir_codegen_target_ir_code_constructor_metadata_init(&target_ir->target_ir_metadata));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_target_ir_free(struct kefir_mem *mem, struct kefir_codegen_amd64_function_target_ir *target_ir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target_ir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function target IR"));

    REQUIRE_OK(kefir_codegen_target_ir_code_constructor_metadata_free(mem, &target_ir->target_ir_metadata));
    REQUIRE_OK(kefir_codegen_target_ir_regalloc_free(mem, &target_ir->regalloc));
    REQUIRE_OK(kefir_codegen_target_ir_coalesce_free(mem, &target_ir->coalesce));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_free(mem, &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_interference_free(mem, &target_ir->interference));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_free(mem, &target_ir->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_code_free(mem, &target_ir->code));
    return KEFIR_OK;
}

struct target_ir_constructor_ops_payload {
    const struct kefir_asmcmp_amd64 *code;
    const struct kefir_codegen_amd64_function_translator_state *translator;
};

static kefir_result_t construct_target_ir_get_allocation_constraint(
    kefir_asmcmp_virtual_register_index_t vreg_idx, struct kefir_codegen_target_ir_allocation_constraint *constraint,
    void *payload) {
    REQUIRE(constraint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR allocation constraint"));
    ASSIGN_DECL_CAST(struct target_ir_constructor_ops_payload *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 target IR constructor payload"));

    const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
    REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(param->code, vreg_idx, &preallocation));
    REQUIRE(preallocation != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find preallocation constraint for the virtual register"));

    switch (preallocation->type) {
        case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT:
            constraint->type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT;
            constraint->physical_register = preallocation->reg;
            break;

        case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT:
        case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS:
            constraint->type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t construct_target_ir_preallocation_match(
    kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_codegen_target_ir_physical_register_t physical_register,
    void *payload) {
    ASSIGN_DECL_CAST(struct target_ir_constructor_ops_payload *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 target IR constructor payload"));

    const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
    REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(param->code, vreg_idx, &preallocation));
    REQUIRE(preallocation != NULL, KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Preallocation does not match"));
    REQUIRE(preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT &&
                preallocation->reg == physical_register,
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Preallocation does not match"));
    return KEFIR_OK;
}

static kefir_result_t construct_target_ir_get_native_id_by_label(kefir_asmcmp_label_index_t label_idx,
                                                                 kefir_codegen_target_ir_native_id_t *native_id_ptr,
                                                                 void *payload) {
    REQUIRE(native_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR native identifier"));
    ASSIGN_DECL_CAST(struct target_ir_constructor_ops_payload *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 target IR constructor payload"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&param->translator->constants, (kefir_hashtree_key_t) label_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested target IR native identifier");
    }
    REQUIRE_OK(res);

    *native_id_ptr = node->value;
    return KEFIR_OK;
}

static kefir_result_t target_ir_use_register(struct kefir_mem *mem, kefir_codegen_target_ir_physical_register_t phreg,
                                             void *payload) {
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64_stack_frame *, stack_frame, payload);
    REQUIRE_OK(
        kefir_codegen_amd64_stack_frame_use_register(mem, stack_frame, (kefir_asm_amd64_xasmgen_register_t) phreg));
    return KEFIR_OK;
}

static kefir_result_t target_ir_use_spill_space(struct kefir_mem *mem, kefir_size_t index, kefir_size_t length,
                                                void *payload) {
    UNUSED(mem);
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64_stack_frame *, stack_frame, payload);
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(stack_frame, index + length));
    return KEFIR_OK;
}

static kefir_result_t target_ir_construct(struct kefir_mem *mem,
                                          struct kefir_codegen_amd64_function_translator_state *translator,
                                          struct kefir_codegen_amd64_function_target_ir *target_ir,
                                          struct kefir_codegen_target_ir_code_constructor_metadata *target_ir_metadata,
                                          struct kefir_asmcmp_amd64 *code) {
    struct target_ir_constructor_ops_payload ops_payload = {
        .code = code,
        .translator = translator
    };
    struct kefir_codegen_target_ir_code_constructor_ops ops = {
        .klass = &KEFIR_TARGET_AMD64_CODE_CONSTRUCTOR_CLASS,
        .get_allocation_constraint = construct_target_ir_get_allocation_constraint,
        .preallocation_match = construct_target_ir_preallocation_match,
        .get_native_id_by_label = construct_target_ir_get_native_id_by_label,
        .payload = &ops_payload};
    REQUIRE_OK(
        kefir_codegen_target_ir_code_construct(mem, &target_ir->code, &code->context, target_ir_metadata, &ops));
    
    // Minimal necessary transformation set
    REQUIRE_OK(kefir_codegen_target_ir_transform_copy_elision(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_phi_removal(mem, &target_ir->code, true));
    REQUIRE_OK(kefir_codegen_target_ir_transform_jump_propagate(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_block_merge(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_placeholder_sink(mem, &target_ir->code));

    return KEFIR_OK;
}

static kefir_result_t target_ir_destruct(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct kefir_codegen_amd64_function_translator_state *translator,
                                          struct kefir_codegen_amd64_function_target_ir *target_ir,
                                          struct kefir_codegen_target_ir_code_constructor_metadata *target_ir_metadata,
                                          struct kefir_asmcmp_amd64 *code,
                                          struct kefir_codegen_target_ir_destructor_amd64_ops *destructor_ops,
                                            struct kefir_codegen_target_ir_stack_frame *stack_frame) {
    REQUIRE_OK(kefir_asmcmp_amd64_reset_code(mem, code));
    REQUIRE_OK(kefir_hashtree_clean(mem, &translator->constants));
    REQUIRE_OK(kefir_codegen_target_ir_amd64_destruct(
                      mem, &target_ir->code, code, stack_frame, &target_ir->control_flow, &target_ir->liveness,
                      &target_ir->interference, &target_ir->regalloc,
                      codegen->config->debug_info ? target_ir_metadata : NULL, &destructor_ops->ops));

    REQUIRE_OK(kefir_codegen_target_ir_coalesce_reset(mem, &target_ir->coalesce));
    REQUIRE_OK(kefir_codegen_target_ir_interference_reset(mem, &target_ir->interference));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_reset(mem, &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_reset(mem, &target_ir->control_flow));

    return KEFIR_OK;
}

static kefir_result_t post_regalloc_opts(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct kefir_codegen_amd64_function_target_ir *target_ir,
                                          struct kefir_codegen_target_ir_amd64_coalesce_class *coalesce_class,
                                            struct kefir_codegen_target_ir_stack_frame *stack_frame,
                                        struct kefir_codegen_amd64_stack_frame *amd64_stack_frame) {
    REQUIRE_OK(kefir_codegen_target_ir_transform_remove_upsilons(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_insert_local_hot_copy(
                            mem, &target_ir->code, &target_ir->liveness, &target_ir->interference,
                            &target_ir->regalloc));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(mem, &target_ir->control_flow,
                                                                &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_rematerialize(
                            mem, &target_ir->code, &target_ir->control_flow, &target_ir->liveness,
                            &target_ir->regalloc));
    REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_dead_code_elimination(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_insert_upsilons(mem, &target_ir->code));

    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(mem, &target_ir->control_flow,
                                                                &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_interference_build(mem, &target_ir->interference,
                                                            &target_ir->control_flow, &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_coalesce_build(mem, &target_ir->coalesce, &target_ir->control_flow,
                                                        &target_ir->interference, &coalesce_class->klass));
    REQUIRE_OK(kefir_codegen_target_ir_regalloc_reset(mem, &target_ir->regalloc));
    REQUIRE_OK(kefir_codegen_target_ir_regalloc_run(mem, &target_ir->regalloc, &target_ir->control_flow,
                                                    &target_ir->liveness, &target_ir->interference,
                                                    &target_ir->coalesce, stack_frame));

    kefir_bool_t is_fused_epilogue;
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_fused_epilogue(codegen->abi_variant, amd64_stack_frame,
                                                                        &is_fused_epilogue));
    if (is_fused_epilogue) {
        REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_late_split_epilogue(mem, &target_ir->code));
        REQUIRE_OK(kefir_codegen_target_ir_liveness_resize(mem, &target_ir->liveness));
    }
    REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_late_jump_propagation(mem, &target_ir->code, &target_ir->regalloc));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_reset(mem, &target_ir->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, &target_ir->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_reset(mem, &target_ir->liveness.numbering));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_build(mem, &target_ir->liveness.numbering, &target_ir->code));

    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_target_ir_apply_impl(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct kefir_codegen_amd64_function_translator_state *translator,
                                          struct kefir_codegen_amd64_function_target_ir *target_ir,
                                          struct kefir_codegen_target_ir_code_constructor_metadata *target_ir_metadata,
                                          struct kefir_asmcmp_amd64 *code,
                                          struct kefir_codegen_target_ir_destructor_amd64_ops *destructor_ops,
                                            struct kefir_codegen_amd64_stack_frame *amd64_stack_frame) {
    REQUIRE_OK(target_ir_construct(mem, translator, target_ir, target_ir_metadata, code));

    if (codegen->config->optimization == KEFIR_CODEGEN_OPTIMIZATION_FULL) {
        REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_dead_code_elimination(mem, &target_ir->code));
        REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_peephole(mem, &target_ir->code));
        REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_dead_code_elimination(mem, &target_ir->code));
    }

    REQUIRE_OK(kefir_codegen_target_ir_transform_split_critical_edges(mem, &target_ir->code));
    REQUIRE_OK(kefir_codegen_target_ir_transform_preserve_virtual_regs(
        mem, &target_ir->code, KEFIR_TARGET_IR_AMD64_OPCODE(preserve_active_virtual_registers)));
    REQUIRE_OK(kefir_codegen_target_ir_transform_insert_upsilons(mem, &target_ir->code));

    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, &target_ir->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(mem, &target_ir->control_flow, &target_ir->liveness));
    REQUIRE_OK(kefir_codegen_target_ir_interference_build(mem, &target_ir->interference,
                                                          &target_ir->control_flow, &target_ir->liveness));

    struct kefir_codegen_target_ir_amd64_coalesce_class coalesce_class;
    REQUIRE_OK(kefir_codegen_target_ir_amd64_coalesce_init(&coalesce_class, &destructor_ops->ops));
    REQUIRE_OK(kefir_codegen_target_ir_coalesce_build(mem, &target_ir->coalesce, &target_ir->control_flow,
                                                         &target_ir->interference, &coalesce_class.klass));

    struct kefir_codegen_target_ir_stack_frame stack_frame = {.use_register = target_ir_use_register,
                                                              .use_spill_space = target_ir_use_spill_space,
                                                              .payload = amd64_stack_frame};
    REQUIRE_OK(kefir_codegen_target_ir_regalloc_run(
                            mem, &target_ir->regalloc, &target_ir->control_flow, &target_ir->liveness,
                            &target_ir->interference, &target_ir->coalesce, &stack_frame));

    if (codegen->config->optimization == KEFIR_CODEGEN_OPTIMIZATION_FULL) {
        REQUIRE_OK(post_regalloc_opts(mem, codegen, target_ir, &coalesce_class, &stack_frame, amd64_stack_frame));
    }

    REQUIRE_OK(target_ir_destruct(mem, codegen, translator, target_ir, target_ir_metadata, code, destructor_ops, &stack_frame));

    return KEFIR_OK;
}


kefir_result_t kefir_codegen_amd64_function_target_ir_apply(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct kefir_codegen_amd64_function_translator_state *translator,
                                          struct kefir_codegen_amd64_function_target_ir *target_ir,
                                          struct kefir_codegen_target_ir_code_constructor_metadata *target_ir_metadata,
                                          struct kefir_asmcmp_amd64 *code,
                                            struct kefir_codegen_amd64_stack_frame *amd64_stack_frame,
                                            const struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator"));
    REQUIRE(translator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function translator"));
    REQUIRE(target_ir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function target IR"));
    REQUIRE(target_ir_metadata != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function target IR metadata"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp code"));
    REQUIRE(amd64_stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code debug info"));

    struct kefir_codegen_target_ir_destructor_amd64_ops destructor_ops;
    REQUIRE_OK(kefir_codegen_target_ir_destructor_amd64_ops_init(debug_info, code, &translator->constants, &destructor_ops));
    
    kefir_result_t res = kefir_codegen_amd64_function_target_ir_apply_impl(mem, codegen, translator, target_ir, target_ir_metadata, code, &destructor_ops, amd64_stack_frame);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_destructor_amd64_ops_free(mem, &destructor_ops);
        return res;
    });
    REQUIRE_OK(kefir_codegen_target_ir_destructor_amd64_ops_free(mem, &destructor_ops));
    return KEFIR_OK;
}