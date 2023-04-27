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

#include "kefir/optimizer/type.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_type_descriptor_init(struct kefir_mem *mem,
                                              const struct kefir_ir_target_platform *target_platform,
                                              kefir_id_t ir_type_id, const struct kefir_ir_type *ir_type,
                                              struct kefir_opt_type_descriptor *type_descr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target_platform != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR target platform"));
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(type_descr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer type descriptor"));

    REQUIRE_OK(KEFIR_IR_TARGET_PLATFORM_GET_TYPE(mem, target_platform, ir_type, &type_descr->target_type));
    type_descr->ir_type_id = ir_type_id;
    type_descr->ir_type = ir_type;
    type_descr->target_platform = target_platform;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_type_descriptor_free(struct kefir_mem *mem, struct kefir_opt_type_descriptor *type_descr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_descr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer type descriptor"));

    REQUIRE_OK(KEFIR_IR_TARGET_PLATFORM_FREE_TYPE(mem, type_descr->target_platform, type_descr->target_type));
    memset(type_descr, 0, sizeof(struct kefir_opt_type_descriptor));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_type_descriptor_entry_info(struct kefir_mem *mem,
                                                    const struct kefir_opt_type_descriptor *type_descr,
                                                    kefir_size_t typeentry_index,
                                                    struct kefir_ir_target_platform_typeentry_info *typeeentry_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_descr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer type descriptor"));

    REQUIRE_OK(KEFIR_IR_TARGET_PLATFORM_TYPEENTRY_INFO(mem, type_descr->target_platform, type_descr->target_type,
                                                       typeentry_index, typeeentry_info));
    return KEFIR_OK;
}
