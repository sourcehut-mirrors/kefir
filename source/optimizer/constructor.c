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

#include "kefir/optimizer/constructor.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                const struct kefir_irblock *ir_block,
                                                struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block"));
    REQUIRE(code != NULL && kefir_opt_code_container_is_empty(code),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid empty optimizer code container"));

    // TODO Implement translation from IR
    return KEFIR_OK;
}
