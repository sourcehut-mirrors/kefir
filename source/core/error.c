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

#include "kefir/core/error.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static _Thread_local kefir_size_t next_error_index = 0;
static _Thread_local struct kefir_error error_stack[KEFIR_ERROR_STACK_SIZE];

const struct kefir_error *kefir_current_error(void) {
    if (next_error_index != 0) {
        return &error_stack[next_error_index - 1];
    } else {
        return NULL;
    }
}

void kefir_clear_error(void) {
    next_error_index = 0;
}

kefir_result_t kefir_set_error(kefir_result_t code, const char *message, const char *file, unsigned int line,
                               struct kefir_error **error_ptr) {
    if (next_error_index == KEFIR_ERROR_STACK_SIZE || kefir_result_get_category(code) == KEFIR_RESULT_CATEGORY_NORMAL) {
        return code;
    }

    if (next_error_index > 0 &&
        kefir_result_get_category(error_stack[next_error_index - 1].code) == KEFIR_RESULT_CATEGORY_WARNING) {
        // Override warning
        next_error_index--;
    }

    struct kefir_error *current_error = &error_stack[next_error_index];
    current_error->code = code;
    current_error->message = message;
    current_error->file = file;
    current_error->line = line;
    if (next_error_index != 0) {
        current_error->prev_error = &error_stack[next_error_index - 1];
    } else {
        current_error->prev_error = NULL;
    }
    current_error->error_overflow = ++next_error_index == KEFIR_ERROR_STACK_SIZE;
    memset(current_error->payload, 0, KEFIR_ERROR_PAYLOAD_LENGTH);
    current_error->payload_type = KEFIR_ERROR_PAYLOAD_NONE;
    if (error_ptr != NULL) {
        *error_ptr = current_error;
    }
    return code;
}

kefir_result_t kefir_set_errorf(kefir_result_t code, const char *fmt, const char *file, unsigned int line,
                                struct kefir_error **error_ptr, ...) {
    struct kefir_error *error = NULL;
    kefir_result_t res = kefir_set_error(code, fmt, file, line, &error);
    if (error == NULL) {
        return res;
    }

    va_list args;
    va_start(args, error_ptr);
    vsnprintf(error->payload, KEFIR_ERROR_PAYLOAD_LENGTH, fmt, args);
    va_end(args);
    error->message = error->payload;

    if (error_ptr != NULL) {
        *error_ptr = error;
    }
    return res;
}
