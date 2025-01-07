/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include <stdlib.h>
#include <stdio.h>
#include "kefir/ir/function.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/module.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/test/codegen.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t id1, id2, id3;
    struct kefir_ir_inline_assembly *inline_asm1 =
        kefir_ir_module_new_inline_assembly(mem, &module, "test1:\nadd rdi, rsi\nmov rax, rdi\nret", &id1);
    struct kefir_ir_inline_assembly *inline_asm2 =
        kefir_ir_module_new_inline_assembly(mem, &module, "xor rax, rax\nmov [rax], rax", &id2);
    struct kefir_ir_inline_assembly *inline_asm3 =
        kefir_ir_module_new_inline_assembly(mem, &module, "test3:\nxor rax, rax\nmov [rax], rax", &id3);

    REQUIRE(inline_asm1 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(inline_asm2 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(inline_asm3 != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_ir_module_inline_assembly_global(mem, &module, id1));
    REQUIRE_OK(kefir_ir_module_inline_assembly_global(mem, &module, id3));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}
