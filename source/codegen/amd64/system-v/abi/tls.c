/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/system-v/abi/tls.h"
#include "kefir/codegen/amd64/system-v/abi.h"
#include "kefir/codegen/amd64/tls.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_amd64_sysv_thread_local_reference(struct kefir_codegen_amd64 *codegen, const char *identifier,
                                                       kefir_bool_t local) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid TLS identifier"));

    if (!codegen->config->emulated_tls) {
        if (local) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                kefir_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_THREAD_LOCAL,
                                                           identifier)),
                    0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                kefir_amd64_xasmgen_operand_segment(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SEGMENT_FS,
                    kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 0))));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                kefir_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                    kefir_amd64_xasmgen_operand_segment(
                        &codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SEGMENT_FS,
                        kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[2], 0)))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                kefir_amd64_xasmgen_operand_rip_indirection(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_THREAD_LOCAL_GOT,
                                                       identifier))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&codegen->xasmgen,
                                                     kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                     kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    } else {
        if (local) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
                kefir_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[1],
                                                      kefir_amd64_xasmgen_helpers_format(
                                                          &codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_V, identifier)),
                    0)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
                kefir_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                    kefir_amd64_xasmgen_operand_rip_indirection(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_GOT,
                                                           identifier)))));
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
            &codegen->xasmgen,
            kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_EMUTLS_GET_ADDR)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    }

    return KEFIR_OK;
}
