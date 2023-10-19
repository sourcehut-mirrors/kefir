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

#include "kefir/codegen/naive-system-v-amd64/builtins.h"

const struct kefir_codegen_naive_amd64_sysv_builtin_type *KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[] = {
    /* KEFIR_IR_TYPE_BUILTIN_VARARG */ &KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILIN_VARARG_TYPE};
