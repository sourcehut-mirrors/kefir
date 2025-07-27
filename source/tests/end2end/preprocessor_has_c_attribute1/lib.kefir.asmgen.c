/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation
version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not
see <http://www.gnu.org/licenses/>.
*/

#define PRINT_ATTR(_attr) _attr = __has_c_attribute(_attr)

PRINT_ATTR(gnu::aligned);
PRINT_ATTR(gnu::__aligned__);
PRINT_ATTR(gnu::gnu_inline);
PRINT_ATTR(gnu::__gnu_inline__);
PRINT_ATTR(gnu::always_inline);
PRINT_ATTR(gnu::__always_inline__);
PRINT_ATTR(gnu::noinline);
PRINT_ATTR(gnu::__noinline__);
PRINT_ATTR(gnu::noipa);
PRINT_ATTR(gnu::__noipa__);
PRINT_ATTR(gnu::returns_twice);
PRINT_ATTR(gnu::__returns_twice__);
PRINT_ATTR(gnu::weak);
PRINT_ATTR(gnu::__weak__);
PRINT_ATTR(gnu::alias);
PRINT_ATTR(gnu::__alias__);
PRINT_ATTR(gnu::visibility);
PRINT_ATTR(gnu::__visibility__);
PRINT_ATTR(gnu::constructor);
PRINT_ATTR(gnu::__constructor__);
PRINT_ATTR(gnu::destructor);
PRINT_ATTR(gnu::__destructor__);
PRINT_ATTR(gnu::packed);
PRINT_ATTR(gnu::__packed__);

PRINT_ATTR(__gnu__::aligned);
PRINT_ATTR(__gnu__::__aligned__);
PRINT_ATTR(__gnu__::gnu_inline);
PRINT_ATTR(__gnu__::__gnu_inline__);
PRINT_ATTR(__gnu__::always_inline);
PRINT_ATTR(__gnu__::__always_inline__);
PRINT_ATTR(__gnu__::noinline);
PRINT_ATTR(__gnu__::__noinline__);
PRINT_ATTR(__gnu__::noipa);
PRINT_ATTR(__gnu__::__noipa__);
PRINT_ATTR(__gnu__::returns_twice);
PRINT_ATTR(__gnu__::__returns_twice__);
PRINT_ATTR(__gnu__::weak);
PRINT_ATTR(__gnu__::__weak__);
PRINT_ATTR(__gnu__::alias);
PRINT_ATTR(__gnu__::__alias__);
PRINT_ATTR(__gnu__::visibility);
PRINT_ATTR(__gnu__::__visibility__);
PRINT_ATTR(__gnu__::constructor);
PRINT_ATTR(__gnu__::__constructor__);
PRINT_ATTR(__gnu__::destructor);
PRINT_ATTR(__gnu__::__destructor__);
PRINT_ATTR(__gnu__::packed);
PRINT_ATTR(__gnu__::__packed__);

PRINT_ATTR(gnu::TEST123);
PRINT_ATTR(__gnu__::TEST123);
PRINT_ATTR(something::weak);