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

static const char KefirDriverHelpContent[] = {
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

#ifdef KEFIR_BUILD_CFLAGS
    fprintf(out, "    Compiler flags: %s\n", KEFIR_BUILD_CFLAGS);
#endif

    fprintf(out, "    Pre-configured host enviroment: %s\n",
#ifdef KEFIR_CONFIG_HOST_ENVIRONMENT
            KEFIR_CONFIG_HOST_ENVIRONMENT
#else
            "none"
#endif
    );

    fprintf(out, "URL: %s\n     %s\n", "https://github.com/protopopov1122/kefir", "https://sr.ht/~jprotopopov/kefir");
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

static kefir_result_t print_host_environment(FILE *out) {
#ifdef KEFIR_CONFIG_HOST_ENVIRONMENT
    fprintf(out, "#define KEFIR_CONFIG_HOST_ENVIRONMENT \"%s\"\n", KEFIR_CONFIG_HOST_ENVIRONMENT);
#endif

#ifdef KEFIR_CONFIG_HOST_PLATFORM
    fprintf(out, "#define KEFIR_CONFIG_HOST_PLATFORM %s\n", STRINGIFY(KEFIR_CONFIG_HOST_PLATFORM));
#endif

#ifdef KEFIR_CONFIG_HOST_VARIANT
    fprintf(out, "#define KEFIR_CONFIG_HOST_VARIANT %s\n", STRINGIFY(KEFIR_CONFIG_HOST_VARIANT));
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH \"%s\"\n", KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH);
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH \"%s\"\n", KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH);
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER \"%s\"\n",
            KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER);
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH \"%s\"\n",
            KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH);
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH \"%s\"\n",
            KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH);
#endif

#ifdef KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER
    fprintf(out, "#define KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER \"%s\"\n",
            KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER);
#endif
    return KEFIR_OK;
}

#endif
