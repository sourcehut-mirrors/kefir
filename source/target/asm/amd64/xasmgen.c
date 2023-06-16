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

#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

kefir_result_t kefir_asm_amd64_xasmgen_register8(kefir_asm_amd64_xasmgen_register_t src,
                                                 kefir_asm_amd64_xasmgen_register_t *dst) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    switch (src) {
        case KEFIR_AMD64_XASMGEN_REGISTER_AL:
        case KEFIR_AMD64_XASMGEN_REGISTER_AX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_AL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_BL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_CL:
        case KEFIR_AMD64_XASMGEN_REGISTER_CX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ECX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_CL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_DL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SI:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_SIL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DI:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_DIL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SP:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_SPL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BP:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_BPL;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R8B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R8B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R9B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R9B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R10B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R10B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R11B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R11B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R12B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R12B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R13B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R13B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R14B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R14B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R15B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R15B;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot obtain 8-bit variant of XMM register");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asm_amd64_xasmgen_register16(kefir_asm_amd64_xasmgen_register_t src,
                                                  kefir_asm_amd64_xasmgen_register_t *dst) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    switch (src) {
        case KEFIR_AMD64_XASMGEN_REGISTER_AL:
        case KEFIR_AMD64_XASMGEN_REGISTER_AX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_AX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_BX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_CL:
        case KEFIR_AMD64_XASMGEN_REGISTER_CX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ECX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_CX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_DX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SI:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_SI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DI:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_DI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SP:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_SP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BP:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_BP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R8B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R8W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R9B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R9W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R10B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R10W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R11B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R11W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R12B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R12W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R13B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R13W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R14B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R14W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R15B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R15W;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot obtain 16-bit variant of XMM register");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asm_amd64_xasmgen_register32(kefir_asm_amd64_xasmgen_register_t src,
                                                  kefir_asm_amd64_xasmgen_register_t *dst) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    switch (src) {
        case KEFIR_AMD64_XASMGEN_REGISTER_AL:
        case KEFIR_AMD64_XASMGEN_REGISTER_AX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_EAX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_EBX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_CL:
        case KEFIR_AMD64_XASMGEN_REGISTER_CX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ECX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_ECX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_EDX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SI:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_ESI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DI:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_EDI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SP:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_ESP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BP:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_EBP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R8B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R8D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R9B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R9D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R10B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R10D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R11B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R11D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R12B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R12D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R13B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R13D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R14B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R14D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R15B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R15D;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot obtain 32-bit variant of XMM register");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asm_amd64_xasmgen_register64(kefir_asm_amd64_xasmgen_register_t src,
                                                  kefir_asm_amd64_xasmgen_register_t *dst) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    switch (src) {
        case KEFIR_AMD64_XASMGEN_REGISTER_AL:
        case KEFIR_AMD64_XASMGEN_REGISTER_AX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RBX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_CL:
        case KEFIR_AMD64_XASMGEN_REGISTER_CX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ECX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RCX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SI:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_DIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DI:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_SPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SP:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RSP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_BPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BP:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_RBP;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R8B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R8;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R9B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R9;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R10B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R10;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R11B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R11;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R12B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R12;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R13B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R13;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R14B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R14;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_R15B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            *dst = KEFIR_AMD64_XASMGEN_REGISTER_R15;
            break;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot obtain 64-bit variant of XMM register");
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_asm_amd64_xasmgen_register_is_floating_point(kefir_asm_amd64_xasmgen_register_t reg) {
    switch (reg) {
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return true;

        default:
            return false;
    }
}

kefir_bool_t kefir_asm_amd64_xasmgen_register_is_wide(kefir_asm_amd64_xasmgen_register_t reg, kefir_size_t width) {
    switch (reg) {
        case KEFIR_AMD64_XASMGEN_REGISTER_AL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BL:
        case KEFIR_AMD64_XASMGEN_REGISTER_CL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_DIL:
        case KEFIR_AMD64_XASMGEN_REGISTER_SPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_BPL:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14B:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15B:
            return width == 8;

        case KEFIR_AMD64_XASMGEN_REGISTER_AX:
        case KEFIR_AMD64_XASMGEN_REGISTER_BX:
        case KEFIR_AMD64_XASMGEN_REGISTER_CX:
        case KEFIR_AMD64_XASMGEN_REGISTER_DX:
        case KEFIR_AMD64_XASMGEN_REGISTER_SI:
        case KEFIR_AMD64_XASMGEN_REGISTER_DI:
        case KEFIR_AMD64_XASMGEN_REGISTER_SP:
        case KEFIR_AMD64_XASMGEN_REGISTER_BP:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14W:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15W:
            return width == 16;

        case KEFIR_AMD64_XASMGEN_REGISTER_EAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ECX:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESI:
        case KEFIR_AMD64_XASMGEN_REGISTER_EDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_ESP:
        case KEFIR_AMD64_XASMGEN_REGISTER_EBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14D:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15D:
            return width == 32;

        case KEFIR_AMD64_XASMGEN_REGISTER_RAX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RCX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDX:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RDI:
        case KEFIR_AMD64_XASMGEN_REGISTER_RSP:
        case KEFIR_AMD64_XASMGEN_REGISTER_RBP:
        case KEFIR_AMD64_XASMGEN_REGISTER_R8:
        case KEFIR_AMD64_XASMGEN_REGISTER_R9:
        case KEFIR_AMD64_XASMGEN_REGISTER_R10:
        case KEFIR_AMD64_XASMGEN_REGISTER_R11:
        case KEFIR_AMD64_XASMGEN_REGISTER_R12:
        case KEFIR_AMD64_XASMGEN_REGISTER_R13:
        case KEFIR_AMD64_XASMGEN_REGISTER_R14:
        case KEFIR_AMD64_XASMGEN_REGISTER_R15:
            return width == 64;

        case KEFIR_AMD64_XASMGEN_REGISTER_XMM0:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM1:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM2:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM3:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM4:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM5:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM6:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM7:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM8:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM9:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM10:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM11:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM12:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM13:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM14:
        case KEFIR_AMD64_XASMGEN_REGISTER_XMM15:
            return width == 128;

        default:
            return false;
    }
}

kefir_result_t kefir_asm_amd64_xasmgen_register_widest(kefir_asm_amd64_xasmgen_register_t src,
                                                       kefir_asm_amd64_xasmgen_register_t *dst) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    if (kefir_asm_amd64_xasmgen_register_is_floating_point(src)) {
        *dst = src;
    } else {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(src, dst));
    }
    return KEFIR_OK;
}

static const char *register_literals[] = {
    [KEFIR_AMD64_XASMGEN_REGISTER_AL] = "al",       [KEFIR_AMD64_XASMGEN_REGISTER_BL] = "bl",
    [KEFIR_AMD64_XASMGEN_REGISTER_CL] = "cl",       [KEFIR_AMD64_XASMGEN_REGISTER_DL] = "dl",
    [KEFIR_AMD64_XASMGEN_REGISTER_SIL] = "sil",     [KEFIR_AMD64_XASMGEN_REGISTER_DIL] = "dil",
    [KEFIR_AMD64_XASMGEN_REGISTER_SPL] = "spl",     [KEFIR_AMD64_XASMGEN_REGISTER_BPL] = "bpl",
    [KEFIR_AMD64_XASMGEN_REGISTER_R8B] = "r8b",     [KEFIR_AMD64_XASMGEN_REGISTER_R9B] = "r9b",
    [KEFIR_AMD64_XASMGEN_REGISTER_R10B] = "r10b",   [KEFIR_AMD64_XASMGEN_REGISTER_R11B] = "r11b",
    [KEFIR_AMD64_XASMGEN_REGISTER_R12B] = "r12b",   [KEFIR_AMD64_XASMGEN_REGISTER_R13B] = "r13b",
    [KEFIR_AMD64_XASMGEN_REGISTER_R14B] = "r14b",   [KEFIR_AMD64_XASMGEN_REGISTER_R15B] = "r15b",
    [KEFIR_AMD64_XASMGEN_REGISTER_AX] = "ax",       [KEFIR_AMD64_XASMGEN_REGISTER_BX] = "bx",
    [KEFIR_AMD64_XASMGEN_REGISTER_CX] = "cx",       [KEFIR_AMD64_XASMGEN_REGISTER_DX] = "dx",
    [KEFIR_AMD64_XASMGEN_REGISTER_SI] = "si",       [KEFIR_AMD64_XASMGEN_REGISTER_DI] = "di",
    [KEFIR_AMD64_XASMGEN_REGISTER_SP] = "sp",       [KEFIR_AMD64_XASMGEN_REGISTER_BP] = "bp",
    [KEFIR_AMD64_XASMGEN_REGISTER_R8W] = "r8w",     [KEFIR_AMD64_XASMGEN_REGISTER_R9W] = "r9w",
    [KEFIR_AMD64_XASMGEN_REGISTER_R10W] = "r10w",   [KEFIR_AMD64_XASMGEN_REGISTER_R11W] = "r11w",
    [KEFIR_AMD64_XASMGEN_REGISTER_R12W] = "r12w",   [KEFIR_AMD64_XASMGEN_REGISTER_R13W] = "r13w",
    [KEFIR_AMD64_XASMGEN_REGISTER_R14W] = "r14w",   [KEFIR_AMD64_XASMGEN_REGISTER_R15W] = "r15w",
    [KEFIR_AMD64_XASMGEN_REGISTER_EAX] = "eax",     [KEFIR_AMD64_XASMGEN_REGISTER_EBX] = "ebx",
    [KEFIR_AMD64_XASMGEN_REGISTER_ECX] = "ecx",     [KEFIR_AMD64_XASMGEN_REGISTER_EDX] = "edx",
    [KEFIR_AMD64_XASMGEN_REGISTER_ESI] = "esi",     [KEFIR_AMD64_XASMGEN_REGISTER_EDI] = "edi",
    [KEFIR_AMD64_XASMGEN_REGISTER_ESP] = "esp",     [KEFIR_AMD64_XASMGEN_REGISTER_EBP] = "ebp",
    [KEFIR_AMD64_XASMGEN_REGISTER_R8D] = "r8d",     [KEFIR_AMD64_XASMGEN_REGISTER_R9D] = "r9d",
    [KEFIR_AMD64_XASMGEN_REGISTER_R10D] = "r10d",   [KEFIR_AMD64_XASMGEN_REGISTER_R11D] = "r11d",
    [KEFIR_AMD64_XASMGEN_REGISTER_R12D] = "r12d",   [KEFIR_AMD64_XASMGEN_REGISTER_R13D] = "r13d",
    [KEFIR_AMD64_XASMGEN_REGISTER_R14D] = "r14d",   [KEFIR_AMD64_XASMGEN_REGISTER_R15D] = "r15d",
    [KEFIR_AMD64_XASMGEN_REGISTER_RAX] = "rax",     [KEFIR_AMD64_XASMGEN_REGISTER_RBX] = "rbx",
    [KEFIR_AMD64_XASMGEN_REGISTER_RCX] = "rcx",     [KEFIR_AMD64_XASMGEN_REGISTER_RDX] = "rdx",
    [KEFIR_AMD64_XASMGEN_REGISTER_RSI] = "rsi",     [KEFIR_AMD64_XASMGEN_REGISTER_RDI] = "rdi",
    [KEFIR_AMD64_XASMGEN_REGISTER_RSP] = "rsp",     [KEFIR_AMD64_XASMGEN_REGISTER_RBP] = "rbp",
    [KEFIR_AMD64_XASMGEN_REGISTER_R8] = "r8",       [KEFIR_AMD64_XASMGEN_REGISTER_R9] = "r9",
    [KEFIR_AMD64_XASMGEN_REGISTER_R10] = "r10",     [KEFIR_AMD64_XASMGEN_REGISTER_R11] = "r11",
    [KEFIR_AMD64_XASMGEN_REGISTER_R12] = "r12",     [KEFIR_AMD64_XASMGEN_REGISTER_R13] = "r13",
    [KEFIR_AMD64_XASMGEN_REGISTER_R14] = "r14",     [KEFIR_AMD64_XASMGEN_REGISTER_R15] = "r15",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM0] = "xmm0",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM1] = "xmm1",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM2] = "xmm2",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM3] = "xmm3",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM4] = "xmm4",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM5] = "xmm5",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM6] = "xmm6",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM7] = "xmm7",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM8] = "xmm8",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM9] = "xmm9",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM10] = "xmm10", [KEFIR_AMD64_XASMGEN_REGISTER_XMM11] = "xmm11",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM12] = "xmm12", [KEFIR_AMD64_XASMGEN_REGISTER_XMM13] = "xmm13",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM14] = "xmm14", [KEFIR_AMD64_XASMGEN_REGISTER_XMM15] = "xmm15"};

static const kefir_size_t register_literal_count = sizeof(register_literals) / sizeof(register_literals[0]);

const char *kefir_asm_amd64_xasmgen_register_symbolic_name(kefir_asm_amd64_xasmgen_register_t reg) {
    return register_literals[reg];
}

kefir_result_t kefir_asm_amd64_xasmgen_register_from_symbolic_name(const char *symbolic,
                                                                   kefir_asm_amd64_xasmgen_register_t *reg_ptr) {
    REQUIRE(symbolic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbolic amd64 register name"));
    REQUIRE(reg_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 xasmgen register"));

    for (kefir_size_t i = 0; i < register_literal_count; i++) {
        if (strcmp(register_literals[i], symbolic) == 0) {
            *reg_ptr = (kefir_asm_amd64_xasmgen_register_t) i;
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested symbolic amd64 register name was not found");
}

struct xasmgen_payload {
    FILE *output;
    kefir_asm_amd64_xasmgen_syntax_t syntax;
};

static kefir_result_t amd64_ident(struct kefir_amd64_xasmgen *xasmgen) {
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);
    if (xasmgen->settings.enable_identation) {
        fprintf(payload->output, "    ");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_prologue(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    switch (payload->syntax) {
        case KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX:
            fprintf(payload->output, ".intel_syntax prefix\n");
            break;

        case KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_NOPREFIX:
            fprintf(payload->output, ".intel_syntax noprefix\n");
            break;

        case KEFIR_AMD64_XASMGEN_SYNTAX_ATT:
            fprintf(payload->output, ".att_syntax\n");
    }

    fprintf(payload->output, "%s\n\n", ".section .note.GNU-stack,\"\",%progbits");
    return KEFIR_OK;
}

static kefir_result_t amd64_close(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fflush(payload->output);
    payload->output = NULL;
    KEFIR_FREE(mem, payload);
    xasmgen->payload = NULL;
    return KEFIR_OK;
}

static kefir_result_t amd64_newline(struct kefir_amd64_xasmgen *xasmgen, unsigned int count) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    while (count--) {
        fprintf(payload->output, "\n");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_comment(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    if (xasmgen->settings.enable_comments) {
        fprintf(payload->output, "# ");
        va_list args;
        va_start(args, format);
        vfprintf(payload->output, format, args);
        va_end(args);
        fprintf(payload->output, "\n");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_label(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, ":\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_global(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".global ");
    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_external(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".extern ");
    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_section(struct kefir_amd64_xasmgen *xasmgen, const char *identifier) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".section %s\n", identifier);
    return KEFIR_OK;
}

static kefir_result_t amd64_align(struct kefir_amd64_xasmgen *xasmgen, kefir_size_t alignment) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, ".align " KEFIR_SIZE_FMT "\n", alignment);
    return KEFIR_OK;
}

static kefir_result_t amd64_string_literal(void (*print)(void *, const char *, ...), void *printarg,
                                           const char *literal, kefir_size_t length) {
    print(printarg, "\"");
    const char *end = literal + length;
    for (; literal < end; ++literal) {
        switch (*literal) {
            case U'\0':
                print(printarg, "\\000");
                break;

            case U'\"':
                print(printarg, "\\\"");
                break;

            case U'\\':
                print(printarg, "\\\\");
                break;

            case U'\a':
                print(printarg, "\\%03o", '\a');
                break;

            case U'\b':
                print(printarg, "\\b");
                break;

            case U'\t':
                print(printarg, "\\t");
                break;

            case U'\n':
                print(printarg, "\\n");
                break;

            case U'\v':
                print(printarg, "\\%03o", '\v');
                break;

            case U'\f':
                print(printarg, "\\f");
                break;

            case U'\r':
                print(printarg, "\\r");
                break;

            default:
                if (isprint(*literal)) {
                    print(printarg, "%c", *literal);
                } else {
                    print(printarg, "\\%03o", (unsigned char) *literal);
                }
                break;
        }
    }
    print(printarg, "\"");
    return KEFIR_OK;
}

static kefir_result_t amd64_symbol_arg(void (*print)(void *, const char *, ...), void *printarg, const char *symbol) {
    const char *EscapedSymbols[] = {// TODO Expand number of escaped symbols
                                    "mod"};
    for (kefir_size_t i = 0; i < sizeof(EscapedSymbols) / sizeof(EscapedSymbols[0]); i++) {
        if (strcasecmp(symbol, EscapedSymbols[i]) == 0) {
            print(printarg, "$%s", symbol);
            return KEFIR_OK;
        }
    }

    print(printarg, "%s", symbol);
    return KEFIR_OK;
}

static kefir_result_t format_pointer(void (*print)(void *, const char *, ...), void *printarg,
                                     kefir_asm_amd64_xasmgen_pointer_type_t type) {
    switch (type) {
        case KEFIR_AMD64_XASMGEN_POINTER_BYTE:
            print(printarg, "BYTE PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_WORD:
            print(printarg, "WORD PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_DWORD:
            print(printarg, "DWORD PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_QWORD:
            print(printarg, "QWORD PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_TBYTE:
            print(printarg, "TBYTE PTR ");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_format_operand_intel(void (*print)(void *, const char *, ...), void *printarg,
                                                 kefir_bool_t prefix,
                                                 const struct kefir_asm_amd64_xasmgen_operand *op) {
    switch (op->klass) {
        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE:
            print(printarg, KEFIR_INT64_FMT, op->imm);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED:
            print(printarg, KEFIR_UINT64_FMT, op->immu);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_REGISTER:
            if (prefix) {
                print(printarg, "%%%s", register_literals[op->reg]);
            } else {
                print(printarg, "%s", register_literals[op->reg]);
            }
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_LABEL:
            REQUIRE_OK(amd64_symbol_arg(print, printarg, op->label));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION:
            print(printarg, "[");
            REQUIRE_OK(amd64_format_operand_intel(print, printarg, prefix, op->indirection.base));
            if (op->indirection.displacement > 0) {
                print(printarg, " + " KEFIR_INT64_FMT, op->indirection.displacement);
            } else if (op->indirection.displacement < 0) {
                print(printarg, " - " KEFIR_INT64_FMT, -op->indirection.displacement);
            }
            print(printarg, "]");
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_OFFSET:
            REQUIRE_OK(amd64_format_operand_intel(print, printarg, prefix, op->offset.base));
            if (op->offset.offset > 0) {
                print(printarg, " + " KEFIR_INT64_FMT, op->offset.offset);
            } else if (op->offset.offset < 0) {
                print(printarg, " - " KEFIR_INT64_FMT, -op->offset.offset);
            }
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT:
            switch (op->segment.segment) {
                case KEFIR_AMD64_XASMGEN_SEGMENT_FS:
                    if (prefix) {
                        print(printarg, "%%fs:");
                    } else {
                        print(printarg, "fs:");
                    }
                    break;
            }
            REQUIRE_OK(amd64_format_operand_intel(print, printarg, prefix, op->segment.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_POINTER:
            REQUIRE_OK(format_pointer(print, printarg, op->pointer.type));
            REQUIRE_OK(amd64_format_operand_intel(print, printarg, prefix, op->pointer.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION:
            REQUIRE_OK(amd64_symbol_arg(print, printarg, op->label));
            if (prefix) {
                print(printarg, "[%%rip]");
            } else {
                print(printarg, "[rip]");
            }
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL:
            REQUIRE_OK(amd64_string_literal(print, printarg, op->string_literal.content, op->string_literal.length));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_format_unprefixed_operand_att(void (*)(void *, const char *, ...), void *,
                                                          const struct kefir_asm_amd64_xasmgen_operand *);

static kefir_result_t amd64_format_operand_att(void (*print)(void *, const char *, ...), void *printarg,
                                               const struct kefir_asm_amd64_xasmgen_operand *op) {
    switch (op->klass) {
        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE:
            print(printarg, "$" KEFIR_INT64_FMT, op->imm);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED:
            print(printarg, "$" KEFIR_UINT64_FMT, op->immu);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_REGISTER:
            print(printarg, "%%%s", register_literals[op->reg]);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_LABEL:
            REQUIRE_OK(amd64_symbol_arg(print, printarg, op->label));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION:
            if (op->indirection.displacement != 0) {
                print(printarg, KEFIR_INT64_FMT, op->indirection.displacement);
            }
            print(printarg, "(");
            REQUIRE_OK(amd64_format_operand_att(print, printarg, op->indirection.base));
            print(printarg, ")");
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_OFFSET:
            REQUIRE_OK(amd64_format_operand_att(print, printarg, op->offset.base));
            if (op->offset.offset > 0) {
                print(printarg, " + " KEFIR_INT64_FMT, op->offset.offset);
            } else if (op->offset.offset < 0) {
                print(printarg, " - " KEFIR_INT64_FMT, -op->offset.offset);
            }
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT:
            switch (op->segment.segment) {
                case KEFIR_AMD64_XASMGEN_SEGMENT_FS:
                    print(printarg, "%%fs:");
                    break;
            }
            REQUIRE_OK(amd64_format_unprefixed_operand_att(print, printarg, op->segment.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_POINTER:
            REQUIRE_OK(amd64_format_operand_att(print, printarg, op->pointer.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION:
            REQUIRE_OK(amd64_symbol_arg(print, printarg, op->label));
            print(printarg, "(%%rip)");
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL:
            REQUIRE_OK(amd64_string_literal(print, printarg, op->string_literal.content, op->string_literal.length));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_format_unprefixed_operand_att(void (*print)(void *, const char *, ...), void *printarg,
                                                          const struct kefir_asm_amd64_xasmgen_operand *op) {
    switch (op->klass) {
        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE:
            print(printarg, KEFIR_INT64_FMT, op->imm);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED:
            print(printarg, KEFIR_UINT64_FMT, op->immu);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_REGISTER:
            print(printarg, "%s", register_literals[op->reg]);
            break;

        default:
            REQUIRE_OK(amd64_format_operand_att(print, printarg, op));
            break;
    }
    return KEFIR_OK;
}

static void print_fprintf(void *arg, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf((FILE *) arg, fmt, args);
    va_end(args);
}

static kefir_result_t amd64_data(struct kefir_amd64_xasmgen *xasmgen, kefir_asm_amd64_xasmgen_data_type_t type,
                                 kefir_size_t length, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    switch (type) {
        case KEFIR_AMD64_XASMGEN_DATA_BYTE:
            fprintf(payload->output, ".byte ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_WORD:
            fprintf(payload->output, ".word ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_DOUBLE:
            fprintf(payload->output, ".long ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_QUAD:
            fprintf(payload->output, ".quad ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_ASCII:
            fprintf(payload->output, ".ascii ");
            break;
    }

    va_list args;
    va_start(args, length);
    while (length--) {
        const struct kefir_asm_amd64_xasmgen_operand *op = va_arg(args, const struct kefir_asm_amd64_xasmgen_operand *);
        if (payload->syntax != KEFIR_AMD64_XASMGEN_SYNTAX_ATT) {
            REQUIRE_OK(amd64_format_operand_intel(print_fprintf, payload->output,
                                                  payload->syntax == KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX, op));
        } else {
            REQUIRE_OK(amd64_format_unprefixed_operand_att(print_fprintf, payload->output, op));
        }
        if (length > 0) {
            fprintf(payload->output, ", ");
        }
    }
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_zerodata(struct kefir_amd64_xasmgen *xasmgen, kefir_size_t length) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, ".zero " KEFIR_SIZE_FMT "\n", length);
    return KEFIR_OK;
}

static kefir_result_t amd64_bindata(struct kefir_amd64_xasmgen *xasmgen, kefir_asm_amd64_xasmgen_data_type_t type,
                                    const void *ptr, kefir_size_t length) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid binary data"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    switch (type) {
        case KEFIR_AMD64_XASMGEN_DATA_BYTE:
            fprintf(payload->output, ".byte ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%02x", ((const kefir_uint8_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_WORD:
            fprintf(payload->output, ".word ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%04x", ((const kefir_uint16_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_DOUBLE:
            fprintf(payload->output, ".long ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%08x", ((const kefir_uint32_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_QUAD:
            fprintf(payload->output, ".quad ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%016lx", ((const kefir_uint64_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_ASCII:
            REQUIRE_OK(amd64_string_literal(print_fprintf, payload->output, ptr, length));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_inline_assembly(struct kefir_amd64_xasmgen *xasmgen, const char *text) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(text != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid inline assembly text"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "%s\n", text);
    return KEFIR_OK;
}

struct op_to_str {
    char *buffer;
    kefir_size_t length;
};

static void print_op_to_str(void *arg, const char *fmt, ...) {
    ASSIGN_DECL_CAST(struct op_to_str *, op_to_str, arg);
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(op_to_str->buffer, op_to_str->length, fmt, args);
    op_to_str->length -= len;
    op_to_str->buffer += len;
    va_end(args);
}

static kefir_result_t amd64_format_operand(struct kefir_amd64_xasmgen *xasmgen,
                                           const struct kefir_asm_amd64_xasmgen_operand *op, char *buf,
                                           kefir_size_t buflen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(buflen > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected non-zero buffer length"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    struct op_to_str op_to_str_buf = {.buffer = buf, .length = buflen};
    if (payload->syntax == KEFIR_AMD64_XASMGEN_SYNTAX_ATT) {
        REQUIRE_OK(amd64_format_operand_att(print_op_to_str, &op_to_str_buf, op));
    } else {
        REQUIRE_OK(amd64_format_operand_intel(print_op_to_str, &op_to_str_buf,
                                              payload->syntax == KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX, op));
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_format_op(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_asm_amd64_xasmgen_operand *op) {
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    if (payload->syntax == KEFIR_AMD64_XASMGEN_SYNTAX_ATT) {
        REQUIRE_OK(amd64_format_operand_att(print_fprintf, payload->output, op));
    } else {
        REQUIRE_OK(amd64_format_operand_intel(print_fprintf, payload->output,
                                              payload->syntax == KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX, op));
    }
    return KEFIR_OK;
}

#define INSTR0_INTEL(_mnemonic)                                                                                        \
    static kefir_result_t amd64_instr_intel_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen) {                         \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic "\n");                                                                     \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR1_INTEL(_mnemonic)                                                                                        \
    static kefir_result_t amd64_instr_intel_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen,                           \
                                                        const struct kefir_asm_amd64_xasmgen_operand *op1) {           \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic " ");                                                                      \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR2_INTEL(_mnemonic)                                                                                        \
    static kefir_result_t amd64_instr_intel_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen,                           \
                                                        const struct kefir_asm_amd64_xasmgen_operand *op1,             \
                                                        const struct kefir_asm_amd64_xasmgen_operand *op2) {           \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op2 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic " ");                                                                      \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op2));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR3_INTEL(_mnemonic)                                                                                        \
    static kefir_result_t amd64_instr_intel_##_mnemonic(                                                               \
        struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asm_amd64_xasmgen_operand *op1,                        \
        const struct kefir_asm_amd64_xasmgen_operand *op2, const struct kefir_asm_amd64_xasmgen_operand *op3) {        \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op2 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op3 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic " ");                                                                      \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op2));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op3));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

static kefir_result_t format_att_mnemonic_suffix_impl(FILE *output, kefir_asm_amd64_xasmgen_pointer_type_t ptr) {
    switch (ptr) {
        case KEFIR_AMD64_XASMGEN_POINTER_BYTE:
            fprintf(output, "b");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_WORD:
            fprintf(output, "w");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_DWORD:
            fprintf(output, "l");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_QWORD:
            fprintf(output, "q");
            break;

        case KEFIR_AMD64_XASMGEN_POINTER_TBYTE:
            fprintf(output, "t");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t format_att_mnemonic_suffix(struct kefir_amd64_xasmgen *xasmgen,
                                                 const struct kefir_asm_amd64_xasmgen_operand *op1,
                                                 const struct kefir_asm_amd64_xasmgen_operand *op2,
                                                 const struct kefir_asm_amd64_xasmgen_operand *op3) {
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);
    if (op1 != NULL && op1->klass == KEFIR_AMD64_XASMGEN_OPERAND_POINTER) {
        REQUIRE_OK(format_att_mnemonic_suffix_impl(payload->output, op1->pointer.type));
    } else if (op2 != NULL && op2->klass == KEFIR_AMD64_XASMGEN_OPERAND_POINTER) {
        REQUIRE_OK(format_att_mnemonic_suffix_impl(payload->output, op2->pointer.type));
    } else if (op3 != NULL && op3->klass == KEFIR_AMD64_XASMGEN_OPERAND_POINTER) {
        REQUIRE_OK(format_att_mnemonic_suffix_impl(payload->output, op3->pointer.type));
    }
    fprintf(payload->output, " ");
    return KEFIR_OK;
}

#define INSTR0_ATT(_mnemonic)                                                                                          \
    static kefir_result_t amd64_instr_att_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen) {                           \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic "\n");                                                                     \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR1_ATT(_mnemonic)                                                                                          \
    static kefir_result_t amd64_instr_att_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen,                             \
                                                      const struct kefir_asm_amd64_xasmgen_operand *op1) {             \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic);                                                                          \
        REQUIRE_OK(format_att_mnemonic_suffix(xasmgen, op1, NULL, NULL));                                              \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR1_ATT_BR(_mnemonic)                                                                                       \
    static kefir_result_t amd64_instr_att_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen,                             \
                                                      const struct kefir_asm_amd64_xasmgen_operand *op1) {             \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic);                                                                          \
        REQUIRE_OK(format_att_mnemonic_suffix(xasmgen, op1, NULL, NULL));                                              \
        if (op1->klass == KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION) {                                                   \
            fprintf(payload->output, "*");                                                                             \
        }                                                                                                              \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR2_ATT(_mnemonic)                                                                                          \
    static kefir_result_t amd64_instr_att_##_mnemonic(struct kefir_amd64_xasmgen *xasmgen,                             \
                                                      const struct kefir_asm_amd64_xasmgen_operand *op1,               \
                                                      const struct kefir_asm_amd64_xasmgen_operand *op2) {             \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op2 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic);                                                                          \
        REQUIRE_OK(format_att_mnemonic_suffix(xasmgen, op1, op2, NULL));                                               \
        REQUIRE_OK(amd64_format_op(xasmgen, op2));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR3_ATT(_mnemonic)                                                                                          \
    static kefir_result_t amd64_instr_att_##_mnemonic(                                                                 \
        struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asm_amd64_xasmgen_operand *op1,                        \
        const struct kefir_asm_amd64_xasmgen_operand *op2, const struct kefir_asm_amd64_xasmgen_operand *op3) {        \
        REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator")); \
        REQUIRE(op1 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op2 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        REQUIRE(op3 != NULL,                                                                                           \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));          \
        ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);                                         \
        REQUIRE_OK(amd64_ident(xasmgen));                                                                              \
        fprintf(payload->output, #_mnemonic);                                                                          \
        REQUIRE_OK(format_att_mnemonic_suffix(xasmgen, op1, op2, op3));                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op3));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op2));                                                                     \
        fprintf(payload->output, ", ");                                                                                \
        REQUIRE_OK(amd64_format_op(xasmgen, op1));                                                                     \
        fprintf(payload->output, "\n");                                                                                \
        return KEFIR_OK;                                                                                               \
    }

#define INSTR0(_opcode) \
    INSTR0_ATT(_opcode) \
    INSTR0_INTEL(_opcode)

#define INSTR1(_opcode) \
    INSTR1_ATT(_opcode) \
    INSTR1_INTEL(_opcode)

#define INSTR2(_opcode) \
    INSTR2_ATT(_opcode) \
    INSTR2_INTEL(_opcode)

#define INSTR3(_opcode) \
    INSTR3_ATT(_opcode) \
    INSTR3_INTEL(_opcode)

INSTR1(push)
INSTR1(pop)
INSTR2(mov)
INSTR2(movsx)
INSTR2(movzx)
INSTR2(movabs)
INSTR2(or)
INSTR2(lea)
INSTR2(movq)
INSTR1_INTEL(jmp)
INSTR1_ATT_BR(jmp)
INSTR1_INTEL(ja)
INSTR1_ATT_BR(ja)
INSTR1_INTEL(jz)
INSTR1_ATT_BR(jz)
INSTR0(ret)
INSTR1(fstcw)
INSTR1(fldcw)
INSTR1_INTEL(call)
INSTR1_ATT_BR(call)
INSTR3(pextrq)
INSTR3(pinsrq)
INSTR2(add)
INSTR2(cmp)
INSTR2(test)
INSTR1(sete)
INSTR1(setg)
INSTR1(setl)
INSTR1(seta)
INSTR1(setb)
INSTR1(setne)
INSTR2(sub)
INSTR2(imul)
INSTR1(idiv)
INSTR1(div)
INSTR2(xor)
INSTR0(cqo)
INSTR0(cld)
INSTR2(and)
INSTR2(shl)
INSTR2(shr)
INSTR2(sar)
INSTR1(not )
INSTR1(neg)
INSTR2(movd)
INSTR1(fstp)
INSTR1(fld)
INSTR2(movdqu)
INSTR0(pushfq)
INSTR0(popfq)
INSTR1(stmxcsr)
INSTR1(ldmxcsr)

#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

#undef INSTR0_ATT
#undef INSTR1_ATT
#undef INSTR1_ATT_BR
#undef INSTR2_ATT
#undef INSTR3_ATT

#undef INSTR0_INTEL
#undef INSTR1_INTEL
#undef INSTR1_INTEL
#undef INSTR2_INTEL
#undef INSTR3_INTEL

static kefir_result_t amd64_instr_intel_movsb(struct kefir_amd64_xasmgen *xasmgen, kefir_bool_t rep) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    if (rep) {
        fprintf(payload->output, "rep ");
    }

    fprintf(payload->output, "movsb\n");
    return KEFIR_OK;
}

#define amd64_instr_att_movsb amd64_instr_intel_movsb

kefir_result_t kefir_asm_amd64_xasmgen_init(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, FILE *output,
                                            kefir_asm_amd64_xasmgen_syntax_t syntax) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 xasmgen"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output FILE"));

    struct xasmgen_payload *payload = KEFIR_MALLOC(mem, sizeof(struct xasmgen_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocator amd64 xasmgen payload"));
    payload->output = output;
    payload->syntax = syntax;

    xasmgen->payload = payload;

    xasmgen->settings.enable_comments = true;
    xasmgen->settings.enable_identation = true;

    xasmgen->prologue = amd64_prologue;
    xasmgen->close = amd64_close;
    xasmgen->newline = amd64_newline;
    xasmgen->comment = amd64_comment;
    xasmgen->label = amd64_label;
    xasmgen->global = amd64_global;
    xasmgen->external = amd64_external;
    xasmgen->section = amd64_section;
    xasmgen->align = amd64_align;
    xasmgen->data = amd64_data;
    xasmgen->zerodata = amd64_zerodata;
    xasmgen->bindata = amd64_bindata;
    xasmgen->inline_assembly = amd64_inline_assembly;
    xasmgen->format_operand = amd64_format_operand;

    if (syntax != KEFIR_AMD64_XASMGEN_SYNTAX_ATT) {
        xasmgen->instr.add = amd64_instr_intel_add;
        xasmgen->instr.and = amd64_instr_intel_and;
        xasmgen->instr.call = amd64_instr_intel_call;
        xasmgen->instr.cld = amd64_instr_intel_cld;
        xasmgen->instr.cmp = amd64_instr_intel_cmp;
        xasmgen->instr.test = amd64_instr_intel_test;
        xasmgen->instr.sete = amd64_instr_intel_sete;
        xasmgen->instr.setg = amd64_instr_intel_setg;
        xasmgen->instr.setl = amd64_instr_intel_setl;
        xasmgen->instr.seta = amd64_instr_intel_seta;
        xasmgen->instr.setb = amd64_instr_intel_setb;
        xasmgen->instr.setne = amd64_instr_intel_setne;
        xasmgen->instr.fld = amd64_instr_intel_fld;
        xasmgen->instr.fstcw = amd64_instr_intel_fstcw;
        xasmgen->instr.fldcw = amd64_instr_intel_fldcw;
        xasmgen->instr.fstp = amd64_instr_intel_fstp;
        xasmgen->instr.ja = amd64_instr_intel_ja;
        xasmgen->instr.jz = amd64_instr_intel_jz;
        xasmgen->instr.jmp = amd64_instr_intel_jmp;
        xasmgen->instr.lea = amd64_instr_intel_lea;
        xasmgen->instr.mov = amd64_instr_intel_mov;
        xasmgen->instr.movsx = amd64_instr_intel_movsx;
        xasmgen->instr.movzx = amd64_instr_intel_movzx;
        xasmgen->instr.movabs = amd64_instr_intel_movabs;
        xasmgen->instr.movd = amd64_instr_intel_movd;
        xasmgen->instr.movq = amd64_instr_intel_movq;
        xasmgen->instr.movsb = amd64_instr_intel_movsb;
        xasmgen->instr.or = amd64_instr_intel_or;
        xasmgen->instr.pextrq = amd64_instr_intel_pextrq;
        xasmgen->instr.pinsrq = amd64_instr_intel_pinsrq;
        xasmgen->instr.pop = amd64_instr_intel_pop;
        xasmgen->instr.push = amd64_instr_intel_push;
        xasmgen->instr.ret = amd64_instr_intel_ret;
        xasmgen->instr.shl = amd64_instr_intel_shl;
        xasmgen->instr.shr = amd64_instr_intel_shr;
        xasmgen->instr.sar = amd64_instr_intel_sar;
        xasmgen->instr.not = amd64_instr_intel_not;
        xasmgen->instr.neg = amd64_instr_intel_neg;
        xasmgen->instr.sub = amd64_instr_intel_sub;
        xasmgen->instr.imul = amd64_instr_intel_imul;
        xasmgen->instr.idiv = amd64_instr_intel_idiv;
        xasmgen->instr.div = amd64_instr_intel_div;
        xasmgen->instr.xor = amd64_instr_intel_xor;
        xasmgen->instr.cqo = amd64_instr_intel_cqo;
        xasmgen->instr.movdqu = amd64_instr_intel_movdqu;
        xasmgen->instr.pushfq = amd64_instr_intel_pushfq;
        xasmgen->instr.popfq = amd64_instr_intel_popfq;
        xasmgen->instr.stmxcsr = amd64_instr_intel_stmxcsr;
        xasmgen->instr.ldmxcsr = amd64_instr_intel_ldmxcsr;
    } else {
        xasmgen->instr.add = amd64_instr_att_add;
        xasmgen->instr.and = amd64_instr_att_and;
        xasmgen->instr.call = amd64_instr_att_call;
        xasmgen->instr.cld = amd64_instr_att_cld;
        xasmgen->instr.cmp = amd64_instr_att_cmp;
        xasmgen->instr.test = amd64_instr_att_test;
        xasmgen->instr.sete = amd64_instr_att_sete;
        xasmgen->instr.setg = amd64_instr_att_setg;
        xasmgen->instr.setl = amd64_instr_att_setl;
        xasmgen->instr.seta = amd64_instr_att_seta;
        xasmgen->instr.setb = amd64_instr_att_setb;
        xasmgen->instr.setne = amd64_instr_att_setne;
        xasmgen->instr.fld = amd64_instr_att_fld;
        xasmgen->instr.fstcw = amd64_instr_att_fstcw;
        xasmgen->instr.fldcw = amd64_instr_att_fldcw;
        xasmgen->instr.fstp = amd64_instr_att_fstp;
        xasmgen->instr.ja = amd64_instr_att_ja;
        xasmgen->instr.jz = amd64_instr_att_jz;
        xasmgen->instr.jmp = amd64_instr_att_jmp;
        xasmgen->instr.lea = amd64_instr_att_lea;
        xasmgen->instr.mov = amd64_instr_att_mov;
        xasmgen->instr.movsx = amd64_instr_att_movsx;
        xasmgen->instr.movzx = amd64_instr_att_movzx;
        xasmgen->instr.movabs = amd64_instr_att_movabs;
        xasmgen->instr.movd = amd64_instr_att_movd;
        xasmgen->instr.movq = amd64_instr_att_movq;
        xasmgen->instr.movsb = amd64_instr_att_movsb;
        xasmgen->instr.or = amd64_instr_att_or;
        xasmgen->instr.pextrq = amd64_instr_att_pextrq;
        xasmgen->instr.pinsrq = amd64_instr_att_pinsrq;
        xasmgen->instr.pop = amd64_instr_att_pop;
        xasmgen->instr.push = amd64_instr_att_push;
        xasmgen->instr.ret = amd64_instr_att_ret;
        xasmgen->instr.shl = amd64_instr_att_shl;
        xasmgen->instr.shr = amd64_instr_att_shr;
        xasmgen->instr.sar = amd64_instr_att_sar;
        xasmgen->instr.not = amd64_instr_att_not;
        xasmgen->instr.neg = amd64_instr_att_neg;
        xasmgen->instr.sub = amd64_instr_att_sub;
        xasmgen->instr.imul = amd64_instr_att_imul;
        xasmgen->instr.idiv = amd64_instr_att_idiv;
        xasmgen->instr.div = amd64_instr_att_div;
        xasmgen->instr.xor = amd64_instr_att_xor;
        xasmgen->instr.cqo = amd64_instr_att_cqo;
        xasmgen->instr.movdqu = amd64_instr_att_movdqu;
        xasmgen->instr.pushfq = amd64_instr_att_pushfq;
        xasmgen->instr.popfq = amd64_instr_att_popfq;
        xasmgen->instr.stmxcsr = amd64_instr_att_stmxcsr;
        xasmgen->instr.ldmxcsr = amd64_instr_att_ldmxcsr;
    }
    return KEFIR_OK;
}

const struct kefir_asm_amd64_xasmgen_operand operand_regs[] = {
#define REG(x) [x] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER, .reg = x}
    REG(KEFIR_AMD64_XASMGEN_REGISTER_AL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_BL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_CL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_DL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_SIL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_DIL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_SPL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_BPL),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R8B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R9B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R10B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R11B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R12B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R13B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R14B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R15B),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_AX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_BX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_CX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_DX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_SI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_DI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_SP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_BP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R8W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R9W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R10W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R11W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R12W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R13W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R14W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R15W),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_EBX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_ECX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_EDX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_ESI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_EDI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_ESP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_EBP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R8D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R9D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R10D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R11D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R12D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R13D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R14D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R15D),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RDX),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R8),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R9),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R10),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R11),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R12),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R13),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R14),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_R15),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM0),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM1),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM2),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM3),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM4),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM5),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM6),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM7),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM8),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM9),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM10),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM11),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM12),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM13),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM14),
    REG(KEFIR_AMD64_XASMGEN_REGISTER_XMM15)
#undef REG
};

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_reg(
    kefir_asm_amd64_xasmgen_register_t reg) {
    return &operand_regs[reg];
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_imm(
    struct kefir_asm_amd64_xasmgen_operand *op, kefir_int64_t imm) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE;
    op->imm = imm;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_immu(
    struct kefir_asm_amd64_xasmgen_operand *op, kefir_uint64_t immu) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED;
    op->immu = immu;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_label(
    struct kefir_asm_amd64_xasmgen_operand *op, const char *label) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_LABEL;
    op->label = label;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_indirect(
    struct kefir_asm_amd64_xasmgen_operand *op, const struct kefir_asm_amd64_xasmgen_operand *base,
    kefir_int64_t disp) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION;
    op->indirection.base = base;
    op->indirection.displacement = disp;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_offset(
    struct kefir_asm_amd64_xasmgen_operand *op, const struct kefir_asm_amd64_xasmgen_operand *base,
    kefir_int64_t offset) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_OFFSET;
    op->offset.base = base;
    op->offset.offset = offset;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_segment(
    struct kefir_asm_amd64_xasmgen_operand *op, kefir_asm_amd64_xasmgen_segment_register_t segment,
    const struct kefir_asm_amd64_xasmgen_operand *base) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT;
    op->segment.segment = segment;
    op->segment.base = base;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_pointer(
    struct kefir_asm_amd64_xasmgen_operand *op, kefir_asm_amd64_xasmgen_pointer_type_t type,
    const struct kefir_asm_amd64_xasmgen_operand *base) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_POINTER;
    op->pointer.type = type;
    op->pointer.base = base;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_rip_indirection(
    struct kefir_asm_amd64_xasmgen_operand *op, const char *identifier) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(identifier != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION;
    op->label = identifier;
    return op;
}

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_string_literal(
    struct kefir_asm_amd64_xasmgen_operand *op, const char *content, kefir_size_t length) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(content != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL;
    op->string_literal.content = content;
    op->string_literal.length = length;
    return op;
}

const char *kefir_asm_amd64_xasmgen_helpers_format(struct kefir_asm_amd64_xasmgen_helpers *helpers, const char *fmt,
                                                   ...) {
    REQUIRE(helpers != NULL, NULL);

    va_list args;
    va_start(args, fmt);
    vsnprintf(helpers->buffer, KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH - 1, fmt, args);
    va_end(args);
    return helpers->buffer;
}
