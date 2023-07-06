/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/codegen/opt-system-v-amd64/inline_assembly.h"
#include "kefir/codegen/opt-system-v-amd64/storage.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t init_available_regs(struct kefir_mem *mem,
                                          struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirOptSysvAmd64GeneralPurposeRegisters[i];

        if (kefir_hashtreeset_has(&context->dirty_registers, reg)) {
            continue;
        }

        kefir_bool_t occupied;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&context->codegen_func->storage, reg, &occupied));
        if (!occupied) {
            REQUIRE_OK(kefir_list_insert_after(mem, &context->available_registers,
                                               kefir_list_tail(&context->available_registers),
                                               (void *) (kefir_uptr_t) reg));
        }
    }

    for (kefir_size_t i = 0; i < KefirOptSysvAmd64NumOfGeneralPurposeRegisters; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirOptSysvAmd64GeneralPurposeRegisters[i];

        if (kefir_hashtreeset_has(&context->dirty_registers, reg)) {
            continue;
        }

        kefir_bool_t occupied;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&context->codegen_func->storage, reg, &occupied));
        if (occupied) {
            REQUIRE_OK(kefir_list_insert_after(mem, &context->available_registers,
                                               kefir_list_tail(&context->available_registers),
                                               (void *) (kefir_uptr_t) reg));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t evaluate_parameter_type(struct kefir_mem *mem, const struct kefir_ir_type *ir_type,
                                              kefir_size_t ir_type_idx,
                                              kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t *param_type,
                                              kefir_size_t *parameter_size) {
    const struct kefir_ir_typeentry *param_typeentry = kefir_ir_type_at(ir_type, ir_type_idx);
    REQUIRE(param_typeentry != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

    struct kefir_abi_sysv_amd64_type_layout layouts;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(ir_type, mem, &layouts));
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    kefir_result_t res = kefir_abi_sysv_amd64_type_layout_at(&layouts, ir_type_idx, &layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, &layouts);
        return res;
    });
    *parameter_size = layout->size;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layouts));

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
            *param_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_SCALAR;
            break;

        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
            *param_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE;
            break;

        case KEFIR_IR_TYPE_BITS:
        case KEFIR_IR_TYPE_BUILTIN:
        case KEFIR_IR_TYPE_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
    }

    return KEFIR_OK;
}

static kefir_result_t obtain_available_register(struct kefir_mem *mem,
                                                struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                                kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(kefir_list_length(&context->available_registers) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
    *reg = (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&context->available_registers)->value);
    REQUIRE_OK(kefir_list_pop(mem, &context->available_registers, kefir_list_head(&context->available_registers)));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &context->dirty_registers, (kefir_hashtreeset_entry_t) *reg));
    return KEFIR_OK;
}

static kefir_result_t allocate_register_parameter(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
    const struct kefir_ir_inline_assembly_parameter *ir_asm_param,
    kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t param_type) {
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
        &context->parameter_allocation[ir_asm_param->parameter_id];
    REQUIRE_OK(obtain_available_register(mem, context, &entry->allocation.reg));
    entry->type = param_type;
    entry->allocation_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER;
    if (param_type == KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE) {
        entry->register_aggregate = true;
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_memory_parameter(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
    const struct kefir_ir_inline_assembly_parameter *ir_asm_param,
    kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t param_type) {
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
        &context->parameter_allocation[ir_asm_param->parameter_id];
    if (param_type == KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE ||
        ir_asm_param->klass != KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
        REQUIRE_OK(obtain_available_register(mem, context, &entry->allocation.reg));
        entry->type = param_type;
        entry->allocation_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT;
    } else {
        if (!context->stack_input_parameters.initialized) {
            REQUIRE_OK(obtain_available_register(mem, context, &context->stack_input_parameters.base_register));
            context->stack_input_parameters.initialized = true;
        }

        entry->type = param_type;
        entry->allocation_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK;
        entry->allocation.stack.index = context->stack_input_parameters.count++;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_allocate_parameters(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

    REQUIRE_OK(init_available_regs(mem, context));
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];

        kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_type_t
            param_read_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_SCALAR,
            param_type = KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_SCALAR;
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
                    REQUIRE(param_type != KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE ||
                                param_size <= KEFIR_AMD64_SYSV_ABI_QWORD,
                            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                    REQUIRE_OK(allocate_register_parameter(mem, context, ir_asm_param, param_type));
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY:
                    if ((param_type == KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_SCALAR ||
                         param_size <= KEFIR_AMD64_SYSV_ABI_QWORD) &&
                        kefir_list_length(&context->available_registers) > 1) {
                        REQUIRE_OK(allocate_register_parameter(mem, context, ir_asm_param, param_type));
                    } else {
                        REQUIRE_OK(allocate_memory_parameter(mem, context, ir_asm_param, param_type));
                    }
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY:
                    REQUIRE_OK(allocate_memory_parameter(mem, context, ir_asm_param, param_type));
                    break;
            }
        }

        if ((ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE ||
             ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE ||
             ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE) &&
            entry->allocation_type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT) {
            entry->output_address.preserved = true;
            entry->output_address.stack_index = context->stack_output_parameters.count++;
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
