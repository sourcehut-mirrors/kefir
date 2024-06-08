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

#ifndef KEFIR_DRIVER_DRIVER_PROLOGUE_INTERNAL_H_
#define KEFIR_DRIVER_DRIVER_PROLOGUE_INTERNAL_H_

#include "kefir/compiler/compiler.h"
#include "kefir/driver/runner.h"
#include "kefir/core/version.h"
#include "kefir/driver/driver.h"
#include "kefir/driver/parser.h"
#include "kefir/platform/tempfile.h"

#ifndef KEFIR_DRIVER_PROLOGUE_INTERNAL
#error "driver_prologue.h shall not be included directly"
#endif

static const unsigned char KefirDriverHelpContent[] = {
#include STRINGIFY(KEFIR_DRIVER_HELP_INCLUDE)
};
static struct kefir_tempfile_manager tmpmgr;

static void tmpmgr_cleanup(void) {
    if (!kefir_process_is_fork()) {
        kefir_tempfile_manager_free(kefir_system_memalloc(), &tmpmgr);
    }
}

static void sighandler(int signum) {
    if (!kefir_process_is_fork() && (signum == SIGSEGV || signum == SIGFPE)) {
        fprintf(stderr, "Kefir caught signal: %d, terminating\n", signum);
    }
    tmpmgr_cleanup();
    exit(EXIT_FAILURE);
}

static kefir_result_t init_tmpmgr(void) {
    REQUIRE_OK(kefir_tempfile_manager_init(&tmpmgr));
    atexit(tmpmgr_cleanup);
    signal(SIGTERM, sighandler);
    signal(SIGABRT, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGFPE, sighandler);
    return KEFIR_OK;
}

static kefir_result_t print_compiler_info(FILE *out, const char *exec_name) {

    fprintf(out, "Executable: %s\n", exec_name);
    fprintf(out, "Version: %s\n", KEFIR_VERSION_FULL);
    fprintf(out, "Host: %s\n",
#if defined(KEFIR_LINUX_HOST_PLATFORM)
            "linux"
#elif defined(KEFIR_FREEBSD_HOST_PLATFORM)
            "freebsd"
#elif defined(KEFIR_OPENBSD_HOST_PLATFORM)
            "openbsd"
#elif defined(KEFIR_NETBSD_HOST_PLATFORM)
            "netbsd"
#elif defined(KEFIR_UNIX_HOST_PLATFORM)
            "unix"
#elif defined(KEFIR_EMSCRIPTEN_HOST_PLATFORM)
            "emscripten"
#else
            "unknown"
#endif
    );

    fprintf(out, "Build information:\n");
#if defined(__KEFIRCC__)
    fprintf(out, "    Compiler: Kefir\n");
    fprintf(out, "    Compiler version: %s\n", __KEFIRCC_VERSION__);
#elif defined(__EMSCRIPTEN__)
    fprintf(out, "    Compiler: Emscripten\n");
    fprintf(out, "    Compiler version: %s\n", __clang_version__);
#elif defined(__clang__)
    fprintf(out, "    Compiler: Clang\n");
    fprintf(out, "    Compiler version: %s\n", __clang_version__);
#elif defined(__GNUC__)
    fprintf(out, "    Compiler: GCC\n");
#ifdef __VERSION__
    fprintf(out, "    Compiler version: %s\n", __VERSION__);
#else
    fprintf(out, "    Compiler version: unknown\n");
#endif
#else
    fprintf(out, "    Compiler: Unknown\n");
#endif

#ifdef KEFIR_BUILD_SOURCE_ID
    fprintf(out, "    Source-ID: %s\n", KEFIR_BUILD_SOURCE_ID);
#endif

#ifdef KEFIR_BUILD_TIMESTAMP
    const time_t build_time = KEFIR_BUILD_TIMESTAMP;
    fprintf(out, "    Build time (UTC): %s", asctime(gmtime(&build_time)));
#endif

#ifdef KEFIR_BUILD_CFLAGS
    fprintf(out, "    Compiler flags: %s\n", KEFIR_BUILD_CFLAGS);
#endif

    fprintf(out, "    Pre-configured host target: %s\n",
#ifdef KEFIR_CONFIG_HOST_TARGET
            KEFIR_CONFIG_HOST_TARGET
#else
            "none"
#endif
    );

    fprintf(out, "URLs  (primary): %s\n", "https://sr.ht/~jprotopopov/kefir");
    fprintf(out, "     (mirror 1): %s\n", "https://git.protopopov.lv/kefir");
    fprintf(out, "     (mirror 2): %s\n", "https://codeberg.org/jprotopopov/kefir");
    return KEFIR_OK;
}

static kefir_result_t print_toolchain_env(FILE *out, const char *name,
                                          const struct kefir_driver_external_resource_toolchain_config *config) {
    if (config->include_path != NULL) {
        fprintf(out, "KEFIR_%s_INCLUDE=\"%s\"\n", name, config->include_path);
    }
    if (config->library_path != NULL) {
        fprintf(out, "KEFIR_%s_LIB=\"%s\"\n", name, config->library_path);
    }
    if (config->dynamic_linker != NULL) {
        fprintf(out, "KEFIR_%s_DYNAMIC_LINKER=\"%s\"\n", name, config->dynamic_linker);
    }
    return KEFIR_OK;
}

static kefir_result_t print_target(FILE *out, const struct kefir_driver_target *target) {
    switch (target->backend) {
        case KEFIR_DRIVER_TARGET_BACKEND_DEFAULT:
            break;
    }

    switch (target->arch) {
        case KEFIR_DRIVER_TARGET_ARCH_X86_64:
            fprintf(out, "x86_64-");
            break;
    }

    switch (target->platform) {
        case KEFIR_DRIVER_TARGET_PLATFORM_LINUX:
            fprintf(out, "linux-");
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD:
            fprintf(out, "freebsd-");
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD:
            fprintf(out, "openbsd-");
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_NETBSD:
            fprintf(out, "netbsd-");
            break;
    }

    switch (target->variant) {
        case KEFIR_DRIVER_TARGET_VARIANT_NONE:
            fprintf(out, "none");
            break;

        case KEFIR_DRIVER_TARGET_VARIANT_GNU:
            fprintf(out, "gnu");
            break;

        case KEFIR_DRIVER_TARGET_VARIANT_MUSL:
            fprintf(out, "musl");
            break;

        case KEFIR_DRIVER_TARGET_VARIANT_SYSTEM:
            fprintf(out, "system");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t print_environment(FILE *out, const struct kefir_driver_target *target,
                                        const struct kefir_driver_external_resources *externals) {
    fprintf(out, "KEFIR_TARGET=\"");
    REQUIRE_OK(print_target(out, target));
    fprintf(out, "\"\n");
    fprintf(out, "KEFIR_AS=\"%s\"\n", externals->assembler_path);
    fprintf(out, "KEFIR_LD=\"%s\"\n", externals->linker_path);
    if (externals->runtime_include != NULL) {
        fprintf(out, "KEFIR_RTINC=\"%s\"\n", externals->runtime_include);
    }
    if (externals->runtime_library) {
        fprintf(out, "KEFIR_RTLIB=\"%s\"\n", externals->runtime_library);
    }
    REQUIRE_OK(print_toolchain_env(out, "GNU", &externals->gnu));
    REQUIRE_OK(print_toolchain_env(out, "MUSL", &externals->musl));
    REQUIRE_OK(print_toolchain_env(out, "FREEBSD", &externals->freebsd));
    REQUIRE_OK(print_toolchain_env(out, "OPENBSD", &externals->openbsd));
    REQUIRE_OK(print_toolchain_env(out, "NETBSD", &externals->netbsd));
    return KEFIR_OK;
}

static kefir_result_t print_target_environment_header(FILE *out, const struct kefir_driver_target *target,
                                                      const struct kefir_driver_external_resources *externals) {
    UNUSED(out);
    fprintf(out, "#define KEFIR_CONFIG_HOST_AS \"%s\"\n", externals->assembler_path);
    fprintf(out, "#define KEFIR_CONFIG_HOST_LD \"%s\"\n", externals->linker_path);
    switch (target->platform) {
        case KEFIR_DRIVER_TARGET_PLATFORM_LINUX:
            switch (target->variant) {
                case KEFIR_DRIVER_TARGET_VARIANT_NONE:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "linux-none");
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_SYSTEM:
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_GNU:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "linux-gnu");
                    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_PLATFORM_LINUX));
                    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n", STRINGIFY(KEFIR_DRIVER_TARGET_VARIANT_GNU));
                    if (externals->gnu.include_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH \"%s\"\n",
                                externals->gnu.include_path);
                    }
                    if (externals->gnu.library_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH \"%s\"\n",
                                externals->gnu.library_path);
                    }
                    if (externals->gnu.dynamic_linker != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER \"%s\"\n",
                                externals->gnu.dynamic_linker);
                    }
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_MUSL:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "linux-musl");
                    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_PLATFORM_LINUX));
                    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n", STRINGIFY(KEFIR_DRIVER_TARGET_VARIANT_MUSL));
                    if (externals->musl.include_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH \"%s\"\n",
                                externals->musl.include_path);
                    }
                    if (externals->musl.library_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH \"%s\"\n",
                                externals->musl.library_path);
                    }
                    if (externals->musl.dynamic_linker != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER \"%s\"\n",
                                externals->musl.dynamic_linker);
                    }
                    break;
            }
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD:
            switch (target->variant) {
                case KEFIR_DRIVER_TARGET_VARIANT_NONE:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "freebsd-none");
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_GNU:
                case KEFIR_DRIVER_TARGET_VARIANT_MUSL:
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_SYSTEM:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "freebsd-system");
                    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD));
                    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_VARIANT_SYSTEM));
                    if (externals->freebsd.include_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_INCLUDE_PATH \"%s\"\n",
                                externals->freebsd.include_path);
                    }
                    if (externals->freebsd.library_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_LIBRARY_PATH \"%s\"\n",
                                externals->freebsd.library_path);
                    }
                    if (externals->freebsd.dynamic_linker != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_DYNAMIC_LINKER \"%s\"\n",
                                externals->freebsd.dynamic_linker);
                    }
                    break;
            }
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD:
            switch (target->variant) {
                case KEFIR_DRIVER_TARGET_VARIANT_NONE:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "openbsd-none");
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_GNU:
                case KEFIR_DRIVER_TARGET_VARIANT_MUSL:
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_SYSTEM:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "openbsd-system");
                    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD));
                    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_VARIANT_SYSTEM));
                    if (externals->openbsd.include_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_INCLUDE_PATH \"%s\"\n",
                                externals->openbsd.include_path);
                    }
                    if (externals->openbsd.library_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_LIBRARY_PATH \"%s\"\n",
                                externals->openbsd.library_path);
                    }
                    if (externals->openbsd.dynamic_linker != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_DYNAMIC_LINKER \"%s\"\n",
                                externals->openbsd.dynamic_linker);
                    }
                    break;
            }
            break;

        case KEFIR_DRIVER_TARGET_PLATFORM_NETBSD:
            switch (target->variant) {
                case KEFIR_DRIVER_TARGET_VARIANT_NONE:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "netbsd-none");
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_GNU:
                case KEFIR_DRIVER_TARGET_VARIANT_MUSL:
                    break;

                case KEFIR_DRIVER_TARGET_VARIANT_SYSTEM:
                    fprintf(out, "#define KEFIR_CONFIG_HOST_TARGET \"x86_64-%s\"\n", "netbsd-system");
                    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_PLATFORM_NETBSD));
                    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n",
                            STRINGIFY(KEFIR_DRIVER_TARGET_VARIANT_SYSTEM));
                    if (externals->netbsd.include_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_INCLUDE_PATH \"%s\"\n",
                                externals->netbsd.include_path);
                    }
                    if (externals->netbsd.library_path != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_LIBRARY_PATH \"%s\"\n",
                                externals->netbsd.library_path);
                    }
                    if (externals->netbsd.dynamic_linker != NULL) {
                        fprintf(out, "#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_DYNAMIC_LINKER \"%s\"\n",
                                externals->netbsd.dynamic_linker);
                    }
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

#endif
