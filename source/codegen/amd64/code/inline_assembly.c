/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/parameters.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

typedef enum inline_assembly_parameter_type {
    INLINE_ASSEMBLY_PARAMETER_SCALAR,
    INLINE_ASSEMBLY_PARAMETER_AGGREGATE
} inline_assembly_parameter_type_t;

typedef enum inline_assembly_parameter_allocation_type {
    INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER,
    INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT,
    INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY
} inline_assembly_parameter_allocation_type_t;

typedef struct inline_assembly_parameter_allocation_entry {
    inline_assembly_parameter_type_t type;
    inline_assembly_parameter_allocation_type_t allocation_type;

    kefir_asmcmp_virtual_register_index_t allocation_vreg;
    kefir_asmcmp_virtual_register_index_t output_address_vreg;

    kefir_bool_t direct_value;
    kefir_bool_t register_aggregate;

    struct {
        inline_assembly_parameter_type_t type;
        kefir_size_t size;
    } parameter_props;

    struct {
        inline_assembly_parameter_type_t type;
        kefir_size_t size;
    } parameter_read_props;
} inline_assembly_parameter_allocation_entry_t;

struct inline_assembly_context {
    const struct kefir_opt_instruction *instruction;
    const struct kefir_opt_inline_assembly_node *inline_assembly;
    const struct kefir_ir_inline_assembly *ir_inline_assembly;
    kefir_asmcmp_stash_index_t stash_idx;
    kefir_bool_t dirty_cc;
    struct kefir_list available_registers;
    struct inline_assembly_parameter_allocation_entry *parameters;
    kefir_asmcmp_inline_assembly_index_t inline_asm_idx;
    struct kefir_hashtree jump_trampolines;
};

static kefir_result_t mark_clobbers(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    struct inline_assembly_context *context) {
    REQUIRE_OK(kefir_asmcmp_register_stash_new(mem, &function->code.context, &context->stash_idx));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&context->ir_inline_assembly->clobbers, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, clobber, node->key);

        if (strcmp(clobber, "cc") == 0) {
            context->dirty_cc = true;
            continue;
        }

        kefir_asm_amd64_xasmgen_register_t dirty_reg;
        kefir_result_t res = kefir_asm_amd64_xasmgen_register_from_symbolic_name(clobber, &dirty_reg);
        if (res == KEFIR_NOT_FOUND) {
            // Ignore unknown clobbers
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(dirty_reg, &dirty_reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, context->stash_idx, dirty_reg));
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_use_register(mem, &function->stack_frame, dirty_reg));
    }

    return KEFIR_OK;
}

static kefir_result_t init_available_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          struct inline_assembly_context *context) {
    for (kefir_size_t i = 0;
         i < kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(function->codegen->abi_variant); i++) {
        kefir_asm_amd64_xasmgen_register_t candidate;
        REQUIRE_OK(kefir_abi_amd64_get_caller_preserved_general_purpose_register(function->codegen->abi_variant, i,
                                                                                 &candidate));

        kefir_bool_t stashed;
        REQUIRE_OK(kefir_asmcmp_register_stash_has(&function->code.context, context->stash_idx, candidate, &stashed));
        if (stashed) {
            continue;
        }

        REQUIRE_OK(kefir_list_insert_after(mem, &context->available_registers,
                                           kefir_list_tail(&context->available_registers),
                                           (void *) (kefir_uptr_t) candidate));
    }

    for (kefir_size_t i = 0;
         i < kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(function->codegen->abi_variant); i++) {
        kefir_asm_amd64_xasmgen_register_t candidate;
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(function->codegen->abi_variant, i,
                                                                                 &candidate));

        kefir_bool_t stashed;
        REQUIRE_OK(kefir_asmcmp_register_stash_has(&function->code.context, context->stash_idx, candidate, &stashed));
        if (stashed) {
            continue;
        }

        REQUIRE_OK(kefir_list_insert_after(mem, &context->available_registers,
                                           kefir_list_tail(&context->available_registers),
                                           (void *) (kefir_uptr_t) candidate));
    }
    return KEFIR_OK;
}

static kefir_result_t obtain_available_register(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                struct inline_assembly_context *context,
                                                kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(kefir_list_length(&context->available_registers) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
    *reg = (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&context->available_registers)->value);
    REQUIRE_OK(kefir_list_pop(mem, &context->available_registers, kefir_list_head(&context->available_registers)));
    REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, context->stash_idx, *reg));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_use_register(mem, &function->stack_frame, *reg));
    return KEFIR_OK;
}

static kefir_result_t allocate_register_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                  struct inline_assembly_context *context,
                                                  const struct kefir_ir_inline_assembly_parameter *ir_asm_param,
                                                  inline_assembly_parameter_type_t param_type) {
    struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];
    kefir_asm_amd64_xasmgen_register_t reg = 0;
    REQUIRE_OK(obtain_available_register(mem, function, context, &reg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &entry->allocation_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, entry->allocation_vreg, reg));
    entry->type = param_type;
    entry->allocation_type = INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER;
    if (param_type == INLINE_ASSEMBLY_PARAMETER_AGGREGATE) {
        entry->register_aggregate = true;
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_memory_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                struct inline_assembly_context *context,
                                                const struct kefir_ir_inline_assembly_parameter *ir_asm_param,
                                                inline_assembly_parameter_type_t param_type) {
    struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];
    if (param_type == INLINE_ASSEMBLY_PARAMETER_AGGREGATE ||
        ir_asm_param->klass != KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
        kefir_asm_amd64_xasmgen_register_t reg = 0;
        REQUIRE_OK(obtain_available_register(mem, function, context, &reg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &entry->allocation_vreg));
        REQUIRE_OK(
            kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, entry->allocation_vreg, reg));
        entry->type = param_type;
        entry->allocation_type = INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT;
    } else {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(mem, &function->code.context, 1, 1,
                                                                                   &entry->allocation_vreg));

        entry->type = param_type;
        entry->allocation_type = INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY;
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_parameter_type(struct kefir_mem *mem, const struct kefir_ir_type *ir_type,
                                              kefir_size_t ir_type_idx, inline_assembly_parameter_type_t *param_type,
                                              kefir_size_t *parameter_size) {
    const struct kefir_ir_typeentry *param_typeentry = kefir_ir_type_at(ir_type, ir_type_idx);
    REQUIRE(param_typeentry != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

    struct kefir_abi_amd64_type_layout layouts;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &layouts));
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    kefir_result_t res = kefir_abi_amd64_type_layout_at(&layouts, ir_type_idx, &layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &layouts);
        return res;
    });
    *parameter_size = layout->size;
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &layouts));

    switch (param_typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
        case KEFIR_IR_TYPE_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
            *param_type = INLINE_ASSEMBLY_PARAMETER_SCALAR;
            break;

        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
            *param_type = INLINE_ASSEMBLY_PARAMETER_AGGREGATE;
            break;

        case KEFIR_IR_TYPE_BITS:
        case KEFIR_IR_TYPE_BUILTIN:
        case KEFIR_IR_TYPE_NONE:
        case KEFIR_IR_TYPE_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
    }

    return KEFIR_OK;
}

static kefir_result_t allocate_parameters(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          struct inline_assembly_context *context) {
    REQUIRE_OK(init_available_regs(mem, function, context));

    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];

        inline_assembly_parameter_type_t param_read_type = INLINE_ASSEMBLY_PARAMETER_SCALAR,
                                         param_type = INLINE_ASSEMBLY_PARAMETER_SCALAR;
        kefir_size_t param_read_size = 0, param_size = 0;
        kefir_bool_t parameter_immediate = false;
        kefir_bool_t direct_value = false;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
                REQUIRE_OK(evaluate_parameter_type(mem, ir_asm_param->type.type, ir_asm_param->type.index, &param_type,
                                                   &param_size));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(evaluate_parameter_type(mem, ir_asm_param->type.type, ir_asm_param->type.index, &param_type,
                                                   &param_size));
                REQUIRE_OK(evaluate_parameter_type(mem, ir_asm_param->read_type.type, ir_asm_param->read_type.index,
                                                   &param_read_type, &param_read_size));
                direct_value = true;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                REQUIRE_OK(evaluate_parameter_type(mem, ir_asm_param->type.type, ir_asm_param->type.index, &param_type,
                                                   &param_size));
                param_read_type = param_type;
                param_read_size = param_size;
                direct_value = true;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                REQUIRE_OK(evaluate_parameter_type(mem, ir_asm_param->type.type, ir_asm_param->type.index, &param_type,
                                                   &param_size));
                parameter_immediate = true;
                break;
        }

        if (!parameter_immediate) {
            switch (ir_asm_param->constraint) {
                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected IR inline assembly parameter constraint");

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER:
                    REQUIRE(param_type != INLINE_ASSEMBLY_PARAMETER_AGGREGATE || param_size <= KEFIR_AMD64_ABI_QWORD,
                            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                    REQUIRE_OK(allocate_register_parameter(mem, function, context, ir_asm_param, param_type));
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY:
                    if ((param_type == INLINE_ASSEMBLY_PARAMETER_SCALAR || param_size <= KEFIR_AMD64_ABI_QWORD) &&
                        kefir_list_length(&context->available_registers) > 1) {
                        REQUIRE_OK(allocate_register_parameter(mem, function, context, ir_asm_param, param_type));
                    } else {
                        REQUIRE_OK(allocate_memory_parameter(mem, function, context, ir_asm_param, param_type));
                    }
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY:
                    REQUIRE_OK(allocate_memory_parameter(mem, function, context, ir_asm_param, param_type));
                    break;
            }
        }

        if ((ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE ||
             ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE ||
             ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE) &&
            entry->allocation_type != INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT) {
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(mem, &function->code.context, 1,
                                                                                       1, &entry->output_address_vreg));
        } else {
            entry->output_address_vreg = KEFIR_ASMCMP_INDEX_NONE;
        }

        entry->parameter_props.size = param_size;
        entry->parameter_props.type = param_type;
        entry->direct_value = direct_value;
        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            entry->parameter_read_props.type = param_read_type;
            entry->parameter_read_props.size = param_read_size;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_output_addresses(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                struct inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];
        if (entry->output_address_vreg != KEFIR_ASMCMP_INDEX_NONE) {
            const struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly_get_parameter(
                &function->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));

            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, asm_param->load_store_ref, &vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(entry->output_address_vreg), &KEFIR_ASMCMP_MAKE_VREG64(vreg), NULL));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t load_inputs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                  struct inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);
        const struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_get_parameter(
            &function->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));
        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE ||
            (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE &&
             entry->allocation_type != INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT)) {
            continue;
        }

        kefir_asmcmp_virtual_register_index_t vreg = KEFIR_ASMCMP_INDEX_NONE;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, asm_param->load_store_ref, &vreg));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, asm_param->read_ref, &vreg));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                // Intentionally left blank
                break;
        }
        if (vreg != KEFIR_ASMCMP_INDEX_NONE) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), entry->allocation_vreg,
                vreg, NULL));
        }
    }
    return KEFIR_OK;
}

kefir_result_t match_vreg_to_size(kefir_asmcmp_virtual_register_index_t in, kefir_size_t sz,
                                  struct kefir_asmcmp_value *value) {
    if (sz <= 1) {
        *value = KEFIR_ASMCMP_MAKE_VREG8(in);
    } else if (sz <= 2) {
        *value = KEFIR_ASMCMP_MAKE_VREG16(in);
    } else if (sz <= 4) {
        *value = KEFIR_ASMCMP_MAKE_VREG32(in);
    } else if (sz <= 8) {
        *value = KEFIR_ASMCMP_MAKE_VREG64(in);
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to match register variant wider than 8 bytes");
    }
    return KEFIR_OK;
}

static kefir_result_t read_inputs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                  struct inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);
        const struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_get_parameter(
            &function->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));
        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        const struct kefir_ir_typeentry *param_type = NULL;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                param_type = kefir_ir_type_at(ir_asm_param->read_type.type, ir_asm_param->read_type.index);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                param_type = kefir_ir_type_at(ir_asm_param->type.type, ir_asm_param->type.index);
                break;
        }
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        struct kefir_asmcmp_value vreg_variant_value;
        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64:
            case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
                switch (entry->allocation_type) {
                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER:
                        if (!entry->direct_value) {
                            REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg, entry->parameter_props.size,
                                                          &vreg_variant_value));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &vreg_variant_value,
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->allocation_vreg, 0,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                NULL));
                        }
                        break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        REQUIRE(
                            !entry->direct_value,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY: {
                        if (!entry->direct_value) {
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &KEFIR_ASMCMP_MAKE_VREG(entry->allocation_vreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->allocation_vreg, 0,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                NULL));
                        }
                    } break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
            case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
            case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
                switch (entry->allocation_type) {
                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
                        REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg,
                                                      entry->direct_value
                                                          ? MIN(entry->parameter_read_props.size, KEFIR_AMD64_ABI_QWORD)
                                                          : entry->parameter_props.size,
                                                      &vreg_variant_value));

                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &vreg_variant_value,
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->allocation_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            NULL));

                    } break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                               "On-stack aggregate parameters of IR inline assembly are not supported");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_NONE:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t prepare_state(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    struct inline_assembly_context *context) {
    REQUIRE_OK(kefir_asmcmp_amd64_activate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), context->stash_idx, NULL));

    if (context->dirty_cc) {
        REQUIRE_OK(kefir_asmcmp_amd64_pushfq(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                          &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD), NULL));
    }

    REQUIRE_OK(preserve_output_addresses(mem, function, context));
    REQUIRE_OK(load_inputs(mem, function, context));
    REQUIRE_OK(read_inputs(mem, function, context));

    return KEFIR_OK;
}

static kefir_result_t match_parameter(const struct kefir_ir_inline_assembly *inline_asm, const char **input_str,
                                      const struct kefir_ir_inline_assembly_parameter **asm_param_ptr,
                                      const struct kefir_ir_inline_assembly_jump_target **jump_target_ptr) {
    *asm_param_ptr = NULL;
    *jump_target_ptr = NULL;
    struct kefir_hashtree_node_iterator iter;
    kefir_size_t last_match_length = 0;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        kefir_size_t template_parameter_length = strlen(identifier);
        if (template_parameter_length > last_match_length &&
            strncmp(*input_str, identifier, template_parameter_length) == 0) {
            *asm_param_ptr = asm_param;
            last_match_length = template_parameter_length;
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_jump_target *, jump_target, node->value);

        kefir_size_t identifier_length = strlen(identifier);
        if (identifier_length > last_match_length && strncmp(*input_str, identifier, identifier_length) == 0) {
            *asm_param_ptr = NULL;
            *jump_target_ptr = jump_target;
            last_match_length = identifier_length;
        }
    }

    if (last_match_length == 0) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Inline assembly parameter not found");
    } else {
        *input_str += last_match_length;
        return KEFIR_OK;
    }
}

static kefir_result_t format_label_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                             struct inline_assembly_context *context,
                                             const struct kefir_ir_inline_assembly_jump_target *jump_target) {
    kefir_asmcmp_label_index_t label;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->jump_trampolines, (kefir_hashtree_key_t) jump_target->uid, &node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));
        REQUIRE_OK(kefir_hashtree_insert(mem, &context->jump_trampolines, (kefir_hashtree_key_t) jump_target->uid,
                                         (kefir_hashtree_value_t) label));
    } else {
        REQUIRE_OK(res);
        label = node->value;
    }

    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(mem, &function->code.context, context->inline_asm_idx,
                                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label)));
    return KEFIR_OK;
}

static kefir_result_t label_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                           struct inline_assembly_context *context, const char **input_str) {
    UNUSED(mem);
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    REQUIRE(asm_param == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                            "Inline assembly label parameter modifier cannot be applied to register parameters"));
    REQUIRE(jump_target != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result"));
    *input_str = input;
    REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    return KEFIR_OK;
}

static kefir_result_t pointer_operand(const struct kefir_ir_inline_assembly_parameter *asm_param,
                                      struct inline_assembly_parameter_allocation_entry *entry,
                                      kefir_size_t override_size, kefir_asmcmp_operand_variant_t *variant) {

    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
    kefir_size_t param_size = override_size == 0 ? entry->parameter_props.size : override_size;

    if (override_size == 0 && entry->parameter_props.type == INLINE_ASSEMBLY_PARAMETER_AGGREGATE &&
        typeentry->typecode != KEFIR_IR_TYPE_LONG_DOUBLE) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
        return KEFIR_OK;
    }

    if (param_size <= 1) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_8BIT;
    } else if (param_size <= 2) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_16BIT;
    } else if (param_size <= 4) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_32BIT;
    } else if (param_size <= 8) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_64BIT;
    } else if (override_size == 10 || typeentry->typecode == KEFIR_IR_TYPE_LONG_DOUBLE) {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_80BIT;
    } else {
        *variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
    }
    return KEFIR_OK;
}

static kefir_result_t format_normal_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                              struct inline_assembly_context *context,
                                              const struct kefir_ir_inline_assembly_parameter *asm_param,
                                              kefir_size_t override_size) {
    if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
        switch (asm_param->immediate_type) {
            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED:
                if (asm_param->immediate_identifier_base != NULL) {
                    const struct kefir_ir_identifier *ir_identifier;
                    REQUIRE_OK(kefir_ir_module_get_identifier(function->module->ir_module,
                                                              asm_param->immediate_identifier_base, &ir_identifier));
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(
                        mem, &function->code.context, context->inline_asm_idx,
                        &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, ir_identifier->symbol,
                                                          asm_param->immediate_value)));
                } else {
                    REQUIRE_OK(
                        kefir_asmcmp_inline_assembly_add_value(mem, &function->code.context, context->inline_asm_idx,
                                                               &KEFIR_ASMCMP_MAKE_INT(asm_param->immediate_value)));
                }
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED: {
                const char *symbol;
                REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbol, KEFIR_AMD64_STRING_LITERAL,
                                               asm_param->immediate_literal_base));
                REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(
                    mem, &function->code.context, context->inline_asm_idx,
                    &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, symbol,
                                                      asm_param->immediate_value)));
            } break;
        }
        return KEFIR_OK;
    }

    struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[asm_param->parameter_id];

    kefir_size_t operand_size = override_size == 0 ? entry->parameter_props.size : override_size;

    switch (entry->allocation_type) {
        case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
            struct kefir_asmcmp_value value;
            REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg, operand_size, &value));
            REQUIRE_OK(
                kefir_asmcmp_inline_assembly_add_value(mem, &function->code.context, context->inline_asm_idx, &value));
        } break;

        case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT: {
            kefir_asmcmp_operand_variant_t op_variant;
            REQUIRE_OK(pointer_operand(asm_param, entry, override_size, &op_variant));
            REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(
                mem, &function->code.context, context->inline_asm_idx,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->allocation_vreg, 0, op_variant)));
        } break;

        case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY: {
            struct kefir_asmcmp_value value;
            REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg, operand_size, &value));
            REQUIRE_OK(
                kefir_asmcmp_inline_assembly_add_value(mem, &function->code.context, context->inline_asm_idx, &value));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t default_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                             struct inline_assembly_context *context, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, function, context, asm_param, 0));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t byte_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          struct inline_assembly_context *context, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, function, context, asm_param, 1));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t word_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          struct inline_assembly_context *context, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, function, context, asm_param, 2));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t dword_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                           struct inline_assembly_context *context, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, function, context, asm_param, 4));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t qword_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                           struct inline_assembly_context *context, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, function, context, asm_param, 8));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, function, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t inline_assembly_build(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                            struct inline_assembly_context *context) {
    const char *const template = context->ir_inline_assembly->template;
    REQUIRE_OK(kefir_asmcmp_inline_assembly_new(mem, &function->code.context, template, &context->inline_asm_idx));

    const char *template_iter = context->ir_inline_assembly->template;
    while (template_iter != NULL) {
        const char *format_specifier = strchr(template_iter, '%');
        if (format_specifier == NULL) {
            REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context, context->inline_asm_idx,
                                                             "%s", template_iter));
            template_iter = NULL;
        } else {
            REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context, context->inline_asm_idx,
                                                             "%.*s", format_specifier - template_iter, template_iter));
            switch (format_specifier[1]) {
                case '%':
                case '{':
                case '|':
                case '}':
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(
                        mem, &function->code.context, context->inline_asm_idx, "%.1s", format_specifier + 1));
                    template_iter = format_specifier + 2;
                    break;

                case '=':
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                     context->inline_asm_idx, "%" KEFIR_SIZE_FMT,
                                                                     context->inline_asm_idx));
                    template_iter = format_specifier + 2;
                    break;

                case 'l': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = label_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, function, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'b': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = byte_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, function, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'w': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = word_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, function, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'd': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = dword_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, function, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'q': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = qword_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, function, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                default: {
                    template_iter = format_specifier + 1;
                    kefir_result_t res = default_param_modifier(mem, function, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_asmcmp_inline_assembly_add_text(mem, &function->code.context,
                                                                    context->inline_asm_idx, "%%");
                    }
                    REQUIRE_OK(res);
                } break;
            }
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             entry->allocation_vreg, NULL));
    }

    kefir_asmcmp_instruction_index_t instr_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_inline_assembly(mem, &function->code,
                                                  kefir_asmcmp_context_instr_tail(&function->code.context),
                                                  context->inline_asm_idx, &instr_idx));

    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, context->stash_idx, instr_idx));

    return KEFIR_OK;
}

static kefir_result_t store_register_aggregate_outputs(struct kefir_mem *mem,
                                                       struct kefir_codegen_amd64_function *function,
                                                       struct inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];

        if (!entry->register_aggregate || ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct kefir_asmcmp_value value;
        REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg, entry->parameter_props.size, &value));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->output_address_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            &value, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t store_outputs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    struct inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct inline_assembly_parameter_allocation_entry *entry = &context->parameters[ir_asm_param->parameter_id];

        const struct kefir_ir_typeentry *param_type =
            kefir_ir_type_at(ir_asm_param->type.type, ir_asm_param->type.index);
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64:
            case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
                switch (entry->allocation_type) {
                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
                        struct kefir_asmcmp_value value;
                        REQUIRE_OK(match_vreg_to_size(entry->allocation_vreg, entry->parameter_props.size, &value));

                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->output_address_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &value, NULL));
                    } break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        REQUIRE(
                            !entry->direct_value,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY:
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->output_address_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(entry->allocation_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            NULL));
                        break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
            case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
            case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
                switch (entry->allocation_type) {
                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER:
                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;

                    case INLINE_ASSEMBLY_PARAMETER_ALLOCATION_MEMORY:
                        return KEFIR_SET_ERROR(
                            KEFIR_INVALID_STATE,
                            "On-stack aggregate parameters of IR inline assembly are not supported yet");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_NONE:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }

    REQUIRE_OK(store_register_aggregate_outputs(mem, function, context));
    return KEFIR_OK;
}

static kefir_result_t restore_state(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    struct inline_assembly_context *context) {
    if (context->dirty_cc) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                          &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_popfq(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), context->stash_idx, NULL));

    return KEFIR_OK;
}

static kefir_result_t jump_trampolines(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       struct inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));
    REQUIRE(!kefir_hashtree_empty(&context->inline_assembly->jump_targets), KEFIR_OK);

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(
        mem, function, context->inline_assembly->default_jump_target, context->instruction->block_id));

    struct kefir_hashtree_node *target_label_node;
    REQUIRE_OK(kefir_hashtree_at(
        &function->labels, (kefir_hashtree_key_t) context->inline_assembly->default_jump_target, &target_label_node));
    ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, target_label_node->value);

    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&context->inline_assembly->jump_targets, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
        kefir_result_t res = kefir_hashtree_at(&context->jump_trampolines, node->key, &target_label_node);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(
                kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &target_label));
        } else {
            REQUIRE_OK(res);
            target_label = target_label_node->value;
        }

        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, target_label));

        REQUIRE_OK(store_outputs(mem, function, context));
        REQUIRE_OK(restore_state(mem, function, context));
        REQUIRE_OK(
            kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block, context->instruction->block_id));

        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block, &target_label_node));
        target_label = (kefir_asmcmp_label_index_t) target_label_node->value;
        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(inline_assembly)(struct kefir_mem *mem,
                                                                     struct kefir_codegen_amd64_function *function,
                                                                     const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct inline_assembly_context context = {.instruction = instruction, .dirty_cc = false};
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(
        &function->function->code, instruction->operation.parameters.inline_asm_ref, &context.inline_assembly));
    context.ir_inline_assembly =
        kefir_ir_module_get_inline_assembly(function->module->ir_module, context.inline_assembly->inline_asm_id);
    REQUIRE(context.ir_inline_assembly != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR inline assembly"));

    REQUIRE_OK(kefir_list_init(&context.available_registers));
    REQUIRE_OK(kefir_hashtree_init(&context.jump_trampolines, &kefir_hashtree_uint_ops));
    context.parameters = KEFIR_MALLOC(
        mem, sizeof(struct inline_assembly_parameter_allocation_entry) * context.inline_assembly->parameter_count);
    REQUIRE(context.parameters != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate inline assembly parameters"));
    memset(context.parameters, 0,
           sizeof(struct inline_assembly_parameter_allocation_entry) * context.inline_assembly->parameter_count);

    kefir_result_t res = mark_clobbers(mem, function, &context);
    REQUIRE_CHAIN(&res, allocate_parameters(mem, function, &context));
    REQUIRE_CHAIN(&res, prepare_state(mem, function, &context));
    REQUIRE_CHAIN(&res, inline_assembly_build(mem, function, &context));
    REQUIRE_CHAIN(&res, store_outputs(mem, function, &context));
    REQUIRE_CHAIN(&res, restore_state(mem, function, &context));
    REQUIRE_CHAIN(&res, jump_trampolines(mem, function, &context));
    REQUIRE_CHAIN(&res, kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, context.parameters);
        kefir_hashtree_free(mem, &context.jump_trampolines);
        kefir_list_free(mem, &context.available_registers);
        return res;
    });

    memset(context.parameters, 0,
           sizeof(struct inline_assembly_parameter_allocation_entry) * context.inline_assembly->parameter_count);
    KEFIR_FREE(mem, context.parameters);
    res = kefir_hashtree_free(mem, &context.jump_trampolines);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &context.available_registers);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &context.available_registers));

    return KEFIR_OK;
}
