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

#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t translate_code(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    UNUSED(mem);
    UNUSED(func);

    // Initialize block labels
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];
        kefir_asmcmp_label_index_t asmlabel;
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
        REQUIRE_OK(kefir_hashtree_insert(mem, &func->labels, (kefir_hashtree_key_t) block_props->block_id,
                                         (kefir_hashtree_value_t) asmlabel));
    }

    // Translate blocks
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];

        struct kefir_hashtree_node *asmlabel_node;
        REQUIRE_OK(kefir_hashtree_at(&func->labels, (kefir_hashtree_key_t) block_props->block_id, &asmlabel_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, asmlabel, asmlabel_node->value);

        kefir_asmcmp_instruction_index_t block_head = KEFIR_ASMCMP_INDEX_NONE;
        REQUIRE_OK(kefir_asmcmp_amd64_ret(mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context),
                                          &block_head));
        REQUIRE_OK(kefir_asmcmp_context_attach_label(&func->code.context, block_head, asmlabel));
    }
    return KEFIR_OK;
}

static kefir_result_t generate_code(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                    struct kefir_codegen_amd64_function *func) {
    UNUSED(mem);
    REQUIRE_OK(kefir_asmcmp_amd64_generate_code(&codegen->xasmgen, &func->code));
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_translate_impl(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64 *codegen,
                                                                  struct kefir_codegen_amd64_function *func) {
    UNUSED(mem);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, "%s", func->function->ir_func->name));

    REQUIRE_OK(translate_code(mem, func));
    REQUIRE_OK(generate_code(mem, codegen, func));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                      const struct kefir_opt_module *module,
                                                      const struct kefir_opt_function *function,
                                                      const struct kefir_opt_code_analysis *function_analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(function_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));

    struct kefir_codegen_amd64_function func = {
        .codegen = codegen, .module = module, .function = function, .function_analysis = function_analysis};
    REQUIRE_OK(kefir_asmcmp_amd64_init(function->ir_func->name, &func.code));
    REQUIRE_OK(kefir_hashtree_init(&func.labels, &kefir_hashtree_uint_ops));

    kefir_result_t res = kefir_codegen_amd64_function_translate_impl(mem, codegen, &func);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &func.labels);
        kefir_asmcmp_amd64_free(mem, &func.code);
        return res;
    });
    res = kefir_hashtree_free(mem, &func.labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_asmcmp_amd64_free(mem, &func.code);
        return res;
    });
    REQUIRE_OK(kefir_asmcmp_amd64_free(mem, &func.code));

    return KEFIR_OK;
}
