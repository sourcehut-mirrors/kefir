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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_mark_clobbers(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

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

        REQUIRE_OK(kefir_hashtreeset_add(mem, &context->dirty_registers, (kefir_hashtreeset_entry_t) dirty_reg));
    }
    return KEFIR_OK;
}
