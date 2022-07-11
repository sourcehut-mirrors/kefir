/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_DRIVER_TARGET_CONFIGURATION_H_
#define KEFIR_DRIVER_TARGET_CONFIGURATION_H_

#include "kefir/core/mem.h"
#include "kefir/driver/externals.h"
#include "kefir/driver/configuration.h"
#include "kefir/compiler/configuration.h"
#include "kefir/driver/target.h"

kefir_result_t kefir_driver_apply_target_profile_configuration(struct kefir_compiler_runner_configuration *,
                                                               const struct kefir_driver_target *);

kefir_result_t kefir_driver_apply_target_configuration(struct kefir_mem *,
                                                       const struct kefir_driver_external_resources *,
                                                       struct kefir_compiler_runner_configuration *,
                                                       struct kefir_driver_assembler_configuration *,
                                                       struct kefir_driver_linker_configuration *,
                                                       const struct kefir_driver_target *);

#endif
