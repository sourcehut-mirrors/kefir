#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_INLINE_ASSEMBLY_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_INLINE_ASSEMBLY_H_

#include "kefir/optimizer/code.h"
#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/core/string_builder.h"

typedef enum kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_SCALAR,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE
} kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t;

typedef enum kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_type {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK
} kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_type_t;

typedef struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry {
    kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t type;
    kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_type_t allocation_type;
    union {
        kefir_asm_amd64_xasmgen_register_t reg;

        struct {
            kefir_asm_amd64_xasmgen_register_t reg;
            kefir_int64_t offset;
        } indirect;

        struct {
            kefir_size_t index;
        } stack;
    } allocation;

    kefir_bool_t direct_value;
    kefir_bool_t register_aggregate;

    struct {
        kefir_bool_t preserved;
        kefir_size_t stack_index;
    } output_address;

    struct {
        kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t type;
        kefir_size_t size;
    } parameter_props;

    struct {
        kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t type;
        kefir_size_t size;
    } parameter_read_props;
} kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry_t;

typedef struct kefir_codegen_opt_sysv_amd64_inline_assembly_context {
    struct kefir_codegen_opt_amd64 *codegen;
    const struct kefir_opt_function *function;
    const struct kefir_opt_code_analysis *func_analysis;
    struct kefir_opt_sysv_amd64_function *codegen_func;
    const struct kefir_opt_instruction *instr;
    struct kefir_opt_inline_assembly_node *inline_assembly;
    const struct kefir_ir_inline_assembly *ir_inline_assembly;

    struct kefir_hashtreeset dirty_registers;
    struct kefir_list available_registers;
    kefir_bool_t dirty_cc;

    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *parameter_allocation;
    struct kefir_string_builder formatted_asm;

    struct {
        kefir_bool_t initialized;
        kefir_asm_amd64_xasmgen_register_t base_register;
        kefir_size_t count;
    } stack_input_parameters;

    struct {
        kefir_size_t count;
    } stack_output_parameters;

    struct {
        kefir_size_t preserved_reg_offset;
        kefir_size_t preserved_reg_size;
        kefir_size_t output_parameter_offset;
        kefir_size_t input_parameter_offset;
        kefir_size_t total_size;
    } stack_map;
} kefir_codegen_opt_sysv_amd64_inline_assembly_context_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_mark_clobbers(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_allocate_parameters(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_prepare_state(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_restore_state(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_format(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_store_outputs(
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);
kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_jump_trampolines(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *);

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
    kefir_asm_amd64_xasmgen_register_t, kefir_size_t, kefir_asm_amd64_xasmgen_register_t *);

#endif
