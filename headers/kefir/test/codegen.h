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

#ifndef KEFIR_TEST_CODEGEN_H_
#define KEFIR_TEST_CODEGEN_H_

#include "kefir/ast-translator/environment.h"
#include "kefir/codegen/codegen.h"
#include "kefir/codegen/naive-system-v-amd64/codegen.h"
#include "kefir/codegen/amd64/codegen.h"

typedef struct kefir_test_codegen {
    struct kefir_codegen iface;

    union {
        struct kefir_codegen_amd64 new_codegen;
        struct kefir_codegen_naive_amd64 naive_codegen;
    };

    const struct kefir_codegen_configuration *config;
} kefir_test_codegen_t;

kefir_result_t kefir_test_codegen_init(struct kefir_mem *, struct kefir_test_codegen *, FILE *,
                                       const struct kefir_codegen_configuration *);

#endif
