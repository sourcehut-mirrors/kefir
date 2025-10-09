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

#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"
#include "kefir/ast/type.h"
#include "kefir/ast/type_conv.h"

DEFINE_CASE(ast_interchange_float_types1, "AST Type analysis - interchange/extended float types #1") {
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_string_pool strings;

    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    ASSERT_OK(kefir_string_pool_init(&strings));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &strings));

    const struct kefir_ast_type *type32 = kefir_ast_type_interchange_float32();
    ASSERT(type32->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32);

    const struct kefir_ast_type *type64 = kefir_ast_type_interchange_float64();
    ASSERT(type64->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64);

    const struct kefir_ast_type *type80 = kefir_ast_type_interchange_float80();
    ASSERT(type80->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80);

    const struct kefir_ast_type *type32x = kefir_ast_type_extended_float32();
    ASSERT(type32x->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32);

    const struct kefir_ast_type *type64x = kefir_ast_type_extended_float64();
    ASSERT(type64x->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64);

    ASSERT(KEFIR_AST_TYPE_SAME(type32, type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_float(), type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_double(), type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(), type32));

    ASSERT(KEFIR_AST_TYPE_SAME(type64, type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_float(), type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_double(), type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(), type64));

    ASSERT(KEFIR_AST_TYPE_SAME(type80, type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_float(), type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_double(), type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type80, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(), type80));

    ASSERT(KEFIR_AST_TYPE_SAME(type32x, type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_float(), type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_double(), type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type32x, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(), type32x));

    ASSERT(KEFIR_AST_TYPE_SAME(type64x, type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, type32));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, type64));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, type80));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, type32x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_float(), type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_double(), type64x));
    ASSERT(!KEFIR_AST_TYPE_SAME(type64x, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(), type64x));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_float(), type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_double(), type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_long_double(), type32));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_float(), type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_double(), type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_long_double(), type64));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_float(), type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_double(), type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type80, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_long_double(), type80));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_float(), type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_double(), type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type32x, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_long_double(), type32x));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, type32));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, type64));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, type80));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, type32x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, kefir_ast_type_float()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_float(), type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, kefir_ast_type_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_double(), type64x));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, type64x, kefir_ast_type_long_double()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, kefir_ast_type_long_double(), type64x));

    ASSERT(KEFIR_AST_TYPE_SAME(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, type32), type32));
    ASSERT(KEFIR_AST_TYPE_SAME(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, type64), type64));
    ASSERT(KEFIR_AST_TYPE_SAME(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, type80), type80));
    ASSERT(KEFIR_AST_TYPE_SAME(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, type32x), type32x));
    ASSERT(KEFIR_AST_TYPE_SAME(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, type64x), type64x));

    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, kefir_ast_type_float()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_float(), type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, kefir_ast_type_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_double(), type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32, kefir_ast_type_long_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_long_double(), type32) == NULL);

    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, kefir_ast_type_float()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_float(), type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, kefir_ast_type_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_double(), type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64, kefir_ast_type_long_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_long_double(), type64) == NULL);

    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, kefir_ast_type_float()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_float(), type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, kefir_ast_type_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_double(), type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type80, kefir_ast_type_long_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_long_double(), type80) == NULL);

    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, kefir_ast_type_float()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_float(), type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, kefir_ast_type_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_double(), type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type32x, kefir_ast_type_long_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_long_double(), type32x) == NULL);

    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, type32) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, type64) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, type80) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, type32x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, kefir_ast_type_float()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_float(), type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, kefir_ast_type_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_double(), type64x) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, type64x, kefir_ast_type_long_double()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, kefir_ast_type_long_double(), type64x) == NULL);

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &strings));
}
END_CASE

DEFINE_CASE(ast_interchange_float_types2, "AST Type analysis - interchange/extended float types #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0})));

    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0})));

    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0})));

    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0})));

    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_float(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_double(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_long_double(),
        kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
            kefir_ast_type_long_double(), (struct kefir_ast_bitfield_properties){0})));

    const struct kefir_ast_type *int_types[] = {
        kefir_ast_type_boolean(),
        kefir_ast_type_char(),
        kefir_ast_type_signed_char(),
        kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_short(),
        kefir_ast_type_unsigned_short(),
        kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_int(),
        kefir_ast_type_signed_long(),
        kefir_ast_type_unsigned_long(),
        kefir_ast_type_signed_long_long(),
        kefir_ast_type_unsigned_long_long()
    };
    for (kefir_size_t i = 0; i < sizeof(int_types) / sizeof(int_types[0]); i++) {
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float32(),
            kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
                int_types[i], (struct kefir_ast_bitfield_properties){0})));
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float32(),
            kefir_ast_type_common_arithmetic(type_traits, int_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0})));

        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
            kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
                int_types[i], (struct kefir_ast_bitfield_properties){0})));
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float64(),
            kefir_ast_type_common_arithmetic(type_traits, int_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0})));

        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
            kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
                int_types[i], (struct kefir_ast_bitfield_properties){0})));
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_interchange_float80(),
            kefir_ast_type_common_arithmetic(type_traits, int_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0})));

        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
            kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
                int_types[i], (struct kefir_ast_bitfield_properties){0})));
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float32(),
            kefir_ast_type_common_arithmetic(type_traits, int_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0})));

        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
            kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
                int_types[i], (struct kefir_ast_bitfield_properties){0})));
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_extended_float64(),
            kefir_ast_type_common_arithmetic(type_traits, int_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0})));
    }

    const struct kefir_ast_type *decimal_types[] = {
        kefir_ast_type_decimal32(),
        kefir_ast_type_decimal64(),
        kefir_ast_type_decimal128()
    };
    for (kefir_size_t i = 0; i < sizeof(decimal_types) / sizeof(decimal_types[0]); i++) {
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0},
                decimal_types[i], (struct kefir_ast_bitfield_properties){0}) == NULL);
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, decimal_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float32(), (struct kefir_ast_bitfield_properties){0}) == NULL);

        ASSERT(kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0},
                decimal_types[i], (struct kefir_ast_bitfield_properties){0}) == NULL);
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, decimal_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float64(), (struct kefir_ast_bitfield_properties){0}) == NULL);

        ASSERT(kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0},
                decimal_types[i], (struct kefir_ast_bitfield_properties){0}) == NULL);
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, decimal_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_interchange_float80(), (struct kefir_ast_bitfield_properties){0}) == NULL);

        ASSERT(kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0},
                decimal_types[i], (struct kefir_ast_bitfield_properties){0}) == NULL);
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, decimal_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_extended_float32(), (struct kefir_ast_bitfield_properties){0}) == NULL);

        ASSERT(kefir_ast_type_common_arithmetic(type_traits, kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0},
                decimal_types[i], (struct kefir_ast_bitfield_properties){0}) == NULL);
        ASSERT(kefir_ast_type_common_arithmetic(type_traits, decimal_types[i], (struct kefir_ast_bitfield_properties){0},
                kefir_ast_type_extended_float64(), (struct kefir_ast_bitfield_properties){0}) == NULL);
    }
}
END_CASE
