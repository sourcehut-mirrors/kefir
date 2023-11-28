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

#include "kefir/driver/target.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/platform.h"
#include <string.h>

static kefir_result_t match_backend(const char *spec, struct kefir_driver_target *target, const char **next) {
    static const char spec_optimizing[] = "opt-";

    if (strncmp(spec, spec_optimizing, sizeof(spec_optimizing) - 1) == 0) {
        target->backend = KEFIR_DRIVER_TARGET_BACKEND_DEFAULT;
        *next = spec + (sizeof(spec_optimizing) - 1);
    }
    return KEFIR_OK;
}

static kefir_result_t match_arch(const char *spec, struct kefir_driver_target *target, const char **next) {
    static const char spec_x86_64[] = "x86_64";
    static const char spec_hostcpu[] = "hostcpu";
    static const char spec_host[] = "host";

    const char *delim = strchr(spec, '-');
    if (delim == NULL) {
        delim = spec + strlen(spec) - 1;
    }

    if (strncmp(spec, spec_x86_64, sizeof(spec_x86_64) - 1) == 0) {
        target->arch = KEFIR_DRIVER_TARGET_ARCH_X86_64;
        *next = delim + 1;
    } else if (strncmp(spec, spec_hostcpu, sizeof(spec_hostcpu) - 1) == 0) {
        *next = delim + 1;
    } else if (strncmp(spec, spec_host, sizeof(spec_host) - 1) == 0) {
        // Intentionally left blank
    } else {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Architecture specification is not found");
    }
    return KEFIR_OK;
}

static kefir_result_t select_host_platform(struct kefir_driver_target *target) {
#ifdef KEFIR_CONFIG_HOST_PLATFORM
    target->platform = KEFIR_CONFIG_HOST_PLATFORM;
#else
#ifdef KEFIR_LINUX_HOST_PLATFORM
    target->platform = KEFIR_DRIVER_TARGET_PLATFORM_LINUX;
#elif defined(KEFIR_FREEBSD_HOST_PLATFORM)
    target->platform = KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD;
#elif defined(KEFIR_OPENBSD_HOST_PLATFORM)
    target->platform = KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD;
#elif defined(KEFIR_NETBSD_HOST_PLATFORM)
    target->platform = KEFIR_DRIVER_TARGET_PLATFORM_NETBSD;
#elif defined(KEFIR_EMSCRIPTEN_HOST_PLATFORM)
    target->platform = KEFIR_DRIVER_TARGET_PLATFORM_LINUX;
#endif
#endif
    return KEFIR_OK;
}

static kefir_result_t match_platform(const char *spec, struct kefir_driver_target *target, const char **next) {
    static const char spec_linux[] = "linux";
    static const char spec_freebsd[] = "freebsd";
    static const char spec_openbsd[] = "openbsd";
    static const char spec_netbsd[] = "netbsd";
    static const char spec_hostos[] = "hostos";
    static const char spec_host[] = "host";

    const char *delim = strchr(spec, '-');
    if (delim == NULL) {
        delim = spec + strlen(spec) - 1;
    }

    if (strncmp(spec, spec_linux, sizeof(spec_linux) - 1) == 0) {
        target->platform = KEFIR_DRIVER_TARGET_PLATFORM_LINUX;
    } else if (strncmp(spec, spec_freebsd, sizeof(spec_freebsd) - 1) == 0) {
        target->platform = KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD;
    } else if (strncmp(spec, spec_openbsd, sizeof(spec_openbsd) - 1) == 0) {
        target->platform = KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD;
    } else if (strncmp(spec, spec_netbsd, sizeof(spec_netbsd) - 1) == 0) {
        target->platform = KEFIR_DRIVER_TARGET_PLATFORM_NETBSD;
    } else if (strncmp(spec, spec_hostos, sizeof(spec_hostos) - 1) == 0 ||
               strncmp(spec, spec_host, sizeof(spec_host) - 1) == 0) {
        REQUIRE_OK(select_host_platform(target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Platform specification is not found");
    }
    *next = delim + 1;
    return KEFIR_OK;
}

static kefir_result_t select_default_variant(struct kefir_driver_target *target) {
#ifdef KEFIR_CONFIG_HOST_VARIANT
    target->variant = KEFIR_CONFIG_HOST_VARIANT;
#else
    target->variant = KEFIR_DRIVER_TARGET_VARIANT_NONE;
    if (target->arch == KEFIR_DRIVER_TARGET_ARCH_X86_64 && target->platform == KEFIR_DRIVER_TARGET_PLATFORM_LINUX) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_GNU;
    } else if (target->arch == KEFIR_DRIVER_TARGET_ARCH_X86_64 &&
               target->platform == KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_SYSTEM;
    } else if (target->arch == KEFIR_DRIVER_TARGET_ARCH_X86_64 &&
               target->platform == KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_SYSTEM;
    }
#endif
    return KEFIR_OK;
}

static kefir_result_t match_variant(const char *spec, struct kefir_driver_target *target) {
    if (strlen(spec) == 0) {
        REQUIRE_OK(select_default_variant(target));
    } else if (strcmp(spec, "none") == 0) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_NONE;
    } else if (strcmp(spec, "musl") == 0) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_MUSL;
    } else if (strcmp(spec, "gnu") == 0) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_GNU;
    } else if (strcmp(spec, "system") == 0) {
        target->variant = KEFIR_DRIVER_TARGET_VARIANT_SYSTEM;
    } else if (strcmp(spec, "default") == 0) {
        REQUIRE_OK(select_default_variant(target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Variant specification is not found");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_driver_target_match(const char *spec, struct kefir_driver_target *target) {
    REQUIRE(spec != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target specification"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver target"));

    REQUIRE_OK(match_backend(spec, target, &spec));
    REQUIRE_OK(match_arch(spec, target, &spec));
    REQUIRE_OK(match_platform(spec, target, &spec));
    REQUIRE_OK(match_variant(spec, target));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_target_default(struct kefir_driver_target *target) {
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver target"));

    target->backend = KEFIR_DRIVER_TARGET_BACKEND_DEFAULT;
    target->arch = KEFIR_DRIVER_TARGET_ARCH_X86_64;
    REQUIRE_OK(select_host_platform(target));
    REQUIRE_OK(select_default_variant(target));
    return KEFIR_OK;
}
