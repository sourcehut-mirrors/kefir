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

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include "kefir/compiler/compiler.h"
#include "kefir/driver/runner.h"
#include "kefir/core/version.h"
#include "kefir/driver/driver.h"
#include "kefir/driver/parser.h"
#include "kefir/platform/tempfile.h"

// Driver main entry

extern const char KefirDriverHelpContent[];
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
    fprintf(out, "Version: %u.%u.%u\n", KEFIR_VERSION_MAJOR, KEFIR_VERSION_MINOR, KEFIR_VERSION_PATCH);
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
#else
            "unknown"
#endif
    );

    fprintf(out, "Build information:\n");
#if defined(__KEFIRCC__)
    fprintf(out, "    Compiler: Kefir\n");
    fprintf(out, "    Compiler version: %s\n", __KEFIRCC_VERSION__);
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

    fprintf(out, "URL: %s\n", "https://github.com/protopopov1122/Kefir");
    return KEFIR_OK;
}

static const char *str_nonnull_or(const char *str, const char *alternative) {
    return str != NULL ? str : alternative;
}

static kefir_result_t print_toolchain_env(FILE *out, const char *name,
                                          const struct kefir_driver_external_resource_toolchain_config *config) {
    fprintf(out, "KEFIR_%s_INCLUDE=\"%s\"\n", name, str_nonnull_or(config->include_path, ""));
    fprintf(out, "KEFIR_%s_LIB=\"%s\"\n", name, str_nonnull_or(config->library_path, ""));
    fprintf(out, "KEFIR_%s_DYNAMIC_LINKER=\"%s\"\n", name, str_nonnull_or(config->dynamic_linker, ""));
    return KEFIR_OK;
}

static kefir_result_t print_environment(FILE *out, const struct kefir_driver_external_resources *externals) {
    fprintf(out, "KEFIR_AS=\"%s\"\n", externals->assembler_path);
    fprintf(out, "KEFIR_LD=\"%s\"\n", externals->linker_path);
    fprintf(out, "KEFIR_RTINC=\"%s\"\n", str_nonnull_or(externals->runtime_include, ""));
    fprintf(out, "KEFIR_RTLIB=\"%s\"\n", str_nonnull_or(externals->runtime_library, ""));
    fprintf(out, "KEFIR_WORKDIR=\"%s\"\n", externals->work_dir);
    REQUIRE_OK(print_toolchain_env(out, "GNU", &externals->gnu));
    REQUIRE_OK(print_toolchain_env(out, "MUSL", &externals->musl));
    REQUIRE_OK(print_toolchain_env(out, "FREEBSD", &externals->freebsd));
    REQUIRE_OK(print_toolchain_env(out, "OPENBSD", &externals->openbsd));
    REQUIRE_OK(print_toolchain_env(out, "NETBSD", &externals->netbsd));
    return KEFIR_OK;
}

int main(int argc, char *const *argv) {
    UNUSED(argc);
    init_tmpmgr();
    kefir_result_t res = KEFIR_OK;
    struct kefir_mem *mem = kefir_system_memalloc();

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

    struct kefir_symbol_table symbols;
    struct kefir_driver_configuration driver_config;
    struct kefir_driver_external_resources exteral_resources;
    kefir_driver_command_t command;
    int exit_code = EXIT_SUCCESS;

    REQUIRE_CHAIN(&res, kefir_symbol_table_init(&symbols));
    REQUIRE_CHAIN(&res, kefir_driver_configuration_init(&driver_config));
    REQUIRE_CHAIN(&res, kefir_driver_external_resources_init_from_env(mem, &exteral_resources, &tmpmgr));
    REQUIRE_CHAIN(&res, kefir_driver_parse_args(mem, &symbols, &driver_config, (const char *const *) argv + 1, argc - 1,
                                                &command, stderr));
    if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_HELP) {
        fprintf(stdout, "%s", KefirDriverHelpContent);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_VERSION) {
        fprintf(stdout, "%u.%u.%u\n", KEFIR_VERSION_MAJOR, KEFIR_VERSION_MINOR, KEFIR_VERSION_PATCH);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_INFO) {
        res = print_compiler_info(stdout, argv[0]);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_ENVIRONMENT) {
        res = print_environment(stdout, &exteral_resources);
    } else {
        REQUIRE_CHAIN(&res, kefir_driver_run(mem, &symbols, &driver_config, &exteral_resources));
        if (res == KEFIR_INTERRUPT) {
            res = KEFIR_OK;
            exit_code = EXIT_FAILURE;
        }
    }

    REQUIRE_CHAIN(&res, kefir_driver_configuration_free(mem, &driver_config));
    REQUIRE_CHAIN(&res, kefir_symbol_table_free(mem, &symbols));
    return kefir_report_error(stderr, res, false) ? exit_code : EXIT_FAILURE;
}
