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

#ifndef KEFIR_DRIVER_CONFIGURATION_H_
#define KEFIR_DRIVER_CONFIGURATION_H_

#include "kefir/core/list.h"
#include "kefir/core/string_pool.h"
#include "kefir/driver/target.h"

typedef struct kefir_driver_assembler_configuration {
    struct kefir_list arguments;
} kefir_driver_assembler_configuration_t;

kefir_result_t kefir_driver_assembler_configuration_init(struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *,
                                                         struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_add_argument(struct kefir_mem *,
                                                                 struct kefir_driver_assembler_configuration *,
                                                                 const char *);

typedef struct kefir_driver_linker_configuration {
    struct kefir_list arguments;

    struct {
        kefir_bool_t static_linking;
        kefir_bool_t shared_linking;
        kefir_bool_t pie_linking;
        kefir_bool_t link_start_files;
        kefir_bool_t link_default_libs;
        kefir_bool_t link_libc;
        kefir_bool_t link_rtlib;
    } flags;

    const char *rtlib_location;
} kefir_driver_linker_configuration_t;

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *, struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_add_argument(struct kefir_mem *,
                                                              struct kefir_driver_linker_configuration *, const char *);

typedef enum kefir_driver_stage {
    KEFIR_DRIVER_STAGE_PREPROCESS,
    KEFIR_DRIVER_STAGE_PREPROCESS_SAVE,
    KEFIR_DRIVER_STAGE_DEPENDENCY_OUTPUT,
    KEFIR_DRIVER_STAGE_PRINT_TOKENS,
    KEFIR_DRIVER_STAGE_PRINT_AST,
    KEFIR_DRIVER_STAGE_PRINT_IR,
    KEFIR_DRIVER_STAGE_PRINT_OPT,
    KEFIR_DRIVER_STAGE_COMPILE,
    KEFIR_DRIVER_STAGE_ASSEMBLE,
    KEFIR_DRIVER_STAGE_LINK,
    KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE,
    KEFIR_DRIVER_STAGE_RUN
} kefir_driver_stage_t;

typedef enum kefir_driver_argument_type {
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA
} kefir_driver_argument_type_t;

typedef struct kefir_driver_argument {
    kefir_driver_argument_type_t type;
    const char *value;
} kefir_driver_argument_t;

typedef struct kefir_driver_definition {
    const char *name;
    const char *value;
} kefir_driver_definition_t;

typedef enum kefir_driver_frame_pointer_omission {
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_ENABLE,
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_DISABLE,
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_UNSPECIFIED
} kefir_driver_frame_pointer_omission_t;

typedef struct kefir_driver_configuration {
    kefir_driver_stage_t stage;
    const char *output_file;
    struct kefir_list arguments;
    struct kefir_list assembler_arguments;
    struct kefir_list compiler_arguments;
    struct kefir_hashtree defines;
    struct kefir_list undefines;
    struct kefir_list include_directories;
    struct kefir_list include_files;
    struct kefir_driver_target target;

    struct {
        kefir_int_t optimization_level;
    } compiler;

    struct {
        kefir_bool_t skip_preprocessor;
        kefir_bool_t restrictive_mode;
        kefir_bool_t static_linking;
        kefir_bool_t shared_linking;
        kefir_bool_t position_independent_code;
        kefir_bool_t position_independent_executable;
        kefir_driver_frame_pointer_omission_t omit_frame_pointer;
        kefir_bool_t link_start_files;
        kefir_bool_t link_default_libs;
        kefir_bool_t link_libc;
        kefir_bool_t include_rtinc;
        kefir_bool_t link_rtlib;
    } flags;

    struct {
        kefir_bool_t output_system_deps;
        const char *target_name;
    } dependency_output;

    struct {
        struct kefir_list args;
        const char *file_stdin;
        const char *file_stdout;
        kefir_bool_t stderr_to_stdout;
        const char *file_stderr;
    } run;
} kefir_driver_configuration_t;

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *);
kefir_result_t kefir_driver_configuration_free(struct kefir_mem *, struct kefir_driver_configuration *);

kefir_result_t kefir_driver_configuration_add_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                       struct kefir_driver_configuration *, const char *,
                                                       kefir_driver_argument_type_t);
kefir_result_t kefir_driver_configuration_add_assembler_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                                 struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_compiler_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *, struct kefir_string_pool *,
                                                     struct kefir_driver_configuration *, const char *, const char *);
kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *, struct kefir_string_pool *,
                                                       struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *, struct kefir_string_pool *,
                                                           struct kefir_driver_configuration *, const char *);

#endif
