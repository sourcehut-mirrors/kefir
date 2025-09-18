/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2025 Jevgenijs Protopopovs

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software without
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __KEFIR_SOFTFLOAT_MATH_H__
#define __KEFIR_SOFTFLOAT_MATH_H__

#include "kefir_softfloat/base.h"

#if __KEFIR_SOFTFLOAT_LDBL_MANT_DIG__ == 64
union __kefir_softfloat_long_double_repr {
    __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ ld;
    __KEFIR_SOFTFLOAT_UINT64_T__ uint[2];
};

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_fabsl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ value) {
    union __kefir_softfloat_long_double_repr rep = {
        .ld = value
    };

    rep.uint[1] &= 0x7fff;
    return rep.ld;
}

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_scalbnl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_INT_T__ n) {
	union __kefir_softfloat_long_double_repr rep;

#define UP_THRESH 0x3fff
#define UP_FACTOR 0x1p16383L
#define DOWN_THRESH 0x3ffe
#define DOWN_OFFSET 0x71
#define DOWN_FACTOR (0x1p-16382L * 0x1p113L)
	if (n > UP_THRESH) {
		x *= UP_FACTOR;
		n -= UP_THRESH;
		if (n > UP_THRESH) {
			x *= UP_FACTOR;
			n -= UP_THRESH;
			if (n > UP_THRESH) {
			    n = UP_THRESH;
            }
		}
	} else if (n < -DOWN_THRESH) {
		x *= DOWN_FACTOR;
		n += DOWN_THRESH - DOWN_OFFSET;
		if (n < -DOWN_THRESH) {
			x *= DOWN_FACTOR;
			n += DOWN_THRESH - DOWN_OFFSET;
			if (n < -DOWN_THRESH) {
				n = -DOWN_THRESH;
            }
		}
	}

	rep.ld = 1.0L;
	rep.uint[1] = UP_THRESH + n;
#undef UP_THRESH
#undef UP_FACTOR
#undef DOWN_THRESH
#undef DOWN_OFFSET
#undef DOWN_FACTOR
	return x * rep.ld;
}
#endif

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_fmaximum_numl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ y) {
    if (__KEFIR_SOFTFLOAT_ISGREATER__(x, y)) {
        return x;
    } else if (__KEFIR_SOFTFLOAT_ISLESS__(x, y)) {
        return y;
    } else if (x == y) {
        return __KEFIR_SOFTFLOAT_COPYSIGNL__(1.0L, x) >= __KEFIR_SOFTFLOAT_COPYSIGNL__(1.0L, y) ? x : y;
    } else if (__KEFIR_SOFTFLOAT_ISNAN__(y)) {
        return __KEFIR_SOFTFLOAT_ISNAN__(x) ? x + y : x;
    } else {
        return y;
    }
}

#endif
