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

#ifndef KEFIR_CODEGEN_ASMCMP_PIPELINE_H_
#define KEFIR_CODEGEN_ASMCMP_PIPELINE_H_

#include "kefir/codegen/asmcmp/context.h"

typedef enum kefir_asmcmp_pipeline_pass_type {
    KEFIR_ASMCMP_PIPELINE_PASS_VIRTUAL,
    KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL,
    KEFIR_ASMCMP_PIPELINE_PASS_BOTH
} kefir_asmcmp_pipeline_pass_type_t;

typedef struct kefir_asmcmp_pipeline_pass {
    const char *name;
    kefir_asmcmp_pipeline_pass_type_t type;
    kefir_result_t (*apply)(struct kefir_mem *, struct kefir_asmcmp_context *,
                            const struct kefir_asmcmp_pipeline_pass *);
    void *payload;
} kefir_asmcmp_pipeline_pass_t;

typedef struct kefir_asmcmp_pipeline {
    struct kefir_list pipeline;
} kefir_asmcmp_pipeline_t;

kefir_result_t kefir_asmcmp_pipeline_pass_resolve(const char *, const struct kefir_asmcmp_pipeline_pass **);

kefir_result_t kefir_asmcmp_pipeline_init(struct kefir_asmcmp_pipeline *);
kefir_result_t kefir_asmcmp_pipeline_free(struct kefir_mem *, struct kefir_asmcmp_pipeline *);
kefir_result_t kefir_asmcmp_pipeline_add(struct kefir_mem *, struct kefir_asmcmp_pipeline *,
                                         const struct kefir_asmcmp_pipeline_pass *);
kefir_result_t kefir_asmcmp_pipeline_apply(struct kefir_mem *, const struct kefir_asmcmp_pipeline *,
                                           kefir_asmcmp_pipeline_pass_type_t, struct kefir_asmcmp_context *);

#ifdef KEFIR_CODEGEN_ASMCMP_PIPELINE_INTERNAL
// clang-format off
#define KEFIR_CODEGEN_ASMCMP_PIPELINE_PASSES(_pass, _separator) \
    _pass(Noop) _separator \
    _pass(Amd64Peephole) _separator \
    _pass(Amd64DropVirtual) _separator \
    _pass(Amd64PropagateJump) _separator \
    _pass(Amd64EliminateLabel)
// clang-format on

#define DECL_PASS(_id) extern const struct kefir_asmcmp_pipeline_pass KefirAsmcmp##_id##Pass
KEFIR_CODEGEN_ASMCMP_PIPELINE_PASSES(DECL_PASS, ;);
#undef DECL_PASS
#endif

#endif
