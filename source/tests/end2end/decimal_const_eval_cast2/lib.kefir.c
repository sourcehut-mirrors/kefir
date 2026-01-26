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

#line __LINE__ "decimal_const_eval_cast1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 a[] = {0.0f,
                  3.148241f,
                  0.0,
                  67582.47284,
                  0.0L,
                  84824819841.852847184L,
                  __builtin_nan(""),
                  __builtin_inf(),
                  84814991ll,
                  58294919481941ull,
                  592948.48294if,
                  592948.48294i,
                  592948.48294il,
                  30319401.3819df,
                  8924958294949.24818dd,
                  949249919.248941841984dl};
_Decimal64 b[] = {0.0f,
                  3.148241f,
                  0.0,
                  67582.47284,
                  0.0L,
                  84824819841.852847184L,
                  __builtin_nan(""),
                  __builtin_inf(),
                  84814991ll,
                  58294919481941ull,
                  592948.48294if,
                  592948.48294i,
                  592948.48294il,
                  30319401.3819df,
                  8924958294949.24818dd,
                  949249919.248941841984dl};
_Decimal128 c[] = {0.0f,
                   3.148241f,
                   0.0,
                   67582.47284,
                   0.0L,
                   84824819841.852847184L,
                   __builtin_nan(""),
                   __builtin_inf(),
                   84814991ll,
                   58294919481941ull,
                   592948.48294if,
                   592948.48294i,
                   592948.48294il,
                   30319401.3819df,
                   8924958294949.24818dd,
                   949249919.248941841984dl};
#else
int a[] = {847249408,  800066001, 847249408, 837230439, 847249408, 1830907570, 2080374784, 2013265920,
           1824615131, 911799156, 847249408, 847249408, 847249408, 864329818,  1835544350, 1827723283};
int b[] = {0,        834666496, -49648252,   803942222, 0,          834666496,  279627008,   813171351,
           0,        834666496, -1116003387, 826155717, 0,          2080374784, 0,           2013265920,
           84814991, 834666496, -671626667,  834680068, 0,          834666496,  0,           834666496,
           0,        834666496, 303194,      838860800, -926007936, 830453043,  -1301928502, 1815722336};
int c[] = {0,          0,       0, 809500672,  1733689461,  -1259864708, 1,          807010304,
           0,          0,       0, 809500672,  -1608694571, 22930925,    314059257,  805784885,
           0,          0,       0, 809500672,  1175828299,  -438829485,  -108812803, 806593079,
           0,          0,       0, 2080374784, 0,           0,           0,          2013265920,
           84814991,   0,       0, 809500672,  -671626667,  13572,       0,          809500672,
           0,          0,       0, 809500672,  0,           0,           0,          809500672,
           0,          0,       0, 809500672,  303194,      0,           0,          809762816,
           -926007936, 2078003, 0, 809107456,  493485632,   1971137591,  51,         807927808};
#endif
