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

#define _POSIX_SOURCE
#include "kefir/driver/target_configuration.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdio.h>
#include <limits.h>

kefir_result_t kefir_driver_apply_target_profile_configuration(
    struct kefir_compiler_runner_configuration *compiler_config, const struct kefir_driver_target *target) {
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver target"));

    if (compiler_config != NULL && target->arch == KEFIR_DRIVER_TARGET_ARCH_X86_64) {
        if (target->platform == KEFIR_DRIVER_TARGET_PLATFORM_LINUX ||
            target->platform == KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD) {
            compiler_config->target_profile = "amd64-sysv-gas";
            compiler_config->codegen.emulated_tls = false;
        } else if (target->platform == KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD) {
            compiler_config->target_profile = "amd64-sysv-gas";
            compiler_config->codegen.emulated_tls = true;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_driver_apply_target_configuration(struct kefir_mem *mem,
                                                       const struct kefir_driver_external_resources *externals,
                                                       struct kefir_compiler_runner_configuration *compiler_config,
                                                       struct kefir_driver_assembler_configuration *assembler_config,
                                                       struct kefir_driver_linker_configuration *linker_config,
                                                       const struct kefir_driver_target *target) {
    UNUSED(assembler_config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(externals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver externals"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver target"));

    REQUIRE_OK(kefir_driver_apply_target_profile_configuration(compiler_config, target));

    if (target->platform == KEFIR_DRIVER_TARGET_PLATFORM_LINUX) {
        if (compiler_config != NULL) {
            REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, "__linux__", "1"));
        }

        if (target->variant == KEFIR_DRIVER_TARGET_VARIANT_MUSL) {
            if (compiler_config != NULL) {
                REQUIRE(externals->musl.include_path != NULL,
                        KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Musl library path shall be passed as KEFIR_MUSL_INCLUDE "
                                                        "environment variable for selected target"));
                REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->include_path,
                                                   kefir_list_tail(&compiler_config->include_path),
                                                   (void *) externals->musl.include_path));
            }

            if (linker_config != NULL) {
                REQUIRE(externals->musl.library_path != NULL,
                        KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Musl library path shall be passed as KEFIR_MUSL_LIB "
                                                        "environment variable for selected target"));
                char libpath[PATH_MAX + 1];
                snprintf(libpath, sizeof(libpath) - 1, "%s/crt1.o", externals->musl.library_path);
                REQUIRE_OK(kefir_driver_linker_configuration_add_linked_file(mem, linker_config, libpath));
                snprintf(libpath, sizeof(libpath) - 1, "%s/libc.a", externals->musl.library_path);
                REQUIRE_OK(kefir_driver_linker_configuration_add_linked_file(mem, linker_config, libpath));
            }
        }
    } else if (target->platform == KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD) {
        if (compiler_config != NULL) {
            REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, "__FreeBSD__", "1"));
        }
    } else if (target->platform == KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD) {
        if (compiler_config != NULL) {
            REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, "__OpenBSD__", "1"));
        }
    }

    if (target->variant != KEFIR_DRIVER_TARGET_VARIANT_NONE && linker_config != NULL) {
        REQUIRE(
            externals->runtime_library != NULL,
            KEFIR_SET_ERROR(
                KEFIR_UI_ERROR,
                "Kefir runtime library path shall be passed as KEFIR_RTLIB environment variable for selected target"));
        REQUIRE_OK(kefir_driver_linker_configuration_add_linked_file(mem, linker_config, externals->runtime_library));
    }
    return KEFIR_OK;
}
