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

#line __LINE__ "decimal_const_eval_cast3"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 a[] = {
    10000000000000000000000000000000000000000000000000000000uwb,
    20000000000000000000000000000000000000000000000000000000wb,
    -30000000000000000000000000000000000000000000000000000000wb,
    162517uwb,
    3873181wb,
    -59380wb
};

_Decimal64 b[] = {
    10000000000000000000000000000000000000000000000000000000uwb,
    20000000000000000000000000000000000000000000000000000000wb,
    -30000000000000000000000000000000000000000000000000000000wb,
    162517uwb,
    3873181wb,
    -59380wb
};

_Decimal128 c[] = {
    10000000000000000000000000000000000000000000000000000000uwb,
    20000000000000000000000000000000000000000000000000000000wb,
    -30000000000000000000000000000000000000000000000000000000wb,
    162517uwb,
    3873181wb,
    -59380wb
};
#else
int a[] = {
    1259291200,
    1260291200,
    -886192448,
    847411925,
    851122589,
    -1300174860
};

int b[] = {
    -1530494976,
    918785406,
    1233977344,
    919018237,
    -296517632,
    -1228232581,
    162517,
    834666496,
    3873181,
    834666496,
    59380,
    -1312817152
};

int c[] = {
    0,
    952195850,
    -968585837,
    812396877,
    0,
    1904391700,
    -1937171674,
    812409499,
    0,
    -1438379746,
    1389209785,
    -1335061527,
    162517,
    0,
    0,
    809500672,
    3873181,
    0,
    0,
    809500672,
    59380,
    0,
    0,
    -1337982976
};
#endif
