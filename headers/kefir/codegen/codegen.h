/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_CODEGEN_H_
#define KEFIR_CODEGEN_CODEGEN_H_

#include <stdio.h>
#include "kefir/core/mem.h"
#include "kefir/core/basic-types.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/analysis.h"

typedef struct kefir_codegen_configuration {
    kefir_bool_t emulated_tls;
    kefir_bool_t position_independent_code;
    kefir_bool_t omit_frame_pointer;
    const char *syntax;
    const char *print_details;
    const char *pipeline_spec;
} kefir_codegen_configuration_t;

typedef struct kefir_codegen {
    kefir_result_t (*translate_optimized)(struct kefir_mem *, struct kefir_codegen *, struct kefir_opt_module *,
                                          struct kefir_opt_module_analysis *);
    kefir_result_t (*close)(struct kefir_mem *, struct kefir_codegen *);

    void *data;
    void *self;
} kefir_codegen_t;

kefir_result_t kefir_codegen_translate_ir(struct kefir_mem *, struct kefir_codegen *, struct kefir_ir_module *);

#define KEFIR_CODEGEN_TRANSLATE(mem, codegen, module) (kefir_codegen_translate_ir((mem), (codegen), (module)))
#define KEFIR_CODEGEN_TRANSLATE_OPTIMIZED(mem, codegen, module, analysis) \
    ((codegen)->translate_optimized((mem), (codegen), (module), (analysis)))
#define KEFIR_CODEGEN_CLOSE(mem, codegen) ((codegen)->close((mem), (codegen)))

extern const struct kefir_codegen_configuration KefirCodegenDefaultConfiguration;

#endif
