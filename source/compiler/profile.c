/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

static kefir_result_t amd64_sysv_free_codegen(struct kefir_mem *mem, struct kefir_codegen *codegen) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));

    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, codegen));
    KEFIR_FREE(mem, codegen->self);
    return KEFIR_OK;
}

static kefir_result_t amd64_new_codegen(struct kefir_mem *mem, FILE *output,
                                        const struct kefir_codegen_configuration *config,
                                        const struct kefir_codegen_runtime_hooks *runtime_hooks,
                                        struct kefir_codegen **codegen_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid FILE"));
    REQUIRE(codegen_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code generator"));

    struct kefir_codegen_amd64 *codegen = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_amd64));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AMD64 code generator"));
    kefir_result_t res =
        kefir_codegen_amd64_init(mem, codegen, output, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, runtime_hooks, config);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, codegen);
        return res;
    });

    *codegen_ptr = &codegen->codegen;
    return KEFIR_OK;
}

static kefir_result_t init_amd64_sysv_profile(struct kefir_compiler_profile *profile,
                                              const struct kefir_compiler_profile_configuration *config) {
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler profile"));

    struct kefir_ast_type_traits type_traits;
    REQUIRE_OK(kefir_ast_type_traits_init(profile->ir_target_platform.data_model, &type_traits));
    if (config != NULL) {
        switch (config->char_signedness) {
            case KEFIR_COMPILER_PROFILE_CHAR_SIGNEDNESS_DEFAULT:
                // Intentionally left blank
                break;

            case KEFIR_COMPILER_PROFILE_CHAR_SIGNED:
                type_traits.character_type_signedness = true;
                break;

            case KEFIR_COMPILER_PROFILE_CHAR_UNSIGNED:
                type_traits.character_type_signedness = false;
                break;
        }
    }

    memcpy(profile,
           &(struct kefir_compiler_profile) {.optimizer_enabled = true,
                                             .type_traits = type_traits,
                                             .new_codegen = amd64_new_codegen,
                                             .free_codegen = amd64_sysv_free_codegen,
                                             .ir_target_platform = profile->ir_target_platform,
                                             .runtime_include_dirname = NULL,
                                             .runtime_hooks_enabled = true},
           sizeof(struct kefir_compiler_profile));

    REQUIRE_OK(kefir_lexer_context_default(&profile->lexer_context));
    REQUIRE_OK(kefir_lexer_context_integral_width_from_data_model(&profile->lexer_context,
                                                                  profile->ir_target_platform.data_model));
    return KEFIR_OK;
}

static kefir_result_t kefir_compiler_new_amd64_sysv_profile(struct kefir_mem *mem,
                                                            struct kefir_compiler_profile *profile,
                                                            const struct kefir_compiler_profile_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler profile"));

    REQUIRE_OK(kefir_abi_amd64_target_platform(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &profile->ir_target_platform));
    kefir_result_t res = init_amd64_sysv_profile(profile, config);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_IR_TARGET_PLATFORM_FREE(mem, &profile->ir_target_platform);
        return res;
    });
    return KEFIR_OK;
}

const struct Profile {
    const char *identifier;
    kefir_result_t (*init)(struct kefir_mem *mem, struct kefir_compiler_profile *,
                           const struct kefir_compiler_profile_configuration *);
} Profiles[] = {{"amd64-sysv-gas", kefir_compiler_new_amd64_sysv_profile},
                {NULL, kefir_compiler_new_amd64_sysv_profile}};
const kefir_size_t ProfileCount = sizeof(Profiles) / sizeof(Profiles[0]);

kefir_result_t kefir_compiler_profile(struct kefir_mem *mem, struct kefir_compiler_profile *profile,
                                      const char *identifier,
                                      const struct kefir_compiler_profile_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(profile != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler profile"));

    for (kefir_size_t i = 0; i < ProfileCount; i++) {
        const struct Profile *profileId = &Profiles[i];
        if ((identifier != NULL && profileId->identifier != NULL && strcmp(identifier, profileId->identifier) == 0) ||
            (identifier == NULL && profileId->identifier == NULL)) {
            REQUIRE_OK(profileId->init(mem, profile, config));
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested compiler profile");
}
