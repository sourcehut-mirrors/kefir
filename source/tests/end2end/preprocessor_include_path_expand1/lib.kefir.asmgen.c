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

#define DIRECTORY1 dir1
#define DIRECTORY2 dir2
#define DIRECTORY(x) DIRECTORY##x

#define FILE1 file1
#define FILE2 file2
#define FILE(x) FILE##x

#define EXT h

#define INCLUDE_PATH <DIRECTORY(1)/FILE(2).EXT>
#if __has_include(INCLUDE_PATH)
#include INCLUDE_PATH
#endif

#ifndef FILE2_INCLUDED_H_
#error "Failed to include correct file"
#else
Success
#endif
