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

#include "kefir/codegen/codegen.h"
#include "kefir/codegen/amd64-common.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

const struct kefir_codegen_configuration KefirCodegenDefaultConfiguration = {
    .emulated_tls = false,
    .position_independent_code = false,
    .omit_frame_pointer = false,
    .syntax = KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_PREFIX,
    .print_details = NULL,
    .detailed_output = false,
    .pipeline_spec = NULL,
    .debug_info = false};

kefir_result_t kefir_codegen_translate_ir(struct kefir_mem *mem, struct kefir_codegen *codegen,
                                          struct kefir_ir_module *ir_module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    struct kefir_opt_module opt_module;
    REQUIRE_OK(kefir_opt_module_init(mem, ir_module, &opt_module));

    kefir_result_t res = codegen->translate_optimized(mem, codegen, &opt_module, NULL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_module_free(mem, &opt_module);
        return res;
    });
    REQUIRE_OK(kefir_opt_module_free(mem, &opt_module));
    return KEFIR_OK;
}
