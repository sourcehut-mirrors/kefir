FROM ubuntu:noble

RUN apt update
# Build dependencies
RUN apt install -y build-essential gcc-14 clang-16 groff m4
# Test dependencies
RUN apt install -y valgrind wget cmake git yasm yacc libncurses-dev zlib1g-dev gettext file openssl libssl-dev unzip libpcre3 libpcre3-dev tcl8.6 tcl8.6-dev gcc-multilib man-db unminimize autoconf autopoint bison gperf texinfo libgmp-dev inotify-tools strace libinotifytools0-dev libtool pkg-config libsqlite3-dev flex libreadline-dev libfont-ttf-perl libsort-versions-perl fonts-liberation libtool-bin libunistring-dev libgc-dev libglib2.0-dev-bin dejagnu

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

# Locales
RUN apt install -y locales
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN echo "fr_FR.UTF-8 UTF-8" >> /etc/locale.gen  
RUN echo "ja_JP.UTF-8 UTF-8" >> /etc/locale.gen  
RUN echo "ru_RU.UTF-8 UTF-8" >> /etc/locale.gen  
RUN locale-gen

# Cleanup
RUN rm -rf /tmp/*

ENV PATH=/opt/make-4.4.1/bin:/opt/csmith/bin:$PATH
ENV CSMITH=/opt/csmith/bin/csmith
ENV LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
ENV LC_ALL=C.UTF-8