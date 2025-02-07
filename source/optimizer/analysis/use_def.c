#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_block_is_dominator(const struct kefir_opt_code_analysis *analysis,
                                                 kefir_opt_block_id_t dominated_block,
                                                 kefir_opt_block_id_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (analysis->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_block_is_dominator(analysis, analysis->blocks[dominated_block].immediate_dominator,
                                                     dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_control_flow(const struct kefir_opt_code_container *code,
                                                          kefir_opt_instruction_ref_t instr_ref,
                                                          kefir_bool_t *result_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, instr->block_id, &block));

    *result_ptr = block->control_flow.head == instr_ref || instr->control_flow.prev != KEFIR_ID_NONE ||
                  instr->control_flow.next != KEFIR_ID_NONE;
    return KEFIR_OK;
}

struct is_locally_sequenced_before_param {
    const struct kefir_opt_code_analysis *analysis;
    kefir_opt_instruction_ref_t instr_ref;
    kefir_bool_t *result;
};

static kefir_result_t is_locally_sequenced_before_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct is_locally_sequenced_before_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction sequence parameter"));

    if (*param->result) {
        kefir_bool_t result = false;
        REQUIRE_OK(
            kefir_opt_code_instruction_is_sequenced_before(param->analysis, instr_ref, param->instr_ref, &result));
        *param->result = *param->result && result;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_locally_sequenced_before(const struct kefir_opt_code_analysis *analysis,
                                                                      kefir_opt_instruction_ref_t instr_ref1,
                                                                      kefir_opt_instruction_ref_t instr_ref2,
                                                                      kefir_bool_t *result_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref2, &instr2));
    REQUIRE(instr1->block_id == instr2->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instructions belong to different optimizer code blocks"));

    kefir_bool_t result = true;

    kefir_bool_t instr1_control_flow, instr2_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(analysis->code, instr_ref1, &instr1_control_flow));
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(analysis->code, instr_ref2, &instr2_control_flow));

    if (instr1_control_flow && instr2_control_flow) {
        kefir_bool_t found = false;
        for (kefir_opt_instruction_ref_t iter = instr_ref2; !found && iter != KEFIR_ID_NONE;) {
            if (iter == instr_ref1) {
                found = true;
            } else {
                REQUIRE_OK(kefir_opt_instruction_prev_control(analysis->code, iter, &iter));
            }
        }
        if (!found) {
            result = false;
        }
    }

    if (result && instr2_control_flow) {
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(
            analysis->code, instr1, true, is_locally_sequenced_before_impl,
            &(struct is_locally_sequenced_before_param) {
                .analysis = analysis, .instr_ref = instr_ref2, .result = &result}));
    }
    *result_ptr = result;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_sequenced_before(const struct kefir_opt_code_analysis *analysis,
                                                              kefir_opt_instruction_ref_t instr_ref1,
                                                              kefir_opt_instruction_ref_t instr_ref2,
                                                              kefir_bool_t *result_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(analysis->code, instr_ref2, &instr2));

    if (instr1->block_id == instr2->block_id) {
        REQUIRE_OK(
            kefir_opt_code_instruction_is_locally_sequenced_before(analysis, instr_ref1, instr_ref2, result_ptr));
    } else {
        REQUIRE_OK(kefir_opt_code_block_is_dominator(analysis, instr2->block_id, instr1->block_id, result_ptr));
    }
    return KEFIR_OK;
}

struct verify_use_def_payload {
    struct kefir_mem *mem;
    struct kefir_opt_code_analysis *analysis;
    kefir_opt_instruction_ref_t instr_ref;
};

static kefir_result_t verify_use_def_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    kefir_bool_t sequenced_before;
    REQUIRE_OK(kefir_opt_code_instruction_is_sequenced_before(param->analysis, instr_ref, param->instr_ref,
                                                              &sequenced_before));
    REQUIRE(sequenced_before, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Reversed use-define chain in optimizer code"));

    return KEFIR_OK;
}

static kefir_result_t verify_use_def(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->analysis->code, instr_ref, &instr));

    param->instr_ref = instr_ref;
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->analysis->code, instr, true, verify_use_def_impl, payload));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_verify_use_def(struct kefir_mem *mem,
                                                       struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    struct verify_use_def_payload payload = {.mem = mem, .analysis = analysis, .instr_ref = KEFIR_ID_NONE};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = verify_use_def, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_container_trace(mem, analysis->code, &tracer));
    return KEFIR_OK;
}
