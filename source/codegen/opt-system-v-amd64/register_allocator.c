#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/hashtreeset.h"
#include <string.h>

const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[] = {
    // Caller-saved
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RCX, KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI, KEFIR_AMD64_XASMGEN_REGISTER_R8,
    KEFIR_AMD64_XASMGEN_REGISTER_R9, KEFIR_AMD64_XASMGEN_REGISTER_R10, KEFIR_AMD64_XASMGEN_REGISTER_R11,
    // Callee-saved
    KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters =
    sizeof(KefirOptSysvAmd64GeneralPurposeRegisters) / sizeof(kefir_asm_amd64_xasmgen_register_t);
const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters =
    sizeof(KefirOptSysvAmd64FloatingPointRegisters) / sizeof(kefir_asm_amd64_xasmgen_register_t);

static kefir_result_t general_purpose_index_of(kefir_asm_amd64_xasmgen_register_t reg, kefir_size_t *index) {
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        if (KefirOptSysvAmd64GeneralPurposeRegisters[i] == reg) {
            *index = i;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to fint requested general purpose AMD64 register");
}

static kefir_result_t floating_point_index_of(kefir_asm_amd64_xasmgen_register_t reg, kefir_size_t *index) {
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfFloatingPointRegisters; i++) {
        if (KefirOptSysvAmd64FloatingPointRegisters[i] == reg) {
            *index = i;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to fint requested floating point AMD64 register");
}

static kefir_result_t build_graph_impl(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                       struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                       struct kefir_list *alive) {
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];

        struct kefir_codegen_opt_sysv_amd64_register_allocation *instr_allocation =
            KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_opt_sysv_amd64_register_allocation));
        REQUIRE(instr_allocation != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate register allocation entry"));

        instr_allocation->done = false;
        instr_allocation->klass = KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE;
        instr_allocation->spilled = false;
        instr_allocation->register_hint.present = false;
        instr_allocation->alias_hint.present = false;

        kefir_result_t res =
            kefir_graph_new_node(mem, &allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref,
                                 (kefir_graph_node_value_t) instr_allocation);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, instr_allocation);
            return res;
        });

        for (const struct kefir_list_entry *iter = kefir_list_head(alive); iter != NULL;) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);

            const struct kefir_opt_instruction_liveness_interval *liveness =
                &func_analysis->liveness.intervals[func_analysis->instructions[instr_ref].linear_position];
            if (liveness->range.end <= instr_idx) {
                const struct kefir_list_entry *next_iter = iter->next;
                REQUIRE_OK(kefir_list_pop(mem, alive, (struct kefir_list_entry *) iter));
                iter = next_iter;
            } else {
                REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->allocation, (kefir_graph_node_id_t) instr_ref,
                                                (kefir_graph_node_id_t) instr_props->instr_ref));
                REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->allocation,
                                                (kefir_graph_node_id_t) instr_props->instr_ref,
                                                (kefir_graph_node_id_t) instr_ref));
                kefir_list_next(&iter);
            }
        }

        REQUIRE_OK(kefir_list_insert_after(mem, alive, kefir_list_tail(alive),
                                           (void *) (kefir_uptr_t) instr_props->instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t build_graph(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                  struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    struct kefir_list alive;
    REQUIRE_OK(kefir_list_init(&alive));

    kefir_result_t res = build_graph_impl(mem, func_analysis, allocator, &alive);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &alive);
        return res;
    });

    REQUIRE_OK(kefir_list_free(mem, &alive));
    return KEFIR_OK;
}

static kefir_result_t hint_phi_coalescing(const struct kefir_opt_function *function,
                                          const struct kefir_opt_instruction *instr,
                                          struct kefir_opt_code_analysis_instruction_properties *instr_props,
                                          struct kefir_codegen_opt_sysv_amd64_register_allocation *instr_allocation,
                                          const struct kefir_opt_code_analysis *func_analysis,
                                          struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(&function->code, instr->operation.parameters.phi_ref, &phi));

    struct kefir_opt_phi_node_link_iterator iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    kefir_result_t res;
    for (res = kefir_opt_phi_node_link_iter(phi, &iter, &link_block_id, &link_instr_ref); res == KEFIR_OK;
         res = kefir_opt_phi_node_link_next(&iter, &link_block_id, &link_instr_ref)) {
        struct kefir_opt_code_analysis_instruction_properties *link_instr_props =
            &func_analysis->instructions[link_instr_ref];
        if (link_instr_props->linear_position > instr_props->linear_position) {
            continue;
        }

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) link_instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, link_allocation, node->value);

        if (link_allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP) {
            instr_allocation->klass = link_allocation->klass;
            instr_allocation->alias_hint.present = true;
            instr_allocation->alias_hint.instr_ref = link_instr_ref;
            break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t hint_return_coalescing(const struct kefir_opt_instruction *instr,
                                             struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    struct kefir_graph_node *node = NULL;
    if (instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr->operation.parameters.refs[0],
                                    &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, ret_allocation, node->value);

        if (!ret_allocation->register_hint.present) {
            if (ret_allocation->klass == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE) {
                REQUIRE_OK(
                    general_purpose_index_of(KEFIR_AMD64_XASMGEN_REGISTER_RAX, &ret_allocation->register_hint.index));
                ret_allocation->register_hint.present = true;
            } else if (ret_allocation->klass == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT) {
                REQUIRE_OK(
                    floating_point_index_of(KEFIR_AMD64_XASMGEN_REGISTER_XMM0, &ret_allocation->register_hint.index));
                ret_allocation->register_hint.present = true;
            }
        }
    }
    return KEFIR_OK;
}

struct hint_input_output_coalesce_param {
    const struct kefir_opt_function *function;
    const struct kefir_opt_code_analysis *func_analysis;
    struct kefir_opt_code_analysis_instruction_properties *instr_props;
    struct kefir_codegen_opt_sysv_amd64_register_allocation *instr_allocation;
    struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator;
};

static kefir_result_t hint_input_output_coalesce(kefir_opt_instruction_ref_t input_ref, void *payload) {
    ASSIGN_DECL_CAST(struct hint_input_output_coalesce_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                           "Expected valid System-V AMD64 register allocator hint parameter"));

    struct kefir_opt_instruction_liveness_interval *instr_liveness =
        &param->func_analysis->liveness.intervals[param->instr_props->linear_position];
    struct kefir_opt_code_analysis_instruction_properties *input_instr_props =
        &param->func_analysis->instructions[input_ref];
    struct kefir_opt_instruction_liveness_interval *input_instr_liveness =
        &param->func_analysis->liveness.intervals[input_instr_props->linear_position];
    REQUIRE(input_instr_liveness->range.end <= instr_liveness->range.end + 1, KEFIR_OK);

    struct kefir_graph_node *node = NULL;
    REQUIRE_OK(kefir_graph_node(&param->allocator->allocation, (kefir_graph_node_id_t) input_ref, &node));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, input_allocation, node->value);

    REQUIRE(input_allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP, KEFIR_OK);
    REQUIRE(param->instr_allocation->klass == input_allocation->klass, KEFIR_OK);

    param->instr_allocation->alias_hint.present = true;
    param->instr_allocation->alias_hint.instr_ref = input_ref;
    return KEFIR_YIELD;
}

static kefir_result_t insert_hints(const struct kefir_opt_function *function,
                                   const struct kefir_opt_code_analysis *func_analysis,
                                   struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {

    kefir_result_t res;
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        struct kefir_opt_code_analysis_instruction_properties *instr_props = func_analysis->linearization[instr_idx];
        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, allocation, node->value);

        kefir_bool_t coalesce_input_output = true;
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_PHI:
                REQUIRE_OK(hint_phi_coalescing(function, instr, instr_props, allocation, func_analysis, allocator));
                coalesce_input_output = false;
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY:
                return KEFIR_SET_ERROR(
                    KEFIR_NOT_IMPLEMENTED,
                    "Inlint assembly support is not implemented in the System-V AMD64 optimized code generator yet");

            case KEFIR_OPT_OPCODE_RETURN:
                REQUIRE_OK(hint_return_coalescing(instr, allocator));
                // Fallthrough

            case KEFIR_OPT_OPCODE_JUMP:
            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_VARARG_START:
            case KEFIR_OPT_OPCODE_VARARG_COPY:
            case KEFIR_OPT_OPCODE_VARARG_END:
            case KEFIR_OPT_OPCODE_SCOPE_POP:
                allocation->klass = KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP;
                coalesce_input_output = false;
                break;

            case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            case KEFIR_OPT_OPCODE_FLOAT32_ADD:
            case KEFIR_OPT_OPCODE_FLOAT32_SUB:
            case KEFIR_OPT_OPCODE_FLOAT32_MUL:
            case KEFIR_OPT_OPCODE_FLOAT32_DIV:
            case KEFIR_OPT_OPCODE_FLOAT32_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_UINT_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32:
            case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            case KEFIR_OPT_OPCODE_FLOAT64_ADD:
            case KEFIR_OPT_OPCODE_FLOAT64_SUB:
            case KEFIR_OPT_OPCODE_FLOAT64_MUL:
            case KEFIR_OPT_OPCODE_FLOAT64_DIV:
            case KEFIR_OPT_OPCODE_FLOAT64_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_UINT_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64:
                allocation->klass = KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT;
                break;

            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG:
            case KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE:
                return KEFIR_SET_ERROR(
                    KEFIR_NOT_IMPLEMENTED,
                    "Long double support is not implemented in the System-V AMD64 optimized code generator yet");

            default:
                // Intentionally left blank
                break;
        }

        if (coalesce_input_output) {
            struct hint_input_output_coalesce_param param = {.instr_props = instr_props,
                                                             .instr_allocation = allocation,
                                                             .allocator = allocator,
                                                             .func_analysis = func_analysis,
                                                             .function = function};
            res = kefir_opt_instruction_extract_inputs(&function->code, instr, hint_input_output_coalesce, &param);
            if (res == KEFIR_YIELD) {
                res = KEFIR_OK;
            }
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_hints(const struct kefir_opt_code_analysis *func_analysis,
                                      struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    for (kefir_size_t instr_rev_idx = 0; instr_rev_idx < func_analysis->linearization_length; instr_rev_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[func_analysis->linearization_length - instr_rev_idx - 1];

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, allocation, node->value);

        if (allocation->register_hint.present && allocation->alias_hint.present &&
            allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP) {
            const struct kefir_opt_code_analysis_instruction_properties *alias_instr_props =
                &func_analysis->instructions[allocation->alias_hint.instr_ref];
            REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                        (kefir_graph_node_id_t) allocation->alias_hint.instr_ref, &node));
            ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, alias_allocation, node->value);
            if (alias_instr_props->linear_position < instr_props->linear_position &&
                alias_allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP &&
                !alias_allocation->register_hint.present) {

                alias_allocation->register_hint.present = true;
                alias_allocation->register_hint.index = allocation->register_hint.index;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t deallocate(struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                 struct kefir_codegen_opt_sysv_amd64_register_allocation *allocation) {
    REQUIRE(allocation->done, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected register allocation entry to be filled"));

    switch (allocation->klass) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected register entry allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE:
            REQUIRE_OK(kefir_bitset_set(&allocator->general_purpose_regs, allocation->index, false));
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT:
            REQUIRE_OK(kefir_bitset_set(&allocator->floating_point_regs, allocation->index, false));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t deallocate_dead(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                      struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                      struct kefir_list *alive, kefir_size_t instr_idx) {
    struct kefir_graph_node *node = NULL;
    for (const struct kefir_list_entry *iter = kefir_list_head(alive); iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, prev_instr_ref, (kefir_uptr_t) iter->value);

        const struct kefir_opt_instruction_liveness_interval *prev_liveness =
            &func_analysis->liveness.intervals[func_analysis->instructions[prev_instr_ref].linear_position];
        const struct kefir_list_entry *next_iter = iter->next;
        if (prev_liveness->range.end <= instr_idx + 1) {
            REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) prev_instr_ref, &node));
            ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, prev_allocation, node->value);
            REQUIRE_OK(deallocate(allocator, prev_allocation));
            REQUIRE_OK(kefir_list_pop(mem, alive, (struct kefir_list_entry *) iter));
        }
        iter = next_iter;
    }
    return KEFIR_OK;
}

static kefir_result_t collect_conflict_hints(
    struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
    struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator, struct kefir_hashtreeset *conflict_hints,
    const struct kefir_opt_code_analysis_instruction_properties *instr_props,
    struct kefir_codegen_opt_sysv_amd64_register_allocation *current_allocation) {
    kefir_result_t res;
    REQUIRE_OK(kefir_hashtreeset_clean(mem, conflict_hints));
    kefir_graph_node_id_t conflict_edge;
    struct kefir_graph_edge_iterator edge_iter;
    for (res = kefir_graph_edge_iter(&allocator->allocation, &edge_iter, (kefir_graph_node_id_t) instr_props->instr_ref,
                                     &conflict_edge);
         res == KEFIR_OK; res = kefir_graph_edge_next(&edge_iter, &conflict_edge)) {

        struct kefir_opt_code_analysis_instruction_properties *conflict_instr_props =
            &func_analysis->instructions[conflict_edge];
        if (conflict_instr_props->linear_position < instr_props->linear_position) {
            continue;
        }

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) conflict_edge, &node));
        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation *, conflict_allocation,
                         node->value);

        if (conflict_allocation->klass != current_allocation->klass) {
            continue;
        }

        if (conflict_allocation->register_hint.present) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, conflict_hints,
                                             (kefir_hashtreeset_entry_t) conflict_allocation->register_hint.index));
        } else if (conflict_allocation->alias_hint.present) {
            struct kefir_graph_node *node = NULL;
            REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                        (kefir_graph_node_id_t) conflict_allocation->alias_hint.instr_ref, &node));
            ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation *, ref_allocation,
                             node->value);
            if (ref_allocation->done) {
                REQUIRE_OK(
                    kefir_hashtreeset_add(mem, conflict_hints, (kefir_hashtreeset_entry_t) ref_allocation->index));
            }
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}

static kefir_result_t allocate_vreg(struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                    struct kefir_codegen_opt_sysv_amd64_register_allocation *allocation,
                                    kefir_size_t index, kefir_bool_t *success) {
    REQUIRE(!allocation->done, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected register allocation entry to be empty"));

    kefir_bool_t reg_allocated;
    switch (allocation->klass) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE:
            REQUIRE_OK(kefir_bitset_get(&allocator->general_purpose_regs, index, &reg_allocated));
            if (!reg_allocated) {
                *success = true;
                REQUIRE_OK(kefir_bitset_set(&allocator->general_purpose_regs, index, true));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT:
            REQUIRE_OK(kefir_bitset_get(&allocator->floating_point_regs, index, &reg_allocated));
            if (!reg_allocated) {
                *success = true;
                REQUIRE_OK(kefir_bitset_set(&allocator->floating_point_regs, index, true));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected register allocation type");
    }

    if (*success) {
        allocation->done = true;
        allocation->index = index;
    }
    return KEFIR_OK;
}

static kefir_result_t attempt_hint_allocation(
    struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
    struct kefir_codegen_opt_sysv_amd64_register_allocation *current_allocation, kefir_bool_t *success) {
    *success = false;
    if (current_allocation->register_hint.present) {
        REQUIRE_OK(allocate_vreg(allocator, current_allocation, current_allocation->register_hint.index, success));

        if (*success) {
            return KEFIR_OK;
        }
    }
    if (current_allocation->alias_hint.present) {
        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                    (kefir_graph_node_id_t) current_allocation->alias_hint.instr_ref, &node));
        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation *, ref_allocation, node->value);
        if (ref_allocation->done && !ref_allocation->spilled && ref_allocation->klass == current_allocation->klass) {
            REQUIRE_OK(allocate_vreg(allocator, current_allocation, ref_allocation->index, success));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_reg(struct kefir_mem *mem,
                                   struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                   struct kefir_codegen_opt_sysv_amd64_register_allocation *current_allocation,
                                   struct kefir_hashtreeset *conflict_hints) {
    struct kefir_bitset *regs_bits = NULL;
    switch (current_allocation->klass) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE:
            regs_bits = &allocator->general_purpose_regs;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT:
            regs_bits = &allocator->floating_point_regs;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected register allocation type constraint");
    }

    // Attempt allocation to avoid conflicts with future hints
    kefir_bool_t found = false;
    kefir_size_t search_index = 0;
    while (!found) {
        kefir_size_t available_index;
        kefir_result_t res = kefir_bitset_find(regs_bits, false, search_index, &available_index);
        if (res == KEFIR_NOT_FOUND) {
            break;
        }
        REQUIRE_OK(res);

        if (!kefir_hashtreeset_has(conflict_hints, (kefir_hashtreeset_entry_t) available_index)) {
            current_allocation->done = true;
            current_allocation->index = available_index;
            current_allocation->spilled = false;
            REQUIRE_OK(kefir_bitset_set(regs_bits, available_index, true));
            found = true;
        } else {
            search_index = available_index + 1;
        }
    }

    // Attempt allocation to any available register
    if (!found) {
        kefir_size_t available_reg_index;
        kefir_result_t res = kefir_bitset_find(regs_bits, false, search_index, &available_reg_index);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            current_allocation->done = true;
            current_allocation->index = available_reg_index;
            current_allocation->spilled = false;
            REQUIRE_OK(kefir_bitset_set(regs_bits, available_reg_index, true));
            found = true;
        }
    }

    // Spill
    if (!found) {
        kefir_size_t available_spill_index;
        kefir_result_t res = kefir_bitset_find(&allocator->spilled_regs, false, search_index, &available_spill_index);
        if (res == KEFIR_NOT_FOUND) {
            kefir_size_t spill_length;
            REQUIRE_OK(kefir_bitset_length(&allocator->spilled_regs, &spill_length));
            REQUIRE_OK(kefir_bitset_resize(mem, &allocator->spilled_regs, spill_length++));
            res = kefir_bitset_find(&allocator->spilled_regs, false, search_index, &available_spill_index);
        }

        REQUIRE_OK(res);
        current_allocation->done = true;
        current_allocation->index = available_spill_index;
        current_allocation->spilled = true;
        REQUIRE_OK(kefir_bitset_set(&allocator->spilled_regs, available_spill_index, true));
        found = true;
    }
    return KEFIR_OK;
}

static kefir_result_t do_allocation_impl(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                         struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator,
                                         struct kefir_list *alive, struct kefir_hashtreeset *conflict_hints) {
    UNUSED(mem);
    UNUSED(allocator);
    UNUSED(alive);
    UNUSED(conflict_hints);
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, allocation, node->value);

        REQUIRE_OK(deallocate_dead(mem, func_analysis, allocator, alive, instr_idx));

        if (allocation->klass != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP) {
            REQUIRE_OK(collect_conflict_hints(mem, func_analysis, allocator, conflict_hints, instr_props, allocation));
            kefir_bool_t success;
            REQUIRE_OK(attempt_hint_allocation(allocator, allocation, &success));
            if (!success) {
                REQUIRE_OK(allocate_reg(mem, allocator, allocation, conflict_hints));
            }
            REQUIRE_OK(kefir_list_insert_after(mem, alive, kefir_list_tail(alive),
                                               (void *) (kefir_uptr_t) instr_props->instr_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t do_allocation(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                    struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {

    struct kefir_list alive;
    struct kefir_hashtreeset conflict_hints;
    REQUIRE_OK(kefir_list_init(&alive));
    REQUIRE_OK(kefir_hashtreeset_init(&conflict_hints, &kefir_hashtree_uint_ops));

    kefir_result_t res = do_allocation_impl(mem, func_analysis, allocator, &alive, &conflict_hints);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &alive);
        kefir_hashtreeset_free(mem, &conflict_hints);
        return res;
    });

    res = kefir_list_free(mem, &alive);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &conflict_hints);
        return res;
    });

    REQUIRE_OK(kefir_hashtreeset_free(mem, &conflict_hints));
    return KEFIR_OK;
}

static kefir_result_t register_allocation_impl(struct kefir_mem *mem, const struct kefir_opt_function *function,
                                               const struct kefir_opt_code_analysis *func_analysis,
                                               struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {

    REQUIRE_OK(build_graph(mem, func_analysis, allocator));
    REQUIRE_OK(insert_hints(function, func_analysis, allocator));
    REQUIRE_OK(propagate_hints(func_analysis, allocator));
    REQUIRE_OK(do_allocation(mem, func_analysis, allocator));
    return KEFIR_OK;
}

static kefir_result_t free_allocation(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_node_id_t key,
                                      kefir_graph_node_value_t value, void *payload) {
    UNUSED(graph);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_sysv_amd64_register_allocation *, allocation, value);
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid register allocation entry"));

    memset(allocation, 0, sizeof(struct kefir_codegen_opt_sysv_amd64_register_allocation));
    KEFIR_FREE(mem, allocation);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(
    struct kefir_mem *mem, const struct kefir_opt_function *function,
    const struct kefir_opt_code_analysis *func_analysis,
    struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 System-V register allocator"));

    REQUIRE_OK(kefir_bitset_init(&allocator->general_purpose_regs));
    REQUIRE_OK(kefir_bitset_init(&allocator->floating_point_regs));
    REQUIRE_OK(kefir_bitset_init(&allocator->spilled_regs));
    REQUIRE_OK(kefir_graph_init(&allocator->allocation, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_graph_on_removal(&allocator->allocation, free_allocation, NULL));

    kefir_result_t res =
        kefir_bitset_resize(mem, &allocator->general_purpose_regs, KefirOptSysvAmd64NumOfGeneralPurposeRegisters);
    REQUIRE_CHAIN(
        &res, kefir_bitset_resize(mem, &allocator->floating_point_regs, KefirOptSysvAmd64NumOfFloatingPointRegisters));
    REQUIRE_CHAIN(&res, register_allocation_impl(mem, function, func_analysis, allocator));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_graph_free(mem, &allocator->allocation);
        kefir_bitset_free(mem, &allocator->spilled_regs);
        kefir_bitset_free(mem, &allocator->floating_point_regs);
        kefir_bitset_free(mem, &allocator->general_purpose_regs);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_of(
    const struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation **allocation) {
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V register allocator"));
    REQUIRE(allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V register allocation entry"));

    struct kefir_graph_node *node = NULL;
    REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_ref, &node));
    *allocation = (struct kefir_codegen_opt_sysv_amd64_register_allocation *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V register allocator"));

    REQUIRE_OK(kefir_graph_free(mem, &allocator->allocation));
    REQUIRE_OK(kefir_bitset_free(mem, &allocator->spilled_regs));
    REQUIRE_OK(kefir_bitset_free(mem, &allocator->floating_point_regs));
    REQUIRE_OK(kefir_bitset_free(mem, &allocator->general_purpose_regs));
    return KEFIR_OK;
}
