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
#include "kefir/codegen/opt-system-v-amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t init_codegen_func(struct kefir_mem *mem, const struct kefir_opt_function *function,
                                        const struct kefir_opt_code_analysis *func_analysis,
                                        struct kefir_opt_sysv_amd64_function *sysv_amd64_function) {
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_init(&sysv_amd64_function->storage));
    REQUIRE_OK(kefir_list_init(&sysv_amd64_function->alive_instr));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_init(&sysv_amd64_function->stack_frame));
    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_alloc(mem, function->ir_func->declaration,
                                                        &sysv_amd64_function->declaration));

    kefir_result_t res = kefir_codegen_opt_amd64_sysv_function_parameters_init(mem, &sysv_amd64_function->declaration,
                                                                               &sysv_amd64_function->parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
        kefir_list_free(mem, &sysv_amd64_function->alive_instr);
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    res = kefir_codegen_opt_sysv_amd64_register_allocation(
        mem, function, func_analysis, &sysv_amd64_function->parameters, &sysv_amd64_function->stack_frame,
        &sysv_amd64_function->register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function->parameters);
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
        kefir_list_free(mem, &sysv_amd64_function->alive_instr);
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    if (function->ir_func->locals != NULL) {
        res = kefir_abi_sysv_amd64_type_layout(function->ir_func->locals, mem, &sysv_amd64_function->locals_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &sysv_amd64_function->register_allocator);
            kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function->parameters);
            kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
            kefir_list_free(mem, &sysv_amd64_function->alive_instr);
            kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
            return res;
        });
    }

    sysv_amd64_function->nonblock_labels = 0;
    return KEFIR_OK;
}

static kefir_result_t free_codegen_func(struct kefir_mem *mem, const struct kefir_opt_function *function,
                                        struct kefir_opt_sysv_amd64_function *sysv_amd64_function) {
    kefir_result_t res;

    if (function->ir_func->locals != NULL) {
        res = kefir_abi_sysv_amd64_type_layout_free(mem, &sysv_amd64_function->locals_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &sysv_amd64_function->register_allocator);
            kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function->parameters);
            kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
            kefir_list_free(mem, &sysv_amd64_function->alive_instr);
            kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
            return res;
        });
    }

    res = kefir_codegen_opt_sysv_amd64_register_allocation_free(mem, &sysv_amd64_function->register_allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function->parameters);
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
        kefir_list_free(mem, &sysv_amd64_function->alive_instr);
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    res = kefir_codegen_opt_amd64_sysv_function_parameters_free(mem, &sysv_amd64_function->parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
        kefir_list_free(mem, &sysv_amd64_function->alive_instr);
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    res = kefir_abi_amd64_sysv_function_decl_free(mem, &sysv_amd64_function->declaration);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &sysv_amd64_function->alive_instr);
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    res = kefir_list_free(mem, &sysv_amd64_function->alive_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage);
        return res;
    });

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_free(mem, &sysv_amd64_function->storage));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *mem,
                                                               struct kefir_codegen_opt_amd64 *codegen,
                                                               struct kefir_opt_module *module,
                                                               const struct kefir_opt_function *function,
                                                               const struct kefir_opt_code_analysis *func_analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));

    struct kefir_opt_sysv_amd64_function sysv_amd64_function;
    REQUIRE_OK(init_codegen_func(mem, function, func_analysis, &sysv_amd64_function));

    kefir_result_t res = kefir_codegen_opt_sysv_amd64_translate_code(mem, codegen, module, function, func_analysis,
                                                                     &sysv_amd64_function);
    REQUIRE_ELSE(res == KEFIR_OK, {
        free_codegen_func(mem, function, &sysv_amd64_function);
        return res;
    });

    REQUIRE_OK(free_codegen_func(mem, function, &sysv_amd64_function));
    return KEFIR_OK;
}
