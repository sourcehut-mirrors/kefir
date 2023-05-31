#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/target/abi/system-v-amd64/function.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/core/bitset.h"
#include "kefir/core/graph.h"

extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[];
extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[];
extern const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters;
extern const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters;

typedef enum kefir_codegen_opt_sysv_amd64_register_allocation_class {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_SKIP,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_GENERAL_PURPOSE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_FLOATING_POINT
} kefir_codegen_opt_sysv_amd64_register_allocation_class_t;

typedef enum kefir_codegen_opt_sysv_amd64_register_allocation_type {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT
} kefir_codegen_opt_sysv_amd64_register_allocation_type_t;

typedef struct kefir_codegen_opt_sysv_amd64_register_allocation {
    kefir_codegen_opt_sysv_amd64_register_allocation_class_t klass;

    struct {
        kefir_codegen_opt_sysv_amd64_register_allocation_type_t type;

        union {
            kefir_size_t register_index;
            kefir_size_t spill_index;
            struct {
                kefir_asm_amd64_xasmgen_register_t base_register;
                kefir_int64_t offset;
            } indirect;
        };
    } result;

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
    struct kefir_hashtree argument_preallocations;
} kefir_codegen_opt_sysv_amd64_register_allocator_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(
    struct kefir_mem *, const struct kefir_opt_function *, const struct kefir_opt_code_analysis *,
    const struct kefir_abi_amd64_sysv_function_decl *, struct kefir_codegen_opt_sysv_amd64_register_allocator *);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_of(
    const struct kefir_codegen_opt_sysv_amd64_register_allocator *, kefir_opt_instruction_ref_t,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation **);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_free(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_register_allocator *);

#endif
