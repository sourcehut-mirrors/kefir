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

#line __LINE__ "decimal_const_eval_ops1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 a[] = {
    -(3.14129df),
    -(-(3.14129df)),
    +(3.14129df),
    -(+(3.14129df)),
    -(0.0df),
    -(-(0.0df)),
    +(0.0df),
    -(+(0.0df)),
    3.14159df + 2.71828df,
    0.0df + 2.71828df,
    3.14159df + 0.0df,
    3.14159df - 2.71828df,
    0.0df - 2.71828df,
    3.14159df - 0.0df,
    3.14159df * 2.71828df,
    0.0df * 2.71828df,
    3.14159df * 0.0df,
    3.14159df / 2.71828df,
    0.0df / 2.71828df,
    3.14159df / 0.0df
};

_Decimal64 b[] = {
    -(3.14129dd),
    -(-(3.14129dd)),
    +(3.14129dd),
    -(+(3.14129dd)),
    -(0.0dd),
    -(-(0.0dd)),
    +(0.0dd),
    -(+(0.0dd)),
    3.14159dd + 2.71828dd,
    0.0dd + 2.71828dd,
    3.14159dd + 0.0dd,
    3.14159dd - 2.71828dd,
    0.0dd - 2.71828dd,
    3.14159dd - 0.0dd,
    3.14159dd * 2.71828dd,
    0.0dd * 2.71828dd,
    3.14159dd * 0.0dd,
    3.14159dd / 2.71828dd,
    0.0dd / 2.71828dd,
    3.14159dd / 0.0dd
};

_Decimal128 c[] = {
    -(3.14129dl),
    -(-(3.14129dl)),
    +(3.14129dl),
    -(+(3.14129dl)),
    -(0.0dl),
    -(-(0.0dl)),
    +(0.0dl),
    -(+(0.0dl)),
    3.14159dl + 2.71828dl,
    0.0dl + 2.71828dl,
    3.14159dl + 0.0dl,
    3.14159dl - 2.71828dl,
    0.0dl - 2.71828dl,
    3.14159dl - 0.0dl,
    3.14159dl * 2.71828dl,
    0.0dl * 2.71828dl,
    3.14159dl * 0.0dl,
    3.14159dl / 2.71828dl,
    0.0dl / 2.71828dl,
    3.14159dl / 0.0dl
};
#else
int a[] = {
    -1341863151,
    805620497,
    805620497,
    -1341863151,
    -1300234240,
    847249408,
    847249408,
    -1300234240,
    805892355,
    805578196,
    805620527,
    805348699,
    -1341905452,
    805620527,
    1809993289,
    805306368,
    805306368,
    798073487,
    889192448,
    2013265920
};

unsigned long b[] = {
    12763201343968299793ull,
    3539829307113523985ull,
    3539829307113523985ull,
    12763201343968299793ull,
    12808237340241690624ull,
    3584865303386914816ull,
    3584865303386914816ull,
    12808237340241690624ull,
    3539829307113795843ull,
    3539829307113481684ull,
    3539829307113524015ull,
    3539829307113252187ull,
    12763201343968257492ull,
    3539829307113524015ull,
    3494793396236717548ull,
    3539829307113209856ull,
    3539829307113209856ull,
    3450913041716792475ull,
    3629901299660619776ull,
    8646911284551352320ull

};

unsigned long c[] = {
    314129, 12697336199417692160ull,
    314129, 3473964162562916352ull,
    314129, 3473964162562916352ull,
    314129, 12697336199417692160ull,
    0, 12700150949184798720ull,
    0, 3476778912330022912ull,
    0, 3476778912330022912ull,
    0, 12700150949184798720ull,
    585987, 3473964162562916352ull,
    271828, 3473964162562916352ull,
    314159, 3473964162562916352ull,
    42331, 3473964162562916352ull,
    271828, 12697336199417692160ull,
    314159, 3473964162562916352ull,
    85397212652, 3471149412795809792ull,
    0, 3473964162562916352ull,
    0, 3473964162562916352ull,
    12755455644677436957ull, 3458264215961514945ull,
    0, 3479593662097129472ull,
    0, 8646911284551352320ull
};
#endif
