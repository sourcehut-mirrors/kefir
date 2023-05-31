#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/core/bitset.h"
#include "kefir/core/graph.h"

extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[];
extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[];
extern const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters;
extern const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters;

typedef enum kefir_codegen_opt_sysv_amd64_register_allocation_class {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SKIP,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT
} kefir_codegen_opt_sysv_amd64_register_allocation_class_t;

typedef struct kefir_codegen_opt_sysv_amd64_register_allocation {
    kefir_bool_t done;
    kefir_codegen_opt_sysv_amd64_register_allocation_class_t klass;
    kefir_bool_t spilled;
    kefir_size_t index;

    struct {
        kefir_bool_t present;
        kefir_size_t index;
    } register_hint;

    struct {
        kefir_bool_t present;
        kefir_opt_instruction_ref_t instr_ref;
    } alias_hint;
} kefir_codegen_opt_sysv_amd64_register_allocation_t;

typedef struct kefir_codegen_opt_sysv_amd64_register_allocator {
    struct kefir_bitset general_purpose_regs;
    struct kefir_bitset floating_point_regs;
    struct kefir_bitset spilled_regs;
    struct kefir_graph allocation;
} kefir_codegen_opt_sysv_amd64_register_allocator_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(
    struct kefir_mem *, const struct kefir_opt_function *, const struct kefir_opt_code_analysis *,
    struct kefir_codegen_opt_sysv_amd64_register_allocator *);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_of(
    const struct kefir_codegen_opt_sysv_amd64_register_allocator *, kefir_opt_instruction_ref_t,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation **);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_free(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_register_allocator *);

#endif
