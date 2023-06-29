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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing type");
}

struct vararg_getter_arg {
    struct kefir_mem *mem;
    struct kefir_codegen_opt_amd64 *codegen;
    struct kefir_opt_sysv_amd64_function *codegen_func;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation;
};

static kefir_result_t vararg_visit_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RDI, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &arg->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&arg->codegen->xasmgen_helpers.operands[0],
                                              KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, tmp_reg, result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM0, &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, &tmp_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RDI, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &arg->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&arg->codegen->xasmgen_helpers.operands[0],
                                              KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &tmp_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));

    return KEFIR_OK;
}

DEFINE_TRANSLATOR(vararg_get) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    const kefir_id_t type_id = (kefir_id_t) instr->operation.parameters.typed_refs.type_id;
    const kefir_size_t type_index = (kefir_size_t) instr->operation.parameters.typed_refs.type_index;
    struct kefir_ir_type *type = kefir_ir_module_get_named_type(module->ir_module, type_id);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named IR type"));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, vararg_visit_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, vararg_visit_sse);
    // visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = vararg_visit_long_double;
    // visitor.visit[KEFIR_IR_TYPE_STRUCT] = vararg_visit_aggregate;
    // visitor.visit[KEFIR_IR_TYPE_UNION] = vararg_visit_aggregate;
    // visitor.visit[KEFIR_IR_TYPE_ARRAY] = vararg_visit_aggregate;
    // visitor.visit[KEFIR_IR_TYPE_BUILTIN] = vararg_visit_builtin;
    struct vararg_getter_arg param = {.mem = mem,
                                      .codegen = codegen,
                                      .codegen_func = codegen_func,
                                      .arg_allocation = arg_allocation,
                                      .result_allocation = result_allocation};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &param, type_index, 1));

    return KEFIR_OK;
}
