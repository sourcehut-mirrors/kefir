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

#include "kefir/compiler/profile.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/target/abi/amd64/platform.h"
#include "kefir/codegen/amd64/codegen.h"
#include <float.h>

static const char KefirAmd64RuntimeCode[] = {
#include STRINGIFY(KEFIR_AMD64_RUNTIME_INCLUDE)
};

static kefir_result_t amd64_sysv_free_codegen(struct kefir_mem *mem, struct kefir_codegen *codegen) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));

    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, codegen));
    KEFIR_FREE(mem, codegen->self);
    return KEFIR_OK;
}

static kefir_result_t amd64_new_codegen(struct kefir_mem *mem, FILE *output,
                                        const struct kefir_codegen_configuration *config,
                                        struct kefir_codegen **codegen_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid FILE"));
    REQUIRE(codegen_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code generator"));

    struct kefir_codegen_amd64 *codegen = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_amd64));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AMD64 code generator"));
    kefir_result_t res = kefir_codegen_amd64_init(mem, codegen, output, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, config);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, codegen);
        return res;
    });

    *codegen_ptr = &codegen->codegen;
    return KEFIR_OK;
}

static kefir_result_t kefir_compiler_new_amd64_sysv_profile(struct kefir_compiler_profile *profile) {
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler profile"));

    static const struct kefir_data_model_descriptor DATA_MODEL_DESCRIPTOR = {
        .model = KEFIR_DATA_MODEL_LP64,
        .byte_order = KEFIR_BYTE_ORDER_LITTLE_ENDIAN,
        .int_width = {.short_int = 16, .integer = 32, .long_int = 64, .long_long_int = 64},
        .floating_point = {.float_radix = FLT_RADIX,
                           .float_mantissa_digits = FLT_MANT_DIG,
                           .double_mantissa_digits = DBL_MANT_DIG,
                           .long_double_mantissa_digits = LDBL_MANT_DIG,
                           .float_digits = FLT_DIG,
                           .double_digits = DBL_DIG,
                           .long_double_digits = LDBL_DIG,
                           .float_min_exponent = FLT_MIN_EXP,
                           .double_min_exponent = DBL_MIN_EXP,
                           .long_double_min_exponent = LDBL_MIN_EXP,
                           .float_min10_exponent = FLT_MIN_10_EXP,
                           .double_min10_exponent = DBL_MIN_10_EXP,
                           .long_double_min10_exponent = LDBL_MIN_10_EXP,
                           .float_max_exponent = FLT_MAX_EXP,
                           .double_max_exponent = DBL_MAX_EXP,
                           .long_double_max_exponent = LDBL_MAX_EXP,
                           .float_max10_exponent = FLT_MAX_10_EXP,
                           .double_max10_exponent = DBL_MAX_10_EXP,
                           .long_double_max10_exponent = LDBL_MAX_10_EXP,
                           .float_max = "3.40282346638528859812e+38F",
                           .double_max = "1.79769313486231570815e+308",
                           .long_double_max = "1.1897314953572317650e+4932L",
                           .float_epsilon = "1.1920928955078125e-07F",
                           .double_epsilon = "2.22044604925031308085e-16",
                           .long_double_epsilon = "1.0842021724855044340e-19L",
                           .float_min = "1.17549435082228750797e-38F",
                           .double_min = "2.22507385850720138309e-308",
                           .long_double_min = "3.3621031431120935063e-4932L"},
        .char_bit = 8};

    static struct kefir_ast_type_traits TYPE_TRAITS;
    static kefir_bool_t TYPE_TRAITS_INIT_DONE = false;
    if (!TYPE_TRAITS_INIT_DONE) {
        REQUIRE_OK(kefir_ast_type_traits_init(&DATA_MODEL_DESCRIPTOR, &TYPE_TRAITS));
        TYPE_TRAITS_INIT_DONE = true;
    }

    REQUIRE_OK(kefir_lexer_context_default(&profile->lexer_context));
    REQUIRE_OK(kefir_lexer_context_integral_width_from_data_model(&profile->lexer_context, &DATA_MODEL_DESCRIPTOR));
    REQUIRE_OK(kefir_abi_amd64_target_platform(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &profile->ir_target_platform));
    profile->optimizer_enabled = true;
    profile->data_model = &DATA_MODEL_DESCRIPTOR;
    profile->type_traits = &TYPE_TRAITS;
    profile->new_codegen = amd64_new_codegen;
    profile->free_codegen = amd64_sysv_free_codegen;
    profile->runtime_code = KefirAmd64RuntimeCode;
    profile->runtime_include_dirname = NULL;
    return KEFIR_OK;
}

const struct Profile {
    const char *identifier;
    kefir_result_t (*init)(struct kefir_compiler_profile *);
} Profiles[] = {{"amd64-sysv-gas", kefir_compiler_new_amd64_sysv_profile},
                {NULL, kefir_compiler_new_amd64_sysv_profile}};
const kefir_size_t ProfileCount = sizeof(Profiles) / sizeof(Profiles[0]);

kefir_result_t kefir_compiler_profile(struct kefir_compiler_profile *profile, const char *identifier) {
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler profile"));

    for (kefir_size_t i = 0; i < ProfileCount; i++) {
        const struct Profile *profileId = &Profiles[i];
        if ((identifier != NULL && profileId->identifier != NULL && strcmp(identifier, profileId->identifier) == 0) ||
            (identifier == NULL && profileId->identifier == NULL)) {
            REQUIRE_OK(profileId->init(profile));
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested compiler profile");
}
