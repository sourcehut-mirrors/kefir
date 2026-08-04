#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2026  Jevgenijs Protopopovs
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

import sys
import argparse
from html.parser import HTMLParser
from urllib.request import urlopen
import urllib.request as urllib_request
from urllib.parse import urljoin

class TableParser(HTMLParser):
    def __init__(self, base_url):
        super().__init__()
        self.base_url = base_url
        self.in_td = False
        self.td_count = 0
        self.href = ''
        self.cells = []
        self.rows = []

    def handle_starttag(self, tag, attrs):
        if tag == 'td':
            self.in_td = True
        elif tag == 'a' and self.in_td:
            for k, v in attrs:
                if k == 'href':
                    self.href = urljoin(self.base_url, v)

    def handle_endtag(self, tag):
        if tag == 'td':
            self.in_td = False
            self.cells.append(self.href)
            self.href = ''
            self.td_count += 1
            if self.td_count % 3 == 0:
                self.rows.append(self.cells)
                self.cells = list()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog=sys.argv[0])
    parser.add_argument('--username', type=str, required=False)
    parser.add_argument('--password', type=str, required=False)
    parser.add_argument('url', type=str)

    args = parser.parse_args()

    password_mgr = urllib_request.HTTPPasswordMgrWithDefaultRealm()
    if args.username and args.password:
        password_mgr.add_password(None, args.url, args.username, args.password)

    auth_handler = urllib_request.HTTPBasicAuthHandler(password_mgr)
    opener = urllib_request.build_opener(auth_handler)
        
    with opener.open(args.url) as f:
        html = f.read().decode('utf-8')

    parser = TableParser(args.url)
    parser.feed(html)

    for row in parser.rows:
        local = row[1]
        orig = row[2]
        print(f'{orig}|{local}')
