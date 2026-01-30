/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_ASMCMP_TRANSFORM_H_
#define KEFIR_CODEGEN_ASMCMP_TRANSFORM_H_

#include "kefir/codegen/asmcmp/context.h"

kefir_result_t kefir_asmcmp_compact_labels(struct kefir_mem *, struct kefir_asmcmp_context *);
kefir_result_t kefir_asmcmp_drop_virtual_instructions(struct kefir_mem *, struct kefir_asmcmp_context *);

#endif
