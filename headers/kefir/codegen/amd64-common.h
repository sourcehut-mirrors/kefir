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

#ifndef KEFIR_CODEGEN_AMD64_COMMON_H_
#define KEFIR_CODEGEN_AMD64_COMMON_H_

#define KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_PREFIX "x86_64-intel_prefix"
#define KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_NOPREFIX "x86_64-intel_noprefix"
#define KEFIR_CODEGEN_SYNTAX_X86_64_ATT "x86_64-att"

#include "kefir/codegen/codegen.h"
#include "kefir/target/asm/amd64/xasmgen.h"

kefir_result_t kefir_codegen_match_syntax(const char *, kefir_asm_amd64_xasmgen_syntax_t *);

#endif
