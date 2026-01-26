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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

int main(void) {
    int i = 0;
#ifndef KEFIR_PLATFORM_DECIMAL_DPD
    assert(a[i++] == 847249408);
    assert(a[i++] == 800066001);
    assert(a[i++] == 847249408);
    assert(a[i++] == 837230439);
    assert(a[i++] == 847249408);
    assert(a[i++] == 1830907570);
    assert(a[i++] == 2080374784);
    assert(a[i++] == 2013265920);
    assert(a[i++] == 1824615131);
    assert(a[i++] == 911799156);
    assert(a[i++] == 847249408);
    assert(a[i++] == 847249408);
    assert(a[i++] == 847249408);
    assert(a[i++] == 864329818);
    assert(a[i++] == 1835544350);
    assert(a[i++] == 1827723283);

    i = 0;
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == -49648252);
    assert(b[i++] == 803942222);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 279627008);
    assert(b[i++] == 813171351);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == -1116003387);
    assert(b[i++] == 826155717);
    assert(b[i++] == 0);
    assert(b[i++] == 2080374784);
    assert(b[i++] == 0);
    assert(b[i++] == 2013265920);
    assert(b[i++] == 84814991);
    assert(b[i++] == 834666496);
    assert(b[i++] == -671626667);
    assert(b[i++] == 834680068);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 303194);
    assert(b[i++] == 838860800);
    assert(b[i++] == -926007936);
    assert(b[i++] == 830453043);
    assert(b[i++] == -1301928502);
    assert(b[i++] == 1815722336);

    i = 0;
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 1733689461);
    assert(c[i++] == -1259864708);
    assert(c[i++] == 1);
    assert(c[i++] == 807010304);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == -1608694571);
    assert(c[i++] == 22930925);
    assert(c[i++] == 314059257);
    assert(c[i++] == 805784885);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 1175828299);
    assert(c[i++] == -438829485);
    assert(c[i++] == -108812803);
    assert(c[i++] == 806593079);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 2080374784);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 2013265920);
    assert(c[i++] == 84814991);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == -671626667);
    assert(c[i++] == 13572);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 303194);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809762816);
    assert(c[i++] == -926007936);
    assert(c[i++] == 2078003);
    assert(c[i++] == 0);
    assert(c[i++] == 809107456);
    assert(c[i++] == 493485632);
    assert(c[i++] == 1971137591);
    assert(c[i++] == 51);
    assert(c[i++] == 807927808);
#else
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 770908481ll);
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 977232199ll);
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 1788389930ll);
    assert(a[i++] == 2080374784ll);
    assert(a[i++] == 2013265920ll);
    assert(a[i++] == 1785212511ll);
    assert(a[i++] == 918863418ll);
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 553648128ll);
    assert(a[i++] == 578161882ll);
    assert(a[i++] == 1790620350ll);
    assert(a[i++] == 1853418079ll);

    i = 0;
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == 1144023084ll);
    assert(b[i++] == 771541072ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == -727711744ll);
    assert(b[i++] == 974116945ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == -387764917ll);
    assert(b[i++] == 1780886146ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 2080374784ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 2013265920ll);
    assert(b[i++] == 78147743ll);
    assert(b[i++] == 574095360ll);
    assert(b[i++] == -1946669875ll);
    assert(b[i++] == 574117974ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == 0ll);
    assert(b[i++] == 568590336ll);
    assert(b[i++] == 396506ll);
    assert(b[i++] == 574619648ll);
    assert(b[i++] == -1783972536ll);
    assert(b[i++] == 1781443759ll);
    assert(b[i++] == -878100968ll);
    assert(b[i++] == 1847474839ll);

    i = 0;
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == -2100934016ll);
    assert(c[i++] == -2063318266ll);
    assert(c[i++] == 204ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == 286127ll);
    assert(c[i++] == -2061678080ll);
    assert(c[i++] == 445ll);
    assert(c[i++] == 570671104ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == -726769498ll);
    assert(c[i++] == -1473343950ll);
    assert(c[i++] == 546ll);
    assert(c[i++] == 570769408ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 2080374784ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 2013265920ll);
    assert(c[i++] == 78147743ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570949632ll);
    assert(c[i++] == -1946669875ll);
    assert(c[i++] == 22614ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570949632ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570605568ll);
    assert(c[i++] == 396506ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570982400ll);
    assert(c[i++] == -1783972536ll);
    assert(c[i++] == 2272431ll);
    assert(c[i++] == 0ll);
    assert(c[i++] == 570900480ll);
    assert(c[i++] == 215037582ll);
    assert(c[i++] == -182141102ll);
    assert(c[i++] == 42ll);
    assert(c[i++] == 570753024ll);
#endif
    return EXIT_SUCCESS;
}
