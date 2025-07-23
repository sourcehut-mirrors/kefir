# Kefir C compiler
Kefir is an independent compiler for C programming language (C17 standard).
Kefir does not rely on any existing frameworks, and provides a complete compiler
implementation written in C. The main priority is self-sufficiency of the
project, compatibility with platform ABI and compliance with C17 language
standard. Any omissions or incompatibilities between the language standard and
Kefir behavior which are not explicitly documented (see `Implementation & Usage
quirks` section below) shall be considered bugs.

Kefir supports modern x86-64 Linux, FreeBSD, OpenBSD and NetBSD environments
(see `Supported environments` section below). Compiler is also able to produce
JSON streams containing program representation at various stages of compilation
(tokens, AST, IR), as well as printing source code in preprocessed form. The
compiler targets GNU As (Intel syntax with/without prefixes and ATT syntax are
supported) and also has limited support for Yasm assembler. Kefir is able to
produce debug information in DWARF5 format for GNU As. Position-independent code
generation is supported. Kefir features `cc`-compatible command line interface.

[Kefir website](https://kefir.protopopov.lv) also provides some additional information.

**Note to the users of Kefir:** if you encounter any behavior that does not
comply with C17 language standard or significantly diverges from other
compilers, please do no hestitate to reach me out via
[email](mailto:jevgenij@protopopov.lv) directly or through the [mailing
list](https://lists.sr.ht/~jprotopopov/public-inbox). Code snippets for easier
reproduction are especially welcome.

### Project name
Kefir compiler is named after [fermented milk
drink](https://en.wikipedia.org/wiki/Kefir), **no other connotations are meant
or intended**.

## Supported environments
Kefir targets x86-64 ISA and System-V ABI. Supported systems include modern
Linux, FreeBSD, OpenBSD and NetBSD operating systems (full test suite is
executed regularly in these environments). A platform is considered supported if
full automated test suite (see `Test suite` section below) sucessfully executes
there -- no other guarantees and claims are made. On Linux, `glibc` and `musl`
standard libraries are supported; `musl` might be preferable because its header
files use significantly less non-standard extensions, however as of now Kefir
supports enough GCC extensions to reasonably use include files from `glibc`; on
BSDs system `libc` can be used (additional macro definitions, such as
`__GNUC__`, `__GNUC_MINOR__`, could be necessary depending on used system libc
features). Kefir supports selection of target platform via `--target` command
line option.

For each respective target, compiler expects a set of environment variables
(e.g. `KEFIR_GNU_INCLUDE`, `KEFIR_GNU_LIB`, `KEFIR_GNU_DYNAMIC_LINKER`) to be
present in order to correctly configure system include and library paths -- the
default values for these variables are detected upon Kefir build, however in the
event of any changes to system toolchain (e.g. after upgrades) configuration
needs to be re-generated.

In addition, see `Implementation & Usage quirks` section below for some other
specifics of Kefir.

## Motivation & goals
The main motivation of the project is deeper understanding of C programming
language, as well as practical experience in the broader scope of compiler
implementation aspects. Based on this, following goals were set for the project:
* Self-sufficiency - the project shall use minimal number of external
dependencies. Runtime dependencies should include only C standard library and
operating system APIs.
* Compliance with C17 standard - resulting product should be reasonably
compliant with language standard. All intentional deviations and exceptions
shall be documented and justified. 
* Compatibility with platform ABI - produced code should adhere ABI of target
platform. It should be possible to transparently link it with code produced by
commonly used compilers of the target platform.
* Manageable scope of the project - full-fledged implementation of C17 compiler
is a demanding task. Project scope shall be managed so that implementation as a
single-person pet-project is feasible. For instance, standard library
implementation is currently out-of-scope. Instead, compiler supports some of
widespread C extensions in order to re-use existing `libc` implementations.
* Portability - compiler code itself should be easily portable across different
environments. Currently, the development is concentrated on a single target
platform, however it might be extended in the future.

Following things are **NON-goals**:
* Performance - trying to outcompete well-established compiler backends, such as
GCC backend or LLVM, is not reasonable, thus performance was never considered a
goal, even though some improvements can be occasionally made. In fact,
performance is deliberately sacrificed to facilitate implementation of other
goals. 
* Compatibility with other compiler extensions - C compilers are known to
include different extensions that are not described by the language standard.
Considerable number of those are supported by Kefir, however it is not project
goal per se, thus there are no guarantees of extension compatibility.

Note on the language standard support: initially the compiler development was
focused on C11 language standard support. The migration to C17 happened when the
original plan was mostly finished. During the migration applicable DRs from
those included in C17 were inspected and code was updated accordingly. The
compiler itself is still written in compliance with C11 language standard.

## Current status
See `CHANGELOG`.

The initial implementation has been finished: at the momement the compiler
features all necessary components, and with some known minor idiosyncrasies
supports C17 language standard. Kefir is able to reasonably re-use standard
library provided by target systems. Significant effort has been spent to compose
a test suite of third-party open source software to validate correctness of
Kefir. Further effort is concentrated on improving and extending the compiler,
including:
* Implementing optimizing code generator -- initial basic implementation has
  been finished. Pending subtasks are:
  * Implementing actual optimization passes -- several basic optimizations have been implemented.
  * Improving debug information generator.
* Adding support for upcoming C23 standard -- the compiler currently implements
  several C23 features as extensions, however their completeness and compliance
  with the actual language standard has not been determined.
* Improving compatibility with mainstream compilers by implementing additional
  extensions and built-ins.
* Bugfixes, improvements in error reporting.
* Extending the number of supported platforms.
* Dropping stack-based intermediate representation step.
* Reimplementing parser, lexer and register allocator for better performance.
* Refactoring and cleaning up analysis and translation stage implementation.

### Implementation quirks
Some implementation details that user needs to take into account:
* Kefir might provide own versions of some header files as well -- if the
  environment is configured correctly, they are added to include path
  automatically.
* Atomic implementation partially relies on software atomic library (`libatomic`
  for GCC, `libcompiler_rt` for Clang). Kefir implements atomic primitives for
  1, 2, 4 and 8 byte integral values natively; atomic operations for any other
  types rely on the software library. Thus any program that utilizes atomic
  operations might need to link a `libatomic`-compatible library. It happens by
  default for Glibc and *BSD targets. Furthermore, if `<stdatomic.h>` header
  from Clang includes is used (the default on FreeBSD and OpenBSD),
  `-D__GNUC__=4 -D__GNUC_MINOR__=20` command line arguments shall be added to
  Kefir invocation.
* Atomic operations on long double variables (both scalar and complex) might
  result in undefined behavior due to uninitialized padding contained at the
  last 48 bits of long double storage unit. Kefir implements padding zeroing
  upon long double storage to mitigate this issue, however linking object files
  produced by Kefir and other compilers might provoke the undefined behavior and
  needs to be done with care.
* Unicode and wide strings are supported under the assumption that source and
  target character sets are the same. No re-encoding is performed.
* No `STDC` pragmas are implemented in preprocessor. Kefir does not perform
  respective optimizations and implements conservative behavior, thus these
  pragmas would be no-op.
* Integration with `glibc` might exhibit problems with non-standard features
  (e.g. attributes). For instance, `glibc` might suppress `__attribute__` by a
  macro or omit `packed` attributes, thus breaking the ABI, so care needs to be
  taken.

### Standard library
Kefir can be used along with [musl libc](https://musl.libc.org) standard
library. Kefir also supports `glibc`, as well as `libc` implementations provided
with FreeBSD, OpenBSD and NetBSD, however header files from these libraries tend
to include non-standard compiler features, and thus support might vary for
different library versions. In practice, Kefir implements enough compiler
extensions to make use of all target system standard libraries. However,
additional macro definitions (such as `__GNUC__` and `__GNUC_MINOR__` on BSD
systems) might be needed for successful compilation.

### Language extensions
Several C language extensions are implemented for better compatibility with GCC.
All of them are enabled by default in the driver, but disabled if the compiler
is invoked directly (consult the manual for details). No specific compability
guarantees are provided. Among the implemented extensions (non-exaustive list):
* Implicit function declarations -- if no function declaration is present at
  call-site, `int function_name()` is automatically defined. The feature was
  part of previous C standards, however it's absent from C11 onwards.
* `int` as implicit function return type -- function definition may omit return
  type, `int` will be used instead.
* Designated initializers in `fieldname:` form -- old, deprecated form which is
  still supported by GCC.
* Labels-as-values -- labels can be addressed with `&&` operator, gotos support
  arbitratry addresses in `goto *`  form.
* Automatic type inference -- `__typeof__`, `__typeof_unqual__`, `__auto_type`
  type specifiers.
* Vararg builtins.
* Some of `__atomic*` and `__sync*` builtins.
* Some of overflow arithmetics builtins.
* Full list of supported builtins and attributes can be obtained via
```bash
echo '__KEFIRCC_SUPPORTED_BUILTINS__' | kefir -E -
```

Kefir also defines a few non-standard macros by default, such as macros
indicating data model (`__LP64__`), endianess (`__BYTE_ORDER__` and
`__ORDER_LITTLE_ENDIAN__`), as well as `__KEFIRCC__` which can be used to
identify the compiler.

Kefir has support of `asm` directive, both in file and function scope.
Implementation supports output and input parameters, parameter constraints
(immediate, register, memory), clobbers and jump labels, however there is no
compatibility with respective GCC/Clang functionality (implemented bits behave
similarly, though, thus basic use cases shall be compatible). Additionally,
`asm`-labels are supported for non-static non-local variables.

Kefir supports `__attribute__(...)` syntax, however only a few attributes are
actually implemented. Presence of attribute in source code can be turned into a syntax error
by CLI option. For the list of supported GNU attributes see:
```bash
echo '__KEFIRCC_SUPPORTED_GNU_ATTRIBUTES__' | kefir -E -
```


## Build & Usage
**Disclaimer: Use at your own risk. This is experimental project which is not
meant for production purposes. No guarantees are being made for correctness,
completeness, stability and fitness for any particular purpose.**

Kefir depends on a C11 compiler (tested with `gcc` and `clang`), bash, GNU Make
as well as basic UNIX utilities for build. Runtime dependencies include `libc`,
an assembler (GNU As or Yasm) and a linker (GNU ld). Test dependencies
optionally include `valgrind`, `python` and `csmith` as well. After installing
all dependencies, kefir can be built with a single command: `make -j$(nproc)`.
By default, kefir builds a shared library and links executables to it. Static
linkage can be enforced by specifying `USE_SHARED=no` in make command line
arguments. Sample `PKGBUILD` is provided in `dist/kefir` directory.

It is also advised to run basic test suite:
```bash
make test all                  # Linux glibc
make test all USE_SHARED=no CC=musl-gcc KEFIR_TEST_USE_MUSL=yes   # Linux musl
gmake test all CC=clang        # FreeBSD
gmake test all CC=clang AS=gas # OpenBSD
gmake test all CC=gcc AS=gas   # NetBSD
```

Optionally, Kefir can be installed via: `make install DESTDIR=... prefix=...`.
Short reference on compiler options can be obtained by running `kefir --help`,
as well as in the manual which is supplied in the compiler distribution.

At the moment, Kefir is automatically tested in Ubuntu 24.04, FreeBSD 14.x and
OpenBSD 7.5 and NetBSD 10.x environments. Arch Linux is used as a primary
development environment. Kefir also provides a Dockerfile (see `dist`) which
defines necessary dependencies for running the whole test suite:
```bash
podman build -t kefir-dev-env -f dist/Dockerfile .
```

## Custom extensions
Kefir has limited support for custom extensions via `-Wextension-lib`
command-line option. To use that, make sure that kefir has been built with
`USE_EXTENSION_SUPPORT=yes` Makefile flag. Check out `kefir_compiler_extensions`
structure (`kefir/compiler/compiler.h` header) to see available hooks, and
`source/end2end/extensions1/extension.c` for example extension. API stability is
not guaranteed.

## Portable Kefir
Kefir provides scripts to build portable, standalone Kefir distribution package
that incorporates statically-linked Kefir C compiler, musl libc, and tools from
GNU Binutils. The package targets modern x86_64-based Linux systems and provides
a minimalistic C17 development toolchain independent of host system tooling.

Portable package can be obtained via:
```bash
make portable
# Build artifact is located in bin/portable/kefir-portable-*.tar.gz
```

In addition, portable package can be fully bootstraped in 3-stage process:
```bash
make portable_bootstrap
```

## Web playground
Kefir supports compilation with [Emscripten](https://emscripten.org/) into a
WebAssembly library, which can be invoked from client-side JavaScript in
Web-applications. Kefir functionality in that mode in limited due to absence of
normal POSIX environment, linker and assembler utilities: only text output
(assembly code, preprocessed code, tokens, ASTs, IR) can be produced from a
single input file. Furthermore, all include files need to be explicitly supplied
from JavaScript side in order to be available during compilation. Note that this
does not imply support for WebAssembly as a compilation target: it only serves
as a host environment. To build `kefir.js` and `kefir.wasm` in `bin/web`
directory, use:
```bash
make web -j$(nproc) # Requires Emscripten installed
```

A simple playground Web application is also available. It bundles Kefir web
build with Musl include files and provides a simple
[Godbolt](https://godbolt.org/)-like interface. Build as follows:
```bash
make webapp -j$(nproc)
```
The Web application is static and requires no server-side logic. An example of
simple server command-line:
```bash
python -m  http.server 8000 -d bin/webapp
```

A hosted version of the Web application is available at [Kefir
playground](https://kefir.protopopov.lv/playground/) (please note that the
Web page uses JavaScript and WebAssembly).

## Bootstrap
Kefir is capable of bootstraping itself (that is, compiling it's own source
code). This can be verified as follows:
```bash
make bootstrap_test BOOTSTRAP_EXTRA_CFLAGS="-O1 -g" -j$(nproc)
```

For practical purposes, `kefir` can be bootstrapped using the normal build
process (note that depending on the target platform, additional defines might be
necessary. See `source/tests/bootstrap/bootstrap.mk`):
```bash
make all CC=$PATH_TO_KEFIR -j$(nproc)
```

## Test suite
Kefir relies on following tests, most of which are executed as part of CI:
* Own (base) test suite that includes:
    - Unit tests
    - Integration tests -- each test is a self-contained program that executes
      some part of compilation process, produces a text output which is then
      compared to the expected.
    - System tests -- each test is a self-contained program that performs
      compilation process, starting from some stage (e.g. compiling a program
      defined as AST structure or IR bytecode) and produces an assembly output,
      which is then combined with the remaining part of test case containing
      asserts (compiled with system compiler) and executed.
    - End-to-end tests -- each test consists of multiple `*.c` files which are
      compiled either using system compiler or kefir depending on file
    extension. Everything is then linked together and executed. The test suite
    is executed on Linux with gcc and clang compilers, on FreeBSD with clang and
    on OpenBSD with clang. In Linux and FreeBSD environments `valgrind` is used
    to control test suite correctness at runtime.
* Bootstrapping test -- kefir is used to compile itself using 2-stage bootstrap
  technique as described above. On Linux, both GNU As and Yasm targets are
  tested.
* GCC Torture Suite -- `compile` & `execute` parts of GCC torture test suite are
  executed with kefir compiler, with some permissive options enabled. At the
  moment, out of 3445 tests, 477 fail and 29 are skipped due to being irrelevant
  (e.g. SIMD or profiling test cases; there is no exhaustive skip list yet). All
  failures happen on compilation stage, no abortions occur at runtime. The work
  with torture test suite will be continued in order to reduce the number of
  failures. The torture tests are included into CI pipeline with some basic test
  result checks.
* Miscallenous tests:
    - [Test suite](https://git.sr.ht/~jprotopopov/c-testsuite) which is a fork
      of [c-testsuite](https://github.com/c-testsuite/c-testsuite) is executed.
      Currently, the test suite reports 3 failures that happen due to C language
      extensions used in the tests. Failing test cases are skipped.
    - Lua from the external test suite (see below).

Own test suite is deterministic (that is, tests do not fail spuriously), however
there might arise problems when executed in unusual environments (e.g. with
non-Unicode locale). For instance, some tests contain unicode characters and
require the environment to have appropriate locale set. Also, issues with local
standard library version might cause test failures.
      
Furthermore, Kefir also provides an [external](source/tests/external) test suite
comprised of open source software that is known to work with Kefir. This suite
are not included in the CI, however, it is regularly executed manually. Kefir is
used to build software from the external test suite, and where possible, run
(most of) tests of the respective software projects. Software projects
constituting the external test suite are compiled with little or no changes.
Only acceptable changes are short patches that do not alter the project logic
(e.g. patching `#ifdef`s, replacing a non-standard keyword or altering the build
system is considered acceptable). Note that running the external test suite
might require additional project-specific dependencies (see `dist/Dockerfile`).

Currently, the external test suite includes: GNU Binutils (except gprofng), GNU
Coreutils, GCC 4.0.4 bootstrap, programming language interpreters (Lua, Perl,
Python, Tcl, Guile, duktape, GNU Awk, pForth), text editors (Vim, nano),
database engines (SQLite3, Postrges), assemblers (yasm, nasm), shells (Bash,
oksh), compression libraries (zlib, zstd), cryptography (OpenSSL, OpenSSH),
networking tools (nginx, curl, wget), build systems (GNU Make, muon), git (git
itself and cgit), other small language compilers (tcc, slimcc, cproc), other
libraries (libCello, libuv, libpng), as well as some lesser-known projects. The
external test suite is used to verify Kefir compatbility with real world
software. Source code for third-party projects has been archived and available
at [Kefir website](https://kefir.protopopov.lv/archive/third-party/).

Furthermore, at development stage,
[CSmith](https://github.com/csmith-project/csmith) is used for randomized,
differential testing against GCC. Problematic csmith seeds are added to the test
suite (executed optionally by `make test` if `CSMITH` environment variable is
defined; requires Python 3.12).

At the moment, extension of the test suite is a major goal. It helps
significantly in eliminating bugs, bringing kefir closer to C language standard
support and improving compiler UX in general.

## Design notes
In order to simplify translation and facilitate portability, intermediate
representation (IR) layer was introduced. It defines architecture-agnostic
64-bit stack machine bytecode, providing generic calling convention and
abstracting out the type layout information. Compiler is structured into
separate modules with respect to IR: code generation, AST analysis and
translation. The IR code is then converted into optimizer SSA-like
representation. IR layer provides several interfaces for AST analyzer to
retrieve necessary target type layout information (for instance, for constant
expression analysis). AST analysis and translation are separate stages to
improve code structure and reusability. Parser uses recursive descent approach
with unlitmited back-tracking. Lexer was implemented before preprocessor and can
be used independently of it (preprocessing stage can be completely omitted),
thus both lexer and preprocessor modules share the same lexing facilities.
Driver links kefir as a library and forks in order to isolate each file
processing.

## Source code hosting
The primary code repository is hosted at
[Sourcehut](https://sr.ht/~jprotopopov/kefir), with secondary mirrors at
[Codeberg](https://codeberg.org/jprotopopov/kefir) and [author's personal
website](https://git.protopopov.lv/kefir).

## Author and license
Author: [Jevgenijs Protopopovs](https://www.protopopov.lv)

The code base also includes patches from:
* Brian Robert Callahan - [initial OpenBSD
  port](https://briancallahan.net/blog/20220629.html)
* remph - [Treat zero-length environment variables as unset](https://lists.sr.ht/~jprotopopov/public-inbox/patches/57368)

License:
* Main body of the compiler - GNU GPLv3
* Compiler-specific includes - BSD 3-clause
  
## Useful links
* [C11 standard final working
  draft](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
* [C17 standard final working
  draft](https://files.lhmouse.com/standards/ISO%20C%20N2176.pdf)
* [Clarification Request Summary for
  C11](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2244.htm) - DRs
  included in C17 language standard as compared to C11.
* [System-V AMD64 ABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
* [Compiler explorer](https://godbolt.org/)
* [C reference](https://en.cppreference.com/w/c)
* [AMD64 instruction set
  reference](https://www.amd.com/system/files/TechDocs/24594.pdf)
* [POSIX c99 specification](https://pubs.opengroup.org/onlinepubs/9699919799/)
* [DWARF Version 5](https://dwarfstd.org/dwarf5std.html)
