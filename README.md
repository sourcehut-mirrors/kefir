# Kefir C compiler
This repository contains an implementation of C17 language compiler from
scratch. No existing open source compiler infrastructure is being reused. The
main priority is self-sufficiency of the project, compatibility with platform
ABI and compliance with C17 language standard. Some exceptions to the standard
were made (see `Exceptions` section below).

Kefir supports modern x86-64 Linux, FreeBSD, OpenBSD and NetBSD environments
(see `Supported environments` section below). Compiler is also able to produce
JSON streams containing program representation on various stages of compilation
(tokens, AST, IR), as well as printing source code in preprocessed form. By
default, the compiler outputs GNU As-compatible assembly (Intel syntax
with/without prefixes and ATT syntax are supported). Position-independent code
generation is supported. Kefir features `cc`-compatible command line interface.

### Project name
Kefir compiler is named after [fermented milk
drink](https://en.wikipedia.org/wiki/Kefir), no other connotations are meant or
intended.

## Supported environments
Kefir targets x86-64 ISA and System-V ABI. The main focus is on modern Linux
environments (with full range of automated tests is executed there), however
Kefir also has support for modern FreeBSD, OpenBSD and NetBSD operating systems
(base test suite and bootstrap are executed successfully in these environments).
A platform is considered supported if the base test suite and 2-stage compiler
bootstrap can be executed there -- no other guarantees and claims are made. On
Linux, `glibc` and `musl` standard libraries are supported (`musl` is
recommended because it's header files are more compilant with standard C
language without extensions), on BSDs system `libc` can be used (additional
macro definitions, such as `__GNUC__`, `__GNUC_MINOR__`, could be necessary
depending on used system libc features). Kefir supports selection of target
platform via `--target` command line option.

For each respective target, a set of environment variables (e.g.
`KEFIR_GNU_INCLUDE`, `KEFIR_GNU_LIB`, `KEFIR_GNU_DYNAMIC_LINKER`) needs to be
defined so that the compiler can find necessary headers and libraries.

## Motivation & goals
The main motivation of the project is deeper understanding of C programming
language, as well as practical experience in the broader scope of compiler
implementation aspects. Based on this, following goals were set for the project:
* Self-sufficiency - project shall use minimal number of external dependencies.
Runtime dependencies should include only C standard library and operating system
APIs.
* Compliance with C17 standard - resulting product should be reasonably
compliant with language standard. All intentional deviations and exceptions
shall be described and justified. 
* Compatibility with platform ABI - produced code should adhere ABI of target
platform. It should be possible to transparently link it with code produced by
commonly used compilers of the target platform.
* Reduced scope of the project - full-fledged implementation of C17 compiler is
demanding task. Project scope shall be reduced so that implementation as a
pet-project is feasible. For instance, standard library implementation is
currently out-of-scope. Instead, compiler supports some of widespread C
extensions in order to re-use existing `libc` implementations.
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
include different extensions that are not described by language standard. Some
of those are implemented, however it is not project goal per se, thus there are
no guarantees of extension compatibility.

Note on the language standard support: initially the compiler development was
focused on C11 language standard support. The migration to C17 happened when the
original plan was mostly finished. During the migration applicable DRs from
those included in C17 were considered and code was updated accordingly. The
compiler itself is still written in compliance with C11 language standard.

## Progress
See `CHANGELOG`.

The initial implementation has been finished: at the momement the compiler
features all necessary components, and with some known exceptions (and unknown
bugs) supports C17 language standard. Kefir is able to re-use standard library
provided by host system. Further effort is concentrated on improving and
extending the compiler, including:
* Implementing alternative, optimizing code generator -- initial basic
  implementation has been finished. Pending subtasks are:
  * Implementing actual optimization passes -- several basic optimizations have been implemented.
  * Producing debug information during code generation.
  * Position-independent code generation, building position-independent
    executables and shared libraries is supported.
* Implementing missing C17 standard features and adding support for upcoming C23
  standard.
* Improving compatibility with mainstream compiler by implementing additional
  extensions and built-ins.
* Bugfixes, improvements in error reporting.
* Extending the number of supported platforms.
* Reimplementing parser and lexer for better performance.

### Exceptions
Following exceptions were made in C17 implementation:
* Absence of `_Complex` floating-point number support. This feature is not being
used particularly frequently, but, at the same time, it complicates target code
generator.
* Absence of atomics. C17 standard defines them as optional feature, which I
decided to omit in initial implementation. Support of atomics would complicate
both IR and target code generation.
* Unicode and wide strings are supported under the assumption that source and
target character sets are the same. No re-encoding is performed.
* No `STDC` pragmas are implemented in preprocessor. Respective standard library
parts are out-of-scope, thus implementing these pragmas have no value at the
moment.

### Built-ins
At the moment, Kefir implements following builtins for compatibility with GCC:
`__builtin_va_list`, `__builtin_va_start`, `__builtin_va_end`,
`__builtin_va_copy`, `__builtin_va_arg`, `__builtin_alloca`,
`__builtin_alloca_with_align`, `__builtin_alloca_with_align_and_max`,
`__builtin_offsetof`. `__builtin_types_compatible_p`, `__builtin_choose_expr`,
`__builtin_constant_p`, `__builtin_classify_type`, `__builtin_trap`,
`__builtin_unreachable`, `__builtin_return_address`, `__builtin_frame_address`,
`__builtin_extract_return_addr`, `__builtin_frob_return_addr`,
`__builtin_ffs[l,ll]`, `__builtin_clz[l,ll]`, `__builtin_ctz[l,ll]`,
`__builtin_clrsb[l,ll]`, `__builtin_popcount[l,ll]`, `__builtin_parity[l,ll]`,
`__builtin_bswap16`, `__builtin_bswap32`, `__builtin_bswap64`,
`__builtin_huge_valf`, `__builtin_huge_val`, `__builtin_huge_vall`,
`__builtin_inff`, `__builtin_inf`, `__builtin_infl`, and provides compatiblity
stubs for some others.

Kefir supports `__attribute__(...)` syntax on parser level, however attributes
are ignored in most cases except `aligned`/`__aligned__` and `__gnu_inline__`
attributes. Presence of attribute in source code can be turned into a syntax
error by CLI option.

### Language extensions
Several C language extensions are implemented for better compatibility with GCC.
All of them are disabled by default and can be enabled via command-line options.
No specific compability guarantees are provided. Among them:
* Implicit function declarations -- if no function declaration is present at
  call-site, `int function_name()` is automatically defined. The feature was
  part of previous C standards, however it's absent from C11 onwards.
* `int` as implicit function return type -- function definition may omit return
  type, `int` will be used instead.
* Designated initializers in `fieldname:` form -- old, deprecated form which is
  still supported by GCC.
* Labels-as-values -- labels can be addressed with `&&` operator, gotos support
  arbitratry addresses in `goto *`  form.

Kefir also defines a few non-standard macros by default. Specifically, macros
indicating data model (`__LP64__`), endianess (`__BYTE_ORDER__` and
`__ORDER_LITTLE_ENDIAN__`), as well as `__KEFIRCC__` which can be used to
identify the compiler.

Kefir has support of `asm` directive, both in file and function scope.
Implementation supports output and input parameters, parameter constraints
(immediate, register, memory), clobbers and jump labels, however there is no
compatibility with respective GCC/Clang functionality (implemented bits behave
similarly, though, thus basic use cases shall be compatible). Additionally,
`asm`-labels are supported for non-static non-local variables.

### Standard library
Kefir can be used along with [musl libc](https://musl.libc.org) standard
library, with the exception for `<complex.h>` and `<tgmath.h>` headers which are
not available due to lacking support of `_Complex` types. Kefir also supports
`glibc`, as well as `libc` implementations provided with FreeBSD, OpenBSD and
NetBSD, however the support is limited due to presence of non-standard C
language extensions in header files which might cause compilation failures
(additional macros/patched stdlib headers might need to be defined for
successful compilation).

**Attention:** code produced by Kefir shall be linked with a runtime library
`libkefirrt.a`. The library is linked automatically if environment is configured
correctly. Kefir can also link built-in versions of runtime, however make sure
that correct `--target` is specified during link phase. Kefir provides own
versions of some header files as well -- if environment is configured correctly,
they are also added to include path automatically.

## Build & Usage
**Usage is strongly discouraged. This is experimental project which is not meant
for production purposes.**

Kefir depends on C11 compiler (tested with `gcc` and `clang`), GNU As assembler,
GNU Makefile as well as basic UNIX utilities for build. Development and test
dependencies include `valgrind` (for test execution) as well. After installing
all dependencies, kefir can be built with a single command: `make all OPT="-O2
-march=native -DNDEBUG" DBG="" -j$(nproc)`. By default, kefir builds a shared
library and links executables to it. Static linkage can be used by specifying
`USE_SHARED=no` in make command line arguments. Sample `PKGBUILD` is provided in
`dist` directory.

It is also advised to run basic test suite:
```bash
LC_ALL=C.UTF-8 make test all OPT=-O3 -j$(nproc)         # Linux
gmake test all PLATFORM=freebsd OPT=-O3 CC=clang        # FreeBSD
gmake test all CC=clang AS=gas PLATFORM=openbsd OPT=-O3 # OpenBSD
gmake test all CC=gcc AS=gas PLATFORM=netbsd OPT=-O3    # NetBSD
```

Optionally, Kefir can be installed via: `make install DESTDIR=...`. Short
reference on compiler options can be obtained by running `kefir --help`.

At the moment, Kefir is automatically tested in Ubuntu 22.04 (full range of
tests), FreeBSD 13.2 and OpenBSD 7.2 (base test suite) environments. Arch Linux
is used as primary development environment.

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

## Bootstrap
Kefir is capable of bootstraping itself (that is, compiling it's own source
code). At the moment, the feature is under testing, however stage 2 bootstrap is
working well. It can be performed as follows:
```bash
make bootstrap -j$(nproc)
```

Alternatively, bootstrap can be performed manually:
```bash
# Stage 0: Build & Test initial Kefir version with system compiler.
#          Produces dynamically-linked binary in bin/kefir and
#          shared library bin/libs/libkefir.so
make test all -j$(nproc)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/bin/libs
# Stage 1: Use previously built Kefir to compile itself.
#          Replace $MUSL with actual path to musl installation.
#          Produces statically-linked binary bin/bootstrap1/kefir
make -f bootstrap.mk bootstrap SOURCE=$(pwd)/source HEADERS=$(pwd)/headers BOOTSTRAP=$(pwd)/bootstrap/stage1 KEFIRCC=./bin/kefir \
LIBC_HEADERS="$INCLUDE" LIBC_LIBS="$LIB" -j$(nproc)
rm -rf bin # Remove kefir version produced by stage 0
# Stage 2: Use bootstrapped Kefir version to compile itself once again.
#          Replace $MUSL with actual path to musl installation.
#          Produces statically-linked binary bin/bootstrap2/kefir
make -f bootstrap.mk bootstrap SOURCE=$(pwd)/source HEADERS=$(pwd)/headers BOOTSTRAP=$(pwd)/bootstrap/stage2 KEFIRCC=./bootstrap/stage1/kefir \
LIBC_HEADERS="$INCLUDE" LIBC_LIBS="$LIB" -j$(nproc)
# Stage 3: Diff assembly files generated by Stage 1 and Stage 2.
#          They shall be identical
./scripts/bootstrap_compare.sh bootstrap/stage1 bootstrap/stage2
```

Furthermore, `kefir` can also be bootstrapped using normal build process:
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
  technique as described above.
* GCC Torture Suite -- `compile` & `execute` parts of GCC torture test suite are
  executed with kefir compiler, with some permissive options enabled. At the
  moment, out of 3445 tests, 584 fail and 29 are skipped due to being irrelevant
  (e.g. SIMD or profiling test cases; there is no exhaustive skip list yet). All
  failures happen on compilation stage, no abortions occur at runtime. The work
  with torture test suite will be continued in order to reduce the number of
  failures. The torture tests are included into CI pipeline with some basic test
  result checks.
* Miscallenous tests:
    - Lua test -- kefir is used to build Lua 5.4 interpreter and then Lua basic
      test suite is executed on the resulting executable 
    - [Test suite](https://github.com/protopopov1122/c-testsuite) which is a
      fork of [c-testsuite](https://github.com/c-testsuite/c-testsuite) is
      executed. Currently, the test suite reports 4 failures that happen due to
      C language extensions used in the tests. Failing test cases are skipped.
    - SQLite3 -- sqlite3 database engine is compiled by kefir, and a manual
      smoke test is performed with resulting executable. Integration with
      `sqllogictest` is planned.
    - Git, Bash, TCC, Nano, Zlib (static) -- software is compiled by kefir, and
      a manual smoke test is performed with resulting executable.

Own test suite is deterministic (that is, tests do not fail spuriously), however
there might arise problems when executed in unusual environments. For instance,
some tests contain unicode characters and require the environment to have
appropriate locale set. Also, issues with local musl version might cause test
failures.

Currently, extension of the test suite is a major goal. It helps significantly
in eliminating bugs, bringing kefir closer to C language standard support,
improving compiler UX in general.

## Design notes
In order to simplify translation and facilitate portability, intermediate
representation (IR) layer was introduced. It defines architecture-agnostic
64-bit stack machine bytecode, providing generic calling convention and
abstracting out type layout information. Compiler is structured into separate
modules with respect to IR: code generation and AST analysis and translation.
Two code generators are available: naive (old) code generator that produces
straightforward threaded code, and (not yet) optimizing code generator. The
latter includes separate optimizer representation which is derived from the IR.
IR layer provides several interfaces for AST analyzer to retrieve necessary
target type layout information (for instance, for constant expression analysis).
AST analysis and translation are separate stages to improve code structure and
reusability. Parser uses recursive descent approach with back-tracking. Lexer
was implemented before preprocessor and can be used independently of it
(preprocessing stage can be completely omitted), thus both lexer and
preprocessor modules share the same lexing facilities. Driver links kefir as a
library and uses `fork` syscalls in order to isolate each file processing.

## Author and license
Author: Jevgenijs Protopopovs

The code base also includes patches from:
* Brian Robert Callahan - [initial OpenBSD
  port](https://briancallahan.net/blog/20220629.html)

License:
* Main body of the compiler - GNU GPLv3
* Runtime library and includes - BSD 3-clause
  
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

