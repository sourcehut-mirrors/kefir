#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2025  Jevgenijs Protopopovs
# 
# This file is part of Kefir project.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
# # 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

require_asmgen_decimal_support () {
    if [[ "x$(echo '__KEFIRCC_DECIMAL_SUPPORT__' | $KEFIRCC -E -o- -)" != "x1" ]]; then
        cp "${SRC_FILE/.kefir.asmgen.c/.asmgen.expected}" "$DST_FILE"
        echo "Skipping end2end test $SCRIPT due to missing decimal support"
        exit 0
    fi
}

require_asmgen_decimal_bitint_conv_support () {
    if [[ "x$(echo '__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__' | $KEFIRCC -E -o- -)" != "x1" ]]; then
        cp "${SRC_FILE/.kefir.asmgen.c/.asmgen.expected}" "$DST_FILE"
        echo "Skipping end2end test $SCRIPT due to missing conversion between decimal and bit-pricese integers support"
        exit 0
    fi
}
