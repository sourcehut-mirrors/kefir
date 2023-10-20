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

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/asm/amd64/xasmgen.h"

static kefir_result_t opcode_mnemonic(kefir_asmcmp_instruction_opcode_t opcode, const char **mnemonic_ptr,
                                      void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to opcode mnemonic"));

    switch (opcode) {
#define CASE(_opcode, _xasmgen, _argtp)      \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        *mnemonic_ptr = #_opcode;            \
        break;

        KEFIR_ASMCMP_AMD64_OPCODES(CASE, )
#undef CASE

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t register_mnemonic(kefir_asmcmp_register_t reg, const char **mnemonic_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to register mnemonic"));

    *mnemonic_ptr = kefir_asm_amd64_xasmgen_register_symbolic_name((kefir_asm_amd64_xasmgen_register_t) reg);
    REQUIRE(mnemonic_ptr != NULL, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unknown amd64 register"));
    return KEFIR_OK;
}

static const struct kefir_asmcmp_context_class AMD64_KLASS = {.opcode_mnemonic = opcode_mnemonic,
                                                              .register_mnemonic = register_mnemonic};

kefir_result_t kefir_asmcmp_amd64_init(const char *function_name, struct kefir_asmcmp_amd64 *target) {
    REQUIRE(function_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function name"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen amd64 target"));

    REQUIRE_OK(kefir_asmcmp_context_init(&AMD64_KLASS, target, &target->context));
    target->function_name = function_name;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_free(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));

    REQUIRE_OK(kefir_asmcmp_context_free(mem, &target->context));
    return KEFIR_OK;
}

#define DEF_OPCODE_arg0(_opcode)                                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,        \
                                                kefir_asmcmp_instruction_index_t after_index,                    \
                                                kefir_asmcmp_instruction_index_t *idx_ptr) {                     \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));       \
        REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target")); \
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(                                                      \
            mem, &target->context, after_index,                                                                  \
            &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode),               \
                                                     .args[0].type = KEFIR_ASMCMP_VALUE_NONE,                    \
                                                     .args[1].type = KEFIR_ASMCMP_VALUE_NONE,                    \
                                                     .args[2].type = KEFIR_ASMCMP_VALUE_NONE},                   \
            idx_ptr));                                                                                           \
        return KEFIR_OK;                                                                                         \
    }
#define DEF_OPCODE_arg2(_opcode)                                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(                                                                 \
        struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target, kefir_asmcmp_instruction_index_t after_index,  \
        const struct kefir_asmcmp_value *arg1, const struct kefir_asmcmp_value *arg2,                            \
        kefir_asmcmp_instruction_index_t *idx_ptr) {                                                             \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));       \
        REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target")); \
        REQUIRE(arg1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 value"));    \
        REQUIRE(arg2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 value"));    \
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(                                                      \
            mem, &target->context, after_index,                                                                  \
            &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode),               \
                                                     .args[0] = *arg1,                                           \
                                                     .args[1] = *arg2,                                           \
                                                     .args[2].type = KEFIR_ASMCMP_VALUE_NONE},                   \
            idx_ptr));                                                                                           \
        return KEFIR_OK;                                                                                         \
    }
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) DEF_OPCODE_##_argtp(_opcode)

KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, )

#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2
#undef DEF_OPCODE

#define LABEL_FMT "_kefir_func_%s_label%" KEFIR_ID_FMT

static kefir_result_t generate_instr(struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asmcmp_amd64 *target,
                                     kefir_asmcmp_instruction_index_t index) {
    const struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, index, &instr));
    for (kefir_asmcmp_label_index_t label = kefir_asmcmp_context_instr_label_head(&target->context, index);
         label != KEFIR_ASMCMP_INDEX_NONE; label = kefir_asmcmp_context_instr_label_next(&target->context, label)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, LABEL_FMT, target->function_name, label));
    }

    switch (instr->opcode) {
#define DEF_OPCODE_arg0(_opcode, _xasmgen) REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen))
#define DEF_OPCODE(_opcode, _xasmgen, _argtp)   \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):    \
        DEF_OPCODE_##_argtp(_opcode, _xasmgen); \
        break

        KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, ;);
#undef DEF_OPCODE
#undef DEF_OPCODE_arg0
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_amd64_xasmgen *xasmgen,
                                                const struct kefir_asmcmp_amd64 *target) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 target"));

    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&target->context, idx)) {
        REQUIRE_OK(generate_instr(xasmgen, target, idx));
    }
    return KEFIR_OK;
}
