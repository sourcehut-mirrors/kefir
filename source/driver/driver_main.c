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

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include "kefir/compiler/compiler.h"
#include "kefir/driver/runner.h"

#include "kefir/driver/driver.h"
#include "kefir/util/tempfile.h"

// Driver main entry

static struct kefir_tempfile_manager tmpmgr;

static void tmpmgr_cleanup(void) {
    if (!kefir_process_is_fork()) {
        kefir_tempfile_manager_free(kefir_system_memalloc(), &tmpmgr);
    }
}

static void sighandler(int signum) {
    fprintf(stderr, "Caught signal: %d\n", signum);
    tmpmgr_cleanup();
}

static kefir_result_t init_tmpmgr() {
    REQUIRE_OK(kefir_tempfile_manager_init(&tmpmgr));
    atexit(tmpmgr_cleanup);
    signal(SIGTERM, sighandler);
    signal(SIGABRT, sighandler);
    signal(SIGSEGV, sighandler);
    return KEFIR_OK;
}

int main(int argc, char *const *argv) {
    UNUSED(argc);
    init_tmpmgr();
    struct kefir_mem *mem = kefir_system_memalloc();

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

    struct kefir_driver_configuration driver_config;
    struct kefir_driver_external_resources exteral_resources;
    kefir_result_t res = kefir_driver_configuration_init(&driver_config);
    REQUIRE_CHAIN(&res, kefir_driver_external_resources_init_from_env(mem, &exteral_resources, &tmpmgr));
    REQUIRE_CHAIN(
        &res, kefir_driver_configuration_add_input(mem, NULL, &driver_config, argv[1], KEFIR_DRIVER_INPUT_FILE_CODE));
    REQUIRE_CHAIN(&res,
                  kefir_driver_configuration_add_input(mem, NULL, &driver_config, exteral_resources.runtime_library,
                                                       KEFIR_DRIVER_INPUT_FILE_LIBRARY));
    REQUIRE_CHAIN(&res, kefir_driver_configuration_add_input(mem, NULL, &driver_config, "/usr/lib/musl/lib/crt1.o",
                                                             KEFIR_DRIVER_INPUT_FILE_OBJECT));
    REQUIRE_CHAIN(&res, kefir_driver_configuration_add_input(mem, NULL, &driver_config, "/usr/lib/musl/lib/libc.a",
                                                             KEFIR_DRIVER_INPUT_FILE_LIBRARY));

    REQUIRE_CHAIN(&res, kefir_driver_run(mem, &driver_config, &exteral_resources));
    int exit_code = EXIT_SUCCESS;
    if (res == KEFIR_INTERRUPT) {
        res = KEFIR_OK;
        exit_code = EXIT_FAILURE;
    }

    REQUIRE_CHAIN(&res, kefir_driver_configuration_free(mem, &driver_config));
    return kefir_report_error(stderr, res, false) ? exit_code : EXIT_FAILURE;
}
