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

#include "./definitions.h"

_Bool test1 (_Bool someflag, char *buf1, unsigned long buf1_size, char *buf2, unsigned long buf2_size) {
  if (someflag) {}
  do {
      unsigned long dst_len = buf2_size;
      unsigned long src_len = buf1_size;
      int rc = somefn (buf2, &dst_len, buf1, &src_len);
      if (rc != 0) {
        return 0;
      }
      buf2 += dst_len;
      buf2_size -= dst_len;
      buf1 += src_len;
      buf1_size -= src_len;
  } while (buf1_size > 0 && buf2_size > 0);
  return 0;
}
