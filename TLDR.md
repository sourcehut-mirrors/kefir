# Kefir C17/C23 compiler

This is a hyper-condensed version of the [README](README.md); refer to the full
version for any details.

## Overview
* Independent compiler for C17 and C23, targeting x86_64 with System-V AMD64 ABI.
* Has been validated with an extensive set of open source software.
* Runs on Linux (glibc & musl) and BSDs (FreeBSD, OpenBSD, NetBSD).
* Translates C source to assembly; relies on external assembler, linker, libc.
* Supports certain GNU C extensions, inline assembly, DWARF-5 debug information,
  position-independent code, and bit-identical bootstrap.

## Build & Installation
* Requires C11 compiler, Bash, GNU Make, coreutils, m4, Groff, mandoc.
* Build via `make`/`gmake`; portable builds include static Kefir + musl + GNU Binutils.
* Detects host toolchain automatically; `kefir-detect-host-env --environment`
  re-generates environment configuration after installation.

On Linux:
```bash
# Installation
make -j$(nproc)
make test -j$(nproc)
sudo make install prefix=/opt/kefir

# Usage
/opt/kefir/bin/kefir --help
/opt/kefir/bin/kefir -O1 -g -o hello_world hello_world.c
```

## Optimization & Codegen
* SSA-based IR; multiple layers: stack IR -> SSA -> 3-address virtual assembly
  -> physical assembly.
* `-O1` optimizations focus on integral scalars and  include: function inlining
  (annotation-based), mem2reg, phi propagation, constant folding,
  simplification, LICM, GVN, dead code elimination, tail-call optimization.
* Generated assembly is self-contained, except for non-native atomics.

## Testing & Validation
* Internal test suite: unit, integration, system, end-to-end, CSmith-generated cases.
* Reproducible bootstrap: 3 stages, identical on both assembly listing and
  generated executable levels
* External test suite: 60+ real-world projects (GNU coreutils, Curl, Nginx,
  OpenSSL, Perl, PostgreSQL, Lua, etc.).
* Additional suites: c-testsuite, GCC Torture.

## Goals & Priorities
1. Independence within source-to-assembly translation scope.
2. Compatibility: C17/C23 standards, System-V ABI, DWARF-5, GNU extensions.
3. Well-rounded architecture; future performance improvements.
4. Limited scope: no custom assembler/linker, no non-x86_64 targets yet.

## History
* Development since Nov 2020; releases 0.1.0 -> 0.5.0 improving codegen,
  optimizer, and real-world validation.

## Distribution & Licensing
* Official source via [SourceHut](https://sr.ht/~jprotopopov/kefir/),
  [Codeberg](https://codeberg.org/jprotopopov/kefir), [author's
  mirror](http://git.protopopov.lv/kefir); Arch Linux
  [PKGBUILD](https://aur.archlinux.org/packages/kefir) available.
* Compiler: GPLv3 only; bigint/runtime headers: BSD 3-clause.
* Third-party packages for
  [Fedora](https://packages.fedoraproject.org/pkgs/kefir/kefir/),
  [Alpine](https://pkgs.alpinelinux.org/package/edge/community/x86_64/kefir) and
  [Ubuntu](https://codeberg.org/tkchia/ppa-de-rebus).

## Contacts & Resources
* Author: [Jevgenij Protopopov](https://www.protopopov.lv),
  [jevgenij@protopopov.lv](mailto:jevgenij@protopopov.lv)
* [Project website](https://kefir.protopopov.lv)
* Full set of documentation, external tests, and project links available in the
  [README](README.md).