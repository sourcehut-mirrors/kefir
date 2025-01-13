#!/usr/bin/env bash
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2025  Jevgenijs Protopopovs
#
# This file is part of Kefir project.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

set -e

SCRIPT_DIR="$(dirname $(realpath $0))"
ROOT_DIR="$(realpath $SCRIPT_DIR/..)"
OUT_DIR="$1"

error () {
    local FMT="$1"
    shift 1
    printf "\e[;31m$FMT\n" $@ >&2
    exit -1
}

warn () {
    local FMT="$1"
    shift 1
    printf "\e[;93mWarning: $FMT\e[0m\n" $@ >&2
}

info () {
    local FMT="$1"
    shift 1
    printf "\e[;92m$FMT\e[0m\n" $@
}

generate_index_html_header () {
    cat<<EOF
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta charset="utf-8">
        <title>Kefir C compiler &mdash; External test archive</title>
        <style>
            body {
                width: fit-content;
            }
            p {
                max-width: 80ch;
            }
            tr:nth-child(odd) {
                background-color: #ddd;
            }
            footer {
                max-width: 100%;
            }
            .footer-content {
                display: flex;
                flex-direction: row;
                flex-wrap: wrap;
                justify-content: space-between;
            }
        </style>
    </head>

    <body>
        <header>
            <h1>Kefir C compiler &mdash; External test archive</h1>

            <p>
                This web page provides an archive of third-party software used by
                <a href="https://kefir.protopopov.lv">Kefir C compiler</a> external test suite.
            </p>

            <p>
                All software provided in this archive is taken from well-known open source projects, original links are provided for each file.
                Please refer to respective project licenses before making any use of these files. This archive is provided solely
                for reproducibility and stability of Kefir external test suite, with no other purpose or meaning. Archive author does not make any claims
                or guarantees with respect to software provided in this archive.
            </p>
        </header>

        <main>
            <table>
                <thead>
                    <tr>
                        <th>Test name</th>
                        <th>Link</th>
                        <th>Original</th>
                    </tr>
                </thead>
                <tbody>
EOF
}

generate_index_entry () {
    local TEST_NAME="$1"
    local FILENAME="$2"
    local ORIGINAL="$3"
    cat<<EOF
                    <tr>
                        <td>$TEST_NAME</td>
                        <td>
                            <a href="./$TEST_NAME/$FILENAME">$FILENAME</a>
                        </td>
                        <td>
                            <a href="$ORIGINAL">$ORIGINAL</a>
                        </td>
                    </tr>
EOF
}

generate_index_html_footer () {
    cat<<EOF
                </tbody>
            </table>
        </main>
        <footer>
            <hr>
            <div class="footer-content">
                <div>
                    <a href="https://www.protopopov.lv">Archive author</a> can be contacted <a href="mailto:jevgenij@protopopov.lv">via email</a>. 
                </div>

                <div>
                    <a href="/about.html">About this site</a>. 
                </div>
            </div>
        </footer>
    </body>
</html>
EOF
}

archive_test () {
    local TEST_DIR="$1"
    local TEST_NAME="$(basename $1)"
    if [[ ! -f "$TEST_DIR/Makefile.mk" ]]; then
        warn "Unable to find Makefile.mk for external test %s" "$TEST_NAME"
        return
    fi

    make -pn -f "$TEST_DIR/Makefile.mk" | grep -E "KEFIR_EXTERNAL_TEST_[a-zA-Z0-9_]+_URL\s+:=" | while read -d $'\n' SOURCE_URL_DEFINITION; do
        local SOURCE_URL=`echo "$SOURCE_URL_DEFINITION" | awk -F':=' '{ print $2 }' | sed -re 's/\s*(.+)\s*/\1/g'`
        local FILENAME=`basename "$SOURCE_URL"`
        info "[%s] found URL: %s" "$TEST_NAME" "$SOURCE_URL"

        mkdir -p "$OUT_DIR/$TEST_NAME"
        pushd "$OUT_DIR/$TEST_NAME" >/dev/null
        wget -O "$FILENAME" "$SOURCE_URL"
        popd >/dev/null
        generate_index_entry "$TEST_NAME" "$FILENAME" "$SOURCE_URL" >> "$OUT_DIR/index.html"
    done
}

if [[ "x$OUT_DIR" == "x" ]]; then
    error "Usage: $0 out_directory"
else
    OUT_DIR="$(realpath $OUT_DIR)"
fi

if [[ ! -d "$OUT_DIR" ]]; then
    mkdir "$OUT_DIR"
fi

generate_index_html_header > "$OUT_DIR/index.html"
find "$ROOT_DIR/source/tests/external" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z | while read -d $'\0' TEST_DIR; do
    archive_test "$TEST_DIR"
done
generate_index_html_footer >> "$OUT_DIR/index.html"