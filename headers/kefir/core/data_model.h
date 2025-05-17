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

#ifndef KEFIR_CORE_DATA_MODEL_H_
#define KEFIR_CORE_DATA_MODEL_H_

#include "kefir/core/base.h"
#include "kefir/core/basic-types.h"

typedef enum kefir_byte_order {
    KEFIR_BYTE_ORDER_UNKNOWN = 0,
    KEFIR_BYTE_ORDER_BIG_ENDIAN,
    KEFIR_BYTE_ORDER_LITTLE_ENDIAN
} kefir_byte_order_t;

typedef enum kefir_data_model_tag {
    KEFIR_DATA_MODEL_UNKNOWN = 0,
    KEFIR_DATA_MODEL_ILP32,
    KEFIR_DATA_MODEL_LLP64,
    KEFIR_DATA_MODEL_LP64,
    KEFIR_DATA_MODEL_ILP64,
    KEFIR_DATA_MODEL_SILP64
} kefir_data_model_tag_t;

typedef struct kefir_data_model_descriptor {
    kefir_data_model_tag_t model;
    kefir_byte_order_t byte_order;
    struct {
        kefir_size_t bool_bits;
        kefir_size_t char_bits;
        kefir_size_t short_bits;
        kefir_size_t int_bits;
        kefir_size_t long_bits;
        kefir_size_t long_long_bits;
        kefir_size_t float_bits;
        kefir_size_t double_bits;
        kefir_size_t long_double_bits;
    } scalar_width;
} kefir_data_model_descriptor_t;

#endif
