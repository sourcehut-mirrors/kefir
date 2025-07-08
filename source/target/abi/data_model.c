/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include "kefir/core/data_model.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define DEFINE_MIN_MAX(_name)                                                               \
    kefir_int64_t kefir_data_model_descriptor_signed_##_name##_min(                         \
        const struct kefir_data_model_descriptor *data_model) {                             \
        return -kefir_data_model_descriptor_signed_##_name##_max(data_model) - 1;           \
    }                                                                                       \
                                                                                            \
    kefir_int64_t kefir_data_model_descriptor_signed_##_name##_max(                         \
        const struct kefir_data_model_descriptor *data_model) {                             \
        return (kefir_int64_t) ((1ull << (data_model->scalar_width._name##_bits - 1)) - 1); \
    }                                                                                       \
                                                                                            \
    kefir_uint64_t kefir_data_model_descriptor_unsigned_##_name##_max(                      \
        const struct kefir_data_model_descriptor *data_model) {                             \
        if (data_model->scalar_width._name##_bits < sizeof(kefir_uint64_t) * CHAR_BIT) {    \
            return (1ull << data_model->scalar_width._name##_bits) - 1;                     \
        } else {                                                                            \
            return ~0ull;                                                                   \
        }                                                                                   \
    }

DEFINE_MIN_MAX(char)
DEFINE_MIN_MAX(short)
DEFINE_MIN_MAX(int)
DEFINE_MIN_MAX(long)
DEFINE_MIN_MAX(long_long)

#undef DEFINE_MIN_MAX
