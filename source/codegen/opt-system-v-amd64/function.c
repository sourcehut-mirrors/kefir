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

#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t generate_code(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                             struct kefir_opt_module *module, const struct kefir_opt_function *function,
                             const struct kefir_opt_code_analysis *func_analysis,
                             struct kefir_codegen_opt_sysv_amd64_register_allocator *register_allocator) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func_analysis);
    UNUSED(register_allocator);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", function->ir_func->name));
    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(register_allocator, instr_props->instr_ref,
                                                                       &reg_allocation));

        switch (reg_allocation->result.type) {
            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%zu - NONE", instr_idx));
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                    &codegen->xasmgen, "%zu - GENERAL PURPOSE REGISTER %s", instr_idx,
                    kefir_asm_amd64_xasmgen_register_symbolic_name(
                        KefirOptSysvAmd64GeneralPurposeRegisters[reg_allocation->result.register_index])));
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                    &codegen->xasmgen, "%zu - FLOATING POINT REGISTER %s", instr_idx,
                    kefir_asm_amd64_xasmgen_register_symbolic_name(
                        KefirOptSysvAmd64FloatingPointRegisters[reg_allocation->result.register_index])));
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%zu - STORAGE %zu", instr_idx,
                                                       reg_allocation->result.spill_index));
                break;

            case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(
                    &codegen->xasmgen, "%zu - INDIRECT %s " KEFIR_INT64_FMT, instr_idx,
                    kefir_asm_amd64_xasmgen_register_symbolic_name(reg_allocation->result.indirect.base_register),
                    reg_allocation->result.indirect.offset));
                break;
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_RET(&codegen->xasmgen));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *mem,
                                                               struct kefir_codegen_opt_amd64 *codegen,
                                                               struct kefir_opt_module *module,
                                                               const struct kefir_opt_function *function,
                                                               const struct kefir_opt_code_analysis *func_analysis) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func_analysis);

    struct kefir_opt_sysv_amd64_function sysv_amd64_function;
    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_alloc(mem, function->ir_func->declaration,
                                                        &sysv_amd64_function.declaration));

    kefir_result_t res = kefir_codegen_opt_amd64_sysv_function_parameters_init(mem, &sysv_amd64_function.declaration,
                                                                               &sysv_amd64_function.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration);
        return res;
    });

    res = kefir_codegen_opt_sysv_amd64_register_allocation(
        mem, function, func_analysis, &sysv_amd64_function.parameters, &sysv_amd64_function.register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function.parameters);
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration);
        return res;
    });

    res = generate_code(mem, codegen, module, function, func_analysis, &sysv_amd64_function.register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &sysv_amd64_function.register_allocator);
        kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function.parameters);
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration);
        return res;
    });

    res = kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &sysv_amd64_function.register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function.parameters);
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration);
        return res;
    });

    res = kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration);
        return res;
    });

    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function.declaration));
    return KEFIR_OK;
}
