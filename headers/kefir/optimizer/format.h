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

#ifndef KEFIR_OPTIMIZER_FORMAT_H_
#define KEFIR_OPTIMIZER_FORMAT_H_

#include "kefir/optimizer/module.h"
#include "kefir/util/json.h"

kefir_result_t kefir_opt_code_format(struct kefir_json_output *, const struct kefir_opt_code_container *);
kefir_result_t kefir_opt_module_format(struct kefir_json_output *, const struct kefir_opt_module *);

#endif
