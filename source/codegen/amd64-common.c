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

#include "kefir/codegen/codegen.h"
#include "kefir/codegen/amd64-common.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

kefir_result_t kefir_codegen_match_syntax(const char *syntax_descr, kefir_asm_amd64_xasmgen_syntax_t *syntax) {
    if (syntax_descr == NULL) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_ATT;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_PREFIX) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_NOPREFIX) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_NOPREFIX;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_ATT) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_ATT;
    } else {
        return KEFIR_SET_ERRORF(KEFIR_INVALID_PARAMETER, "Unknown amd64 assembly syntax descriptor '%s'", syntax_descr);
    }
    return KEFIR_OK;
}
