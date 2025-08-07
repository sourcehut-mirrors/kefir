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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/configuration.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t lowering_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                     struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                     const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(config != NULL && config->target_lowering != NULL, KEFIR_OK);

    REQUIRE_OK(config->target_lowering->lower(mem, module, func, config->target_lowering->payload));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassLowering = {
    .name = "lowering", .apply = lowering_apply, .payload = NULL};
