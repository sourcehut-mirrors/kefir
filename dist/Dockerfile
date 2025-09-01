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

# Base image includes minimum number of dependencies necessary to build kefir and run its own test suite
FROM ubuntu:noble AS base

RUN apt update
# Minimum build dependencies -- either gcc or clang are sufficient
RUN apt install -y build-essential gcc-14 clang-16 groff m4
# For musl builds
RUN apt install -y musl-dev musl-tools

########################################################################
# Dev image includes base image + all remaining packages necessary for local development + emscripten for webapp builds
FROM base AS dev

RUN apt install -y valgrind unminimize wget yasm cmake git

# Prepare the system
RUN yes | unminimize

# GNU Make 4.4
RUN wget -O /tmp/make-4.4.1.tar.gz https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz
RUN cd /tmp && tar xvf make-4.4.1.tar.gz
RUN cd /tmp/make-4.4.1 && ./configure --prefix=/opt/make-4.4.1
RUN cd /tmp/make-4.4.1 && make -j$(nproc)
RUN cd /tmp/make-4.4.1 && make install

# Emscripten
RUN cd /tmp && wget "https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.74.tar.gz"
RUN cd /opt && tar xvf /tmp/3.1.74.tar.gz
RUN cd /opt/emsdk-3.1.74 && ./emsdk install 3.1.74
RUN cd /opt/emsdk-3.1.74 && ./emsdk activate 3.1.74
RUN echo "export EM_CACHE=\$HOME/.emcache" >> /opt/emsdk-3.1.74/emsdk_env.sh

# CSmith
RUN cd /tmp && git clone --depth 1 "https://github.com/csmith-project/csmith"
RUN cd /tmp/csmith && mkdir build
RUN cd /tmp/csmith/build && cmake -DCMAKE_INSTALL_PREFIX=/opt/csmith ..
RUN cd /tmp/csmith/build && make -j$(nproc)
RUN cd /tmp/csmith/build && make install

# Cleanup
RUN rm -rf /tmp/*

# Development environment
ADD ./dist/kefir_dev_env.sh /opt/kefir_dev_env.sh
RUN echo "source /opt/kefir_dev_env.sh" >> /etc/profile
RUN echo "source /opt/kefir_dev_env.sh" >> /etc/bash.bashrc

########################################################################
# Full image includes base image + all dependencies necessary to run external test suite
# Not that these dependencies are not of Kefir itself, but of projects within external test suite
FROM dev as full

RUN apt install -y nasm yacc libncurses-dev zlib1g-dev gettext file openssl \
    libssl-dev unzip libpcre3 libpcre3-dev tcl8.6 tcl8.6-dev gcc-multilib \
    man-db autoconf autopoint bison gperf texinfo libgmp-dev inotify-tools strace \
    libinotifytools0-dev libtool pkg-config libsqlite3-dev flex libreadline-dev \
    libfont-ttf-perl libsort-versions-perl fonts-liberation libtool-bin libunistring-dev \
    libgc-dev libglib2.0-dev-bin dejagnu locales rake python3-dev libyaml-dev libgtest-dev \
    libevent-dev libtommath-dev libpsl-dev

# Locales
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN echo "fr_FR.UTF-8 UTF-8" >> /etc/locale.gen  
RUN echo "ja_JP.UTF-8 UTF-8" >> /etc/locale.gen  
RUN echo "ru_RU.UTF-8 UTF-8" >> /etc/locale.gen  
RUN locale-gen
