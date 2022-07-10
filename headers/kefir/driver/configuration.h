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

#ifndef KEFIR_DRIVER_CONFIGURATION_H_
#define KEFIR_DRIVER_CONFIGURATION_H_

#include "kefir/core/list.h"
#include "kefir/core/symbol_table.h"
#include "kefir/driver/target.h"

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
    KEFIR_DRIVER_LINKER_FLAG_ENTRY_POINT,
    KEFIR_DRIVER_LINKER_FLAG_STRIP,
    KEFIR_DRIVER_LINKER_FLAG_RETAIN_RELOC,
    KEFIR_DRIVER_LINKER_FLAG_UNDEFINED_SYMBOL,
    KEFIR_DRIVER_LINKER_FLAG_STATIC,
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
    struct kefir_list compiler_flags;
    struct kefir_list defines;
    struct kefir_list undefines;
    struct kefir_list include_directories;
    struct kefir_list include_files;
    struct kefir_driver_target target;

    struct {
        kefir_bool_t restrictive_mode;
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
kefir_result_t kefir_driver_configuration_add_compiler_flag(struct kefir_mem *, struct kefir_symbol_table *,
                                                            struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *, struct kefir_symbol_table *,
                                                     struct kefir_driver_configuration *, const char *, const char *);
kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *, struct kefir_symbol_table *,
                                                       struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *, struct kefir_symbol_table *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *, struct kefir_symbol_table *,
                                                           struct kefir_driver_configuration *, const char *);

#endif
