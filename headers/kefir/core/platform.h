/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_CORE_PLATFORM_H_
#define KEFIR_CORE_PLATFORM_H_

#include "kefir/core/base.h"

#if defined(__linux__)
#define KEFIR_LINUX_HOST_PLATFORM
#elif defined(__FreeBSD__)
#define KEFIR_FREEBSD_HOST_PLATFORM
#elif defined(__OpenBSD__)
#define KEFIR_OPENBSD_HOST_PLATFORM
#elif defined(__NetBSD__)
#define KEFIR_NETBSD_HOST_PLATFORM
#elif defined(__DragonFly__)
#define KEFIR_DRAGONFLYBSD_HOST_PLATFORM
#elif defined(__unix__)
#define KEFIR_UNIX_HOST_PLATFORM
#endif

#if defined(__STDC_IEC_60559_DFP__) ||                                                                                 \
    (((defined(__GNUC__) && !defined(__clang__)) || (defined(__KEFIRCC__) && defined(__KEFIRCC_DECIMAL_SUPPORT__))) && \
     (defined(__DECIMAL_BID_FORMAT__) || defined(__DECIMAL_DPD_FORMAT__) || defined(KEFIR_PLATFORM_DECIMAL_BID) ||     \
      defined(KEFIR_PLATFORM_DECIMAL_DPD)))
#define KEFIR_PLATFORM_HAS_DECIMAL_FP

#if !defined(KEFIR_PLATFORM_DECIMAL_BID) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
#ifndef __DECIMAL_DPD_FORMAT__
#define KEFIR_PLATFORM_DECIMAL_BID
#else
#define KEFIR_PLATFORM_DECIMAL_DPD
#endif
#endif

#if (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && defined(KEFIR_PLATFORM_DECIMAL_BID)
#define KEFIR_PLATFORM_HAS_DECIMAL_FP_BITINT_CONV
#endif
#endif

#endif
