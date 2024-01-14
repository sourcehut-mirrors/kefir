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

#ifndef KEFIR_CORE_ERROR_H_
#define KEFIR_CORE_ERROR_H_

#include "kefir/core/basic-types.h"
#include <stddef.h>

#define KEFIR_ERROR_STACK_SIZE 32
#define KEFIR_ERROR_PAYLOAD_LENGTH 256

typedef enum kefir_error_payload_type {
    KEFIR_ERROR_PAYLOAD_NONE,
    KEFIR_ERROR_PAYLOAD_SOURCE_LOCATION,
    KEFIR_ERROR_PAYLOAD_STRING,
} kefir_error_payload_type_t;

typedef struct kefir_error {
    kefir_result_t code;
    const char *message;
    const char *file;
    unsigned int line;

    kefir_error_payload_type_t payload_type;
    _Alignas(max_align_t) char payload[KEFIR_ERROR_PAYLOAD_LENGTH];

    kefir_bool_t error_overflow;
    const struct kefir_error *prev_error;
} kefir_error_t;

const struct kefir_error *kefir_current_error(void);
void kefir_clear_error(void);
kefir_result_t kefir_set_error(kefir_result_t, const char *, const char *, unsigned int, struct kefir_error **);

#define KEFIR_SET_ERROR(code, message) kefir_set_error((code), (message), __FILE__, __LINE__, NULL)

kefir_result_t kefir_set_errorf(kefir_result_t, const char *, const char *, unsigned int, struct kefir_error **, ...);
#define KEFIR_SET_ERRORF(code, message, ...) kefir_set_errorf((code), (message), __FILE__, __LINE__, NULL, __VA_ARGS__)

#endif
