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

#define VAL(_x) #_x = _x

VAL(__STDC_EMBED_EMPTY__)
VAL(__STDC_EMBED_FOUND__)
VAL(__STDC_EMBED_NOT_FOUND__)

#define SOMEDIR somedir
#define SOMEFILE(N) <dir2/file##N.txt>

__has_embed("file1.txt")
__has_embed("filex.txt")
__has_embed(<file2.txt>)
__has_embed(<somedir/file2.txt>)
__has_embed(SOMEFILE(2))
__has_embed(SOMEFILE(3))
__has_embed(__FILE__)
__has_embed(__DATE__)

XXX

__has_embed("file1.txt" limit(0))
__has_embed("file1.txt" limit(1))
__has_embed("file1.txt" limit(~0ull))

XXX

__has_embed("empty.txt")
__has_embed("empty.txt" limit(100))
__has_embed("empty.txt" limit(100) prefix(1))

XXX

__has_embed(__FILE__ limit(10) prefix(X) suffix(Y) if_empty(Z))
__has_embed(__FILE__ limit(10) prefix(X) suffix(Y) if_empty(Z) something_else)