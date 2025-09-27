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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

int main(void) {
    int i = 0;
    assert(a[i++] == -1341863151);
    assert(a[i++] == 805620497);
    assert(a[i++] == 805620497);
    assert(a[i++] == -1341863151);
    assert(a[i++] == -1300234240);
    assert(a[i++] == 847249408);
    assert(a[i++] == 847249408);
    assert(a[i++] == -1300234240);
    assert(a[i++] == 805892355);
    assert(a[i++] == 805578196);
    assert(a[i++] == 805620527);
    assert(a[i++] == 805348699);
    assert(a[i++] == -1341905452);
    assert(a[i++] == 805620527);
    assert(a[i++] == 1809993289);
    assert(a[i++] == 805306368);
    assert(a[i++] == 805306368);
    assert(a[i++] == 798073487);
    assert(a[i++] == 889192448);
    assert(a[i++] == 2013265920);

    i = 0;
    assert(b[i++] == 12763201343968299793ull);
    assert(b[i++] == 3539829307113523985ull);
    assert(b[i++] == 3539829307113523985ull);
    assert(b[i++] == 12763201343968299793ull);
    assert(b[i++] == 12808237340241690624ull);
    assert(b[i++] == 3584865303386914816ull);
    assert(b[i++] == 3584865303386914816ull);
    assert(b[i++] == 12808237340241690624ull);
    assert(b[i++] == 3539829307113795843ull);
    assert(b[i++] == 3539829307113481684ull);
    assert(b[i++] == 3539829307113524015ull);
    assert(b[i++] == 3539829307113252187ull);
    assert(b[i++] == 12763201343968257492ull);
    assert(b[i++] == 3539829307113524015ull);
    assert(b[i++] == 3494793396236717548ull);
    assert(b[i++] == 3539829307113209856ull);
    assert(b[i++] == 3539829307113209856ull);
    assert(b[i++] == 3450913041716792475ull);
    assert(b[i++] == 3629901299660619776ull);
    assert(b[i++] == 8646911284551352320ull);

    i = 0;
    assert(c[i++] == 314129);
    assert(c[i++] == 12697336199417692160ull);
    assert(c[i++] == 314129);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 314129);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 314129);
    assert(c[i++] == 12697336199417692160ull);
    assert(c[i++] == 0);
    assert(c[i++] == 12700150949184798720ull);
    assert(c[i++] == 0);
    assert(c[i++] == 3476778912330022912ull);
    assert(c[i++] == 0);
    assert(c[i++] == 3476778912330022912ull);
    assert(c[i++] == 0);
    assert(c[i++] == 12700150949184798720ull);
    assert(c[i++] == 585987);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 271828);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 314159);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 42331);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 271828);
    assert(c[i++] == 12697336199417692160ull);
    assert(c[i++] == 314159);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 85397212652);
    assert(c[i++] == 3471149412795809792ull);
    assert(c[i++] == 0);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 0);
    assert(c[i++] == 3473964162562916352ull);
    assert(c[i++] == 12755455644677436957ull);
    assert(c[i++] == 3458264215961514945ull);
    assert(c[i++] == 0);
    assert(c[i++] == 3479593662097129472ull);
    assert(c[i++] == 0);
    assert(c[i++] == 8646911284551352320ul);
    return EXIT_SUCCESS;
}
