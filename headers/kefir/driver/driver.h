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

#ifndef KEFIR_DRIVER_DRIVER_H_
#define KEFIR_DRIVER_DRIVER_H_

#include "kefir/cli/options.h"
#include "kefir/util/process.h"
#include "kefir/util/tempfile.h"
#include "kefir/core/list.h"
#include "kefir/core/symbol_table.h"
#include <stdio.h>

typedef struct kefir_driver_external_resources {
    const char *assembler_path;
    const char *linker_path;

    const char *runtime_library;

    const char *work_dir;

    struct kefir_tempfile_manager *tmpfile_manager;
} kefir_driver_external_resources_t;

kefir_result_t kefir_driver_external_resources_init_from_env(struct kefir_mem *,
                                                             struct kefir_driver_external_resources *,
                                                             struct kefir_tempfile_manager *);

typedef struct kefir_driver_assembler_configuration {
    struct kefir_list extra_args;
} kefir_driver_assembler_configuration_t;

kefir_result_t kefir_driver_assembler_configuration_init(struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *,
                                                         struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_add_extra_argument(struct kefir_mem *,
                                                                       struct kefir_driver_assembler_configuration *,
                                                                       const char *);

typedef struct kefir_driver_linker_configuration {
    struct kefir_list linked_files;
    struct kefir_list extra_args;
} kefir_driver_linker_configuration_t;

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *, struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_add_linked_file(struct kefir_mem *,
                                                                 struct kefir_driver_linker_configuration *,
                                                                 const char *);
kefir_result_t kefir_driver_linker_configuration_add_extra_argument(struct kefir_mem *,
                                                                    struct kefir_driver_linker_configuration *,
                                                                    const char *);

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *, struct kefir_process *);
kefir_result_t kefir_driver_run_assembler(struct kefir_mem *, const char *,
                                          const struct kefir_driver_assembler_configuration *,
                                          const struct kefir_driver_external_resources *, struct kefir_process *);
kefir_result_t kefir_driver_run_linker(struct kefir_mem *, const char *,
                                       const struct kefir_driver_linker_configuration *,
                                       const struct kefir_driver_external_resources *, struct kefir_process *);

typedef enum kefir_driver_stage {
    KEFIR_DRIVER_STAGE_PREPROCESS,
    KEFIR_DRIVER_STAGE_PREPROCESS_SAVE,
    KEFIR_DRIVER_STAGE_COMPILE,
    KEFIR_DRIVER_STAGE_ASSEMBLE,
    KEFIR_DRIVER_STAGE_LINK
} kefir_driver_stage_t;

typedef enum kefir_driver_input_file_type {
    KEFIR_DRIVER_INPUT_FILE_CODE,
    KEFIR_DRIVER_INPUT_FILE_PREPROCESSED,
    KEFIR_DRIVER_INPUT_FILE_ASSEMBLY,
    KEFIR_DRIVER_INPUT_FILE_OBJECT,
    KEFIR_DRIVER_INPUT_FILE_LIBRARY
} kefir_driver_input_file_type_t;

typedef enum kefir_driver_linker_flag_type {
    KEFIR_DRIVER_LINKER_FLAG_LINK_LIBRARY,
    KEFIR_DRIVER_LINKER_FLAG_LINK_PATH,
    KEFIR_DRIVER_LINKER_FLAG_EXTRA
} kefir_driver_linker_flag_type_t;

typedef struct kefir_driver_input_file {
    const char *file;
    kefir_driver_input_file_type_t type;
} kefir_driver_input_file_t;

typedef struct kefir_driver_linker_flag {
    const char *flag;
    kefir_driver_linker_flag_type_t type;
} kefir_driver_linker_flag_t;

typedef struct kefir_driver_definition {
    const char *name;
    const char *value;
} kefir_driver_definition_t;

typedef struct kefir_driver_configuration {
    kefir_driver_stage_t stage;
    const char *output_file;
    struct kefir_list input_files;
    struct kefir_list assembler_flags;
    struct kefir_list linker_flags;
    struct kefir_list defines;
    struct kefir_list undefines;
    struct kefir_list include_directories;
    struct kefir_list include_files;
    struct {
        kefir_bool_t strip;
    } flags;
} kefir_driver_configuration_t;

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *);
kefir_result_t kefir_driver_configuration_free(struct kefir_mem *, struct kefir_driver_configuration *);

kefir_result_t kefir_driver_configuration_add_input(struct kefir_mem *, struct kefir_symbol_table *,
                                                    struct kefir_driver_configuration *, const char *,
                                                    kefir_driver_input_file_type_t);
kefir_result_t kefir_driver_configuration_add_assembler_extra_flag(struct kefir_mem *, struct kefir_symbol_table *,
                                                                   struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_linker_flag(struct kefir_mem *, struct kefir_symbol_table *,
                                                          struct kefir_driver_configuration *, const char *,
                                                          kefir_driver_linker_flag_type_t);
kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *, struct kefir_symbol_table *,
                                                     struct kefir_driver_configuration *, const char *, const char *);
kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *, struct kefir_symbol_table *,
                                                       struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *, struct kefir_symbol_table *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *, struct kefir_symbol_table *,
                                                           struct kefir_driver_configuration *, const char *);

kefir_result_t kefir_driver_run(struct kefir_mem *, struct kefir_driver_configuration *,
                                const struct kefir_driver_external_resources *);

#endif
