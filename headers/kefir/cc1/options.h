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

#ifndef KEFIR_CC1_OPTIONS_H_
#define KEFIR_CC1_OPTIONS_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/compiler/configuration.h"
#include <time.h>

typedef enum kefir_cli_command {
    KEFIR_CLI_COMMAND_RUN,
    KEFIR_CLI_COMMAND_HELP,
    KEFIR_CLI_COMMAND_VERSION
} kefir_cli_command_t;

kefir_result_t kefir_cli_parse_runner_configuration(struct kefir_mem *, struct kefir_compiler_runner_configuration *,
                                                    char *const *, kefir_size_t, kefir_cli_command_t *);

#endif
