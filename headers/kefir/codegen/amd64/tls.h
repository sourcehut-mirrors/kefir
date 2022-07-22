/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_AMD64_TLS_H_
#define KEFIR_CODEGEN_AMD64_TLS_H_

#define KEFIR_AMD64_THREAD_LOCAL "%s@tpoff"
#define KEFIR_AMD64_THREAD_LOCAL_GOT "%s@gottpoff"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GOT "__emutls_v.%s@GOTPCREL"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address@PLT"

#endif
