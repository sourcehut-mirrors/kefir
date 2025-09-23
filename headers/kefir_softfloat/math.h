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

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#include "kefir_softfloat/base.h"

union __kefir_softfloat_float_repr {
    __KEFIR_SOFTFLOAT_FLOAT_T__ f;
    __KEFIR_SOFTFLOAT_UINT64_T__ uint;
};

union __kefir_softfloat_double_repr {
    __KEFIR_SOFTFLOAT_DOUBLE_T__ d;
    __KEFIR_SOFTFLOAT_UINT64_T__ uint;
};

static __KEFIR_SOFTFLOAT_FLOAT_T__ __kefir_softfloat_fabsf(__KEFIR_SOFTFLOAT_FLOAT_T__ value) {
    union __kefir_softfloat_float_repr rep = {
        .f = value
    };

    rep.uint &= (1ull << 31) - 1;
    return rep.f;
}

static __KEFIR_SOFTFLOAT_INT_T__ __kefir_softfloat_ilogbf(__KEFIR_SOFTFLOAT_FLOAT_T__ x) {
#pragma STDC FENV_ACCESS ON
#define MANT_WIDTH 23
#define EXP_SIGN_WIDTH 9
#define EXP_MASK 0xff
#define EXP_OFFSET 0x7f
#define LOGNAN (-0x80000000)
	union __kefir_softfloat_float_repr repr = {
        .f = x
    };
	__KEFIR_SOFTFLOAT_INT_T__ exponent = ((repr.uint) >> MANT_WIDTH) & EXP_MASK;
    __KEFIR_SOFTFLOAT_UINT64_T__ irepr = repr.uint << EXP_SIGN_WIDTH;

    __KEFIR_SOFTFLOAT_INT_T__ res = exponent - EXP_OFFSET;
	if (exponent == EXP_MASK) {
        volatile __KEFIR_SOFTFLOAT_FLOAT_T__ tmp = 0.0 / 0.0;
        (void) tmp;
        res = irepr ? LOGNAN : __KEFIR_SOFTFLOAT_INT_MAX__;
	} else if (exponent == 0) {
		if (irepr == 0) {
            volatile __KEFIR_SOFTFLOAT_FLOAT_T__ tmp = 0.0 / 0.0;
            (void) tmp;
            res = LOGNAN;
		} else {
            res = -EXP_OFFSET;
            for (; irepr >> 31 == 0; irepr <<= 1) {
                res--;
            }
        }
	}
    return res;
#undef EXP_OFFSET
#undef LOGNAN
#undef EXP_SIGN_WIDTH
#undef MANT_WIDTH
#undef EXP_MASK
}

static __KEFIR_SOFTFLOAT_FLOAT_T__ __kefir_softfloat_scalbnf(__KEFIR_SOFTFLOAT_FLOAT_T__ x, __KEFIR_SOFTFLOAT_INT_T__ n) {
	union __kefir_softfloat_float_repr rep;

#define UP_THRESH 127
#define UP_FACTOR 0x1p127f
#define DOWN_THRESH 126
#define DOWN_OFFSET 24
#define DOWN_FACTOR (0x1p-126f * 0x1p24f)
#define EXP_OFFSET 23
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n > UP_THRESH; i++) {
		x *= UP_FACTOR;
		n -= UP_THRESH;
    }
    if (n > UP_THRESH) {
        n = UP_THRESH;
    }
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n < -DOWN_THRESH; i++) {
		x *= DOWN_FACTOR;
		n += DOWN_THRESH - DOWN_OFFSET;
    }
    if (n < -DOWN_THRESH) {
        n = -DOWN_THRESH;
    }

    rep.uint = ((__KEFIR_SOFTFLOAT_UINT64_T__) (UP_THRESH + n)) << EXP_OFFSET;
	return x * rep.f;
#undef EXP_OFFSET
#undef UP_THRESH
#undef UP_FACTOR
#undef DOWN_THRESH
#undef DOWN_OFFSET
#undef DOWN_FACTOR
}

static __KEFIR_SOFTFLOAT_FLOAT_T__ __kefir_softfloat_logbf(__KEFIR_SOFTFLOAT_FLOAT_T__ x) {
    if (!__KEFIR_SOFTFLOAT_ISFINITE__(x)) {
        return x * x;
    } else if (x == 0.0) {
		return -__KEFIR_SOFTFLOAT_INFINITY__;
    } else {
	    return (__KEFIR_SOFTFLOAT_FLOAT_T__) __kefir_softfloat_ilogbf(x);
    }
}

static __KEFIR_SOFTFLOAT_FLOAT_T__ __kefir_softfloat_fmaximum_numf(__KEFIR_SOFTFLOAT_FLOAT_T__ x, __KEFIR_SOFTFLOAT_FLOAT_T__ y) {
    if (__KEFIR_SOFTFLOAT_ISLESS__(x, y)) {
        return y;
    } else if (__KEFIR_SOFTFLOAT_ISGREATER__(x, y)) {
        return x;
    } else if (x == y) {
        return __KEFIR_SOFTFLOAT_COPYSIGNF__(1.0f, x) >= __KEFIR_SOFTFLOAT_COPYSIGNF__(1.0f, y) ? x : y;
    }

    return __KEFIR_SOFTFLOAT_ISNAN__(y)
        ? (__KEFIR_SOFTFLOAT_ISNAN__(x)
            ? x + y
            : x)
        : y;
}

static __KEFIR_SOFTFLOAT_DOUBLE_T__ __kefir_softfloat_fabs(__KEFIR_SOFTFLOAT_DOUBLE_T__ value) {
    union __kefir_softfloat_double_repr rep = {
        .d = value
    };

    rep.uint &= (1ull << 63) - 1;
    return rep.d;
}

static __KEFIR_SOFTFLOAT_INT_T__ __kefir_softfloat_ilogb(__KEFIR_SOFTFLOAT_DOUBLE_T__ x) {
#pragma STDC FENV_ACCESS ON
#define MANT_WIDTH 52
#define EXP_SIGN_WIDTH 12
#define EXP_MASK 0x7ff
#define EXP_OFFSET 0x3ff
#define LOGNAN (-0x80000000)
	union __kefir_softfloat_double_repr repr = {
        .d = x
    };
	__KEFIR_SOFTFLOAT_INT_T__ exponent = ((repr.uint) >> MANT_WIDTH) & EXP_MASK;
    __KEFIR_SOFTFLOAT_UINT64_T__ irepr = repr.uint << EXP_SIGN_WIDTH;

    __KEFIR_SOFTFLOAT_INT_T__ res = exponent - EXP_OFFSET;
	if (exponent == EXP_MASK) {
        volatile __KEFIR_SOFTFLOAT_DOUBLE_T__ tmp = 0.0 / 0.0;
        (void) tmp;
        res = irepr ? LOGNAN : __KEFIR_SOFTFLOAT_INT_MAX__;
	} else if (exponent == 0) {
		if (irepr == 0) {
            volatile __KEFIR_SOFTFLOAT_DOUBLE_T__ tmp = 0.0 / 0.0;
            (void) tmp;
            res = LOGNAN;
		} else {
            res = -EXP_OFFSET;
            for (; irepr >> 63 == 0; irepr <<= 1) {
                res--;
            }
        }
	}
    return res;
#undef EXP_OFFSET
#undef LOGNAN
#undef EXP_SIGN_WIDTH
#undef MANT_WIDTH
#undef EXP_MASK
}

static __KEFIR_SOFTFLOAT_DOUBLE_T__ __kefir_softfloat_scalbn(__KEFIR_SOFTFLOAT_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_INT_T__ n) {
	union __kefir_softfloat_double_repr rep;

#define UP_THRESH 1023
#define UP_FACTOR 0x1p1023
#define DOWN_THRESH 1022
#define DOWN_OFFSET 53
#define DOWN_FACTOR (0x1p-1022 * 0x1p53)
#define EXP_OFFSET 52
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n > UP_THRESH; i++) {
		x *= UP_FACTOR;
		n -= UP_THRESH;
    }
    if (n > UP_THRESH) {
        n = UP_THRESH;
    }
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n < -DOWN_THRESH; i++) {
		x *= DOWN_FACTOR;
		n += DOWN_THRESH - DOWN_OFFSET;
    }
    if (n < -DOWN_THRESH) {
        n = -DOWN_THRESH;
    }

    rep.uint = ((__KEFIR_SOFTFLOAT_UINT64_T__) (UP_THRESH + n)) << EXP_OFFSET;
	return x * rep.d;
#undef EXP_OFFSET
#undef UP_THRESH
#undef UP_FACTOR
#undef DOWN_THRESH
#undef DOWN_OFFSET
#undef DOWN_FACTOR
}

static __KEFIR_SOFTFLOAT_DOUBLE_T__ __kefir_softfloat_logb(__KEFIR_SOFTFLOAT_DOUBLE_T__ x) {
    if (!__KEFIR_SOFTFLOAT_ISFINITE__(x)) {
        return x * x;
    } else if (x == 0.0) {
		return -__KEFIR_SOFTFLOAT_INFINITY__;
    } else {
	    return (__KEFIR_SOFTFLOAT_DOUBLE_T__) __kefir_softfloat_ilogb(x);
    }
}

static __KEFIR_SOFTFLOAT_DOUBLE_T__ __kefir_softfloat_fmaximum_num(__KEFIR_SOFTFLOAT_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_DOUBLE_T__ y) {
    if (__KEFIR_SOFTFLOAT_ISLESS__(x, y)) {
        return y;
    } else if (__KEFIR_SOFTFLOAT_ISGREATER__(x, y)) {
        return x;
    } else if (x == y) {
        return __KEFIR_SOFTFLOAT_COPYSIGN__(1.0, x) >= __KEFIR_SOFTFLOAT_COPYSIGN__(1.0, y) ? x : y;
    }

    return __KEFIR_SOFTFLOAT_ISNAN__(y)
        ? (__KEFIR_SOFTFLOAT_ISNAN__(x)
            ? x + y
            : x)
        : y;
}

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
	rep.ld = 1.0L;
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n > UP_THRESH; i++) {
		x *= UP_FACTOR;
		n -= UP_THRESH;
    }
    if (n > UP_THRESH) {
        n = UP_THRESH;
    }
    for (__KEFIR_SOFTFLOAT_INT_T__ i = 0; i < 2 && n < -DOWN_THRESH; i++) {
		x *= DOWN_FACTOR;
		n += DOWN_THRESH - DOWN_OFFSET;
    }
    if (n < -DOWN_THRESH) {
        n = -DOWN_THRESH;
    }

	rep.uint[1] = UP_THRESH + n;
#undef UP_THRESH
#undef UP_FACTOR
#undef DOWN_THRESH
#undef DOWN_OFFSET
#undef DOWN_FACTOR
	return x * rep.ld;
}

static __KEFIR_SOFTFLOAT_INT_T__ __kefir_softfloat_ilogbl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x) {
#pragma STDC FENV_ACCESS ON 
	union __kefir_softfloat_long_double_repr rep = {
        .ld = x
    };
#define EXP_MAX 0x7fff
#define EXP_OFFSET 0x3fff
#define MNT_BITS 64
#define MNT_MASK (1ull << (MNT_BITS - 1))
#define LOGNAN (-0x80000000)
	__KEFIR_SOFTFLOAT_UINT64_T__ mantissa = rep.uint[0],
	                             exponent = rep.uint[1] & EXP_MAX;

    __KEFIR_SOFTFLOAT_INT_T__ res = exponent;
    if (exponent == 0 && mantissa == 0) {
        volatile __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ tmp = 0.0L / 0.0L;
        (void) tmp;
        res = LOGNAN;
    } else if (exponent == 0) {
        __KEFIR_SOFTFLOAT_INT_T__ i = 0;
		while (!(mantissa & MNT_MASK)) {
            mantissa <<= 1;
            i++;
        }
		res = -EXP_OFFSET + 1 - i;
	} else if (exponent == EXP_MAX) {
        volatile __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ tmp = 0.0L / 0.0L;
        (void) tmp;
		res = (mantissa & MNT_MASK) ? LOGNAN : __KEFIR_SOFTFLOAT_INT_MAX__;
	} else {
	    res -= EXP_OFFSET;
    }
    return res;
#undef LOGNAN
#undef MNT_BITS
#undef EXP_OFFSET
#undef EXP_MAX
}

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_logbl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x) {
    if (!__KEFIR_SOFTFLOAT_ISFINITE__(x)) {
        return x * x;
    } else if (x == 0.0L) {
		return -__KEFIR_SOFTFLOAT_INFINITY__;
    } else {
	    return (__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__) __kefir_softfloat_ilogbl(x);
    }
}
#else
static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_fabsl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ value) {
    return (__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__) __kefir_softfloat_fabs((__KEFIR_SOFTFLOAT_DOUBLE_T__) value);
}

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_scalbnl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_INT_T__ n) {
    return (__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__) __kefir_softfloat_scalbn((__KEFIR_SOFTFLOAT_DOUBLE_T__) x, n);
}

static __KEFIR_SOFTFLOAT_INT_T__ __kefir_softfloat_ilogbl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x) {
    return __kefir_softfloat_ilogb((__KEFIR_SOFTFLOAT_DOUBLE_T__) x);
}

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_logbl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x) {
    return (__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__) __kefir_softfloat_logb((__KEFIR_SOFTFLOAT_DOUBLE_T__) x);
}
#endif

static __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ __kefir_softfloat_fmaximum_numl(__KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x, __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ y) {
    if (__KEFIR_SOFTFLOAT_ISLESS__(x, y)) {
        return y;
    } else if (__KEFIR_SOFTFLOAT_ISGREATER__(x, y)) {
        return x;
    } else if (x == y) {
        return __KEFIR_SOFTFLOAT_COPYSIGNL__(1.0L, x) >= __KEFIR_SOFTFLOAT_COPYSIGNL__(1.0L, y) ? x : y;
    }

    return __KEFIR_SOFTFLOAT_ISNAN__(y)
        ? (__KEFIR_SOFTFLOAT_ISNAN__(x)
            ? x + y
            : x)
        : y;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
