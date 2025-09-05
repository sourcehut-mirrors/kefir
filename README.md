# Kefir C17/C23 compiler

Kefir is an independent compiler for the C17/C23 programming language, developed
by [Jevgenij Protopopov](https://www.protopopov.lv). Kefir has been validated
with a test suite of 80 software projects, among which are GNU core- and
binutils, Curl, Nginx, OpenSSL, Perl, Postgresql, Tcl and many others. The
compiler targets x86_64 architecture and System-V AMD64 ABI, supporting Linux,
FreeBSD, NetBSD an OpenBSD. While the primary priority is compatibility and
compliance, Kefir also features conservative SSA-based optimization pipeline,
debug information generation, position-independent code, and implements
bit-identical bootstrap. Kefir focuses on C source code translation to assembly,
and integrates with the rest of system toolchain (assembler, linker, standard
library) for other tasks.

**Warning:** the following document might be perceived as overly heavyweight,
long and dense. Audiences less engaged into compiler development technicalities
might want to read [TLDR.md](TLDR.md).

## At a glance

Kefir:
* Supports the C17 standard -- including complex numbers, atomics,
  variable-length arrays, etc. (see *Implementation quirks*).
* Supports the C23 standard -- excluding `_Decimal` numbers, but including the
  rest (see *Implementation quirks*).
* Supports some of widespread GNU C built-ins, certain extensions, inline
  assembly.
* Is written in C11 -- runtime dependencies are limited to the standard library,
  bits of POSIX and the shell.
* Targets x86_64 and System-V ABI -- primarily Linux (both glibc and musl libc),
  secondarily FreeBSD, NetBSD and OpenBSD (see *Supported environments*).
* Is extensively validated on real-world open source software test suites --
  including dozens of well-known projects (see *Testing and validation*).
* Implements conservative SSA-based optimizations -- primarily targetting local
  integral scalars: local variable promotion to registers, dead code
  elimination, constant folding, global value numbering, loop-invariant code
  motion, function inlining, tail-call optimization (see *Optimization and
  codegen*).
* Supports DWARF5 debug information, position-independent code, AT&T and Intel
  syntaxes of GNU As and has limited support for Yasm.
* Implements bit-identical bootstrap -- within fixed environment, Kefir produces
  identical copies of itself.
* Is able to generate freestanding assembly code -- with the exception for
  thread-local storage (might require external calls per ABI) and atomic
  operations of non-platform native sizes, which require `libatomic`-compatible
  library.
* Provides `cc`-compatible command line interface.
* Is able to output internal representations (tokens, abstract syntax tree,
  intermediate representation) in machine-readable JSON form.
* Provides auditable logs and all build artifacts for pre-release testing of the
  most recent release.
* Licensed under GNU GPLv3 (only) terms for the compiler, and BSD-3 terms for
  runtime includes (see *License*).
* Is written by a single developer.
* Is named after [fermented milk drink](https://en.wikipedia.org/wiki/Kefir) --
**no other connotations are meant or intended**.

**Important note:** as the project is developed and maintained by a single
person, unfunded and in spare time, the author warns that uses of Kefir in
production settings might be undesirable due to insufficient level of support
the author can provide.

**Important note #2:** to the best of author's knowledge, all of the claims
above are true (and many are reproducibly demonstrated by the test suite). Yet
even with full rigour, many bugs, unintended omissions, inconsistencies and
misunderstandings may slip through. The author intends to faithfully represent
capabilities of the project, and is especially sensitive to any overstatements
in this regard. If you have any doubts, objections or otherwise disagree with
the above, please do not hesitate contact the author (see *Author and contacts*)
-- corrections will be issued immediately after identifying the deficiency.

## Installation and usage

On supported platforms, Kefir is built and tested as follows:
```bash
make test all                  # Linux glibc
make test all USE_SHARED=no CC=musl-gcc KEFIR_TEST_USE_MUSL=yes   # Linux musl
gmake test all CC=clang        # FreeBSD
gmake test all CC=clang AS=gas # OpenBSD
gmake test all CC=gcc AS=gas   # NetBSD
```
The installation is done via `(g)make install prefix=...`. The default prefix is
`/opt/kefir`.

Kefir build time dependencies are:
* C11 compiler -- tested with `gcc` and `clang`.
* Bash
* GNU Make
* GNU Coreutils
* Groff
* m4
* mandoc

Kefir runtime dependencies are:
* The C standard library and POSIX
* Shell
* Furthermore, for correct end-to-end compilation, Kefir requires:
    * External assembler -- full support for GNU As, limited support for Yasm
    * External linker -- GNU ld
    * External libc -- glibc or musl libc on Linux, system libc on *BSD systems.
    Note that this might be different from the libc Kefir itself is linked with.
    * External startfiles -- `crti.o`, `Scrt.o`, etc.
    * `libatomic`-compatible library (i.e. `libatomic` of gcc or `compiler_rt`
      of clang).

Users can consult `dist/Dockerfile*` files that document the necessary
environment for Ubuntu (`base` target), Fedora and Alpine, respectively, as well
as `dist/PKGBUILD` for Arch Linux. For *BSD systems, consult respective
`.builds/*.yml` files.

**Note:** upon build, Kefir detects host system toolchain (assembler, linker,
include and library paths) and configures itself respectively. Upon update of
the toolchain, Kefir provides `kefir-detect-host-env --environment` command
whose output shall be placed into `$(prefix)/etc/kefir.local` file.

**Note #2:** aforementioned dependencies do not include optional development and
full test suite dependencies. For these, please consult `dist/Dockerfile` `dev`
and `full` targets.

At the moment, Kefir is automatically tested in Ubuntu 24.04, FreeBSD 14.x,
OpenBSD 7.5 and NetBSD 10.x environments; Arch Linux used as the primary
development environment.

### Usage

Kefir implements `cc`-compatible command line interface and therefore can be
used as a near-drop-in replacement (see *Implementation quirks*) of `cc` in
standard compilation pipelines:

```bash
which kefir # Should output correct path to kefir after installation

# Example usage
kefir -O1 -g -fPIC -o hello_world ./hello_world.c
./hello_world
```

Furthermore, kefir provides a manual page that documents command-line options
and environment considerations:
```bash
man kefir # Make sure that kefir installation directory is available to man
kefir --help # Identical to the manual page contents
```

### Portable Kefir

Kefir provides scripts to build portable and standalone Kefir distribution
package for Linux. The package includes statically-linked Kefir C compiler, musl
libc, and selected tools from GNU Binutils. The package is intended to provide a
minimalistic C development toolchain independent of host system tooling.

```bash
make portable_bootstrap -j$(nproc)
# Build artifact is located in bin/portable/kefir-portable-*.tar.gz
```

### Web playground

For demonstration purposes, Kefir provides an optional web playground page,
similar to a simplistic [Godbolt](https://godbolt.org/) version. The web
playground version hosted at [project's
website](https://kefir.protopopov.lv/playground/) provides latest released Kefir
version with musl libc headers. Please note that the playground runs with
JavaScript and WebAssembly, and requires particular Emscripten version to build
(see `dist/Dockerfile` for precise setup):
```bash
make webapp -j$(nproc)
```

Please also note that WebAssembly environment is not a normal Kefir operating
environment, and certain bugs or issues might be uncovered that are not present
normally.

## Supported environments
Kefir targets x86_64 instruction set architecture and System-V AMD64 ABI.
Supported platforms include modern versions of Linux (glibc & musl libc),
FreeBSD, OpenBSD, NetBSD operating systems. A platform is considered supported
if:

* Kefir can be built with system compiler and successfully executes own test
  suite (see *Testing and validation*).
* Kefir can be built with itself and successfully executes own test suite.
* Kefir passes `c-testsuite` and `gcc-torture` tests (see *Testing and
  validation* for the exact tests, conditions, etc).
* Kefir can compile Lua and run its base test suite.
* Kefir can perform reproducible (i.e. bit-identical bootstrap) of itself within
  fixed environment.

To claim a platform supported, no other requirements are imposed. Other tests
and validations described in *Testing and validation* section are focused
predominantly on Linux to ensure overall compilation process correctness. In
general, there are very few differences between Linux and BSD system code
generation, thus full *testing and validation* sequence shall suffice only on a
single platform. Please note that libc header quirks are generally the main
offender of compatibility, thus additional macro definitions or individual
header overrides might be necessary. Musl libc provides the most smooth
experience, however Kefir has accumulated sufficient support for GNU C
extensions to use glibc and BSD libc implentations reasonably (consult
*Implementation quirks* and the external test suite part of *Testing and
validation*, as well as respective `.build/*.yml` files for platform of choice
for detailed examples).

As mentioned in the *Installation* section, Kefir detects system toolchain
configuration on build and uses it later. The compiler also supports a set of
environment variables that take precedence over the built-in configuration.
Consult respective section of the manual page for details of supported
environment variables.

### Standard library considerations

On Linux, Kefir works with both glibc and musl libc. Musl headers are more
standards-compliant and generally provide smoother compatibility. glibc, by
contrast, may introduce incompatibilities with non-mainstream compilers (see
*Implementation quirks*).

On FreeBSD, OpenBSD, and NetBSD, the system standard library can be used, though
additional macro definitions (e.g. `__GNUC__`, `__GNUC_MINOR__`) may be required
for successful builds.

## Implementation quirks

The following details need to be taken into account:

* Kefir implementation of C23 standard misses the support for `_Decimal`
  floating-point types. The standard designates this feature as optional,
  conditioned on `__STDC_IEC_60559_DFP__` presence.
* The C23 standard mandates use of Unicode for `char8_t`, `char16_t` and
  `char32_t` types and literals. Kefir relies on the standard library wide
  character encoding facilities, and thus implements this requirement under
  condition that the system locale is Unicode-based. The author believes that
  this is a reasonable assumption.
* In general, the author has much higher confidence in compatibility with
  features of C17 and earlier versions. As of current version of external test
  suite (see *Testing and validation*), the absolute majority of third-party
  projects do not rely on any of C23 features, which makes external validation
  of C23 support much more limited. Hereby, the author affirms that they have
  faithfully read the changes included into the C23 standard and implemented
  these in good conscience and to the best of their ability, including
  implementing own tests for respective features. 
* With glibc on Linux, there exist corner cases where the library breaks certain
  features on non-mainstream compilers. For instance, glibc overrides
  `__attribute__` specificaton with an empty macro, omits `packed` attributes,
  etc., breaking compatibility despite the fact that respective features are
  supported by Kefir. The author recommends getting acquainted with project
  build configurations from the external test suite (see *Testing and
  validation*) to learn about fixups for typical issues. While such fixups could
  be provided along with other Kefir runtime headers, the author has
  deliberately avoided including any library- or platform-specific hacks into
  Kefir.
* Kefir does not explicitly implement `STDC` floating-point environment pragmas.
  The compiler does not optimize floating-point operations, therefore these
  pragmas are effectively no-op.
* Atomic operations implement sequentially-consistent semantics irrespective of
  specified memory order. This behavior is safe and shall not break any
  software. Atomic operations of non-native sizes rely on external software
  atomic library (`libatomic` from gcc, or `compiler_rt` from clang). Kefir
  links resulting executables with the library automatically in all
  configurations except musl libc. Furthermore, use of `<stdatomic.h>` system
  header from Clang requires `-D__GNUC__=4 -D__GNUC_MINOR__=20` command line
  arguments.
* Should atomic operations on long double variables (both scalar and complex) be
  used, care needs to be taken due to the fact that the last 48 bits of each
  long double storage unit may be uninitialized. Kefir implements zeroing of
  uninitialized padding to mitigate possible issues.
* Reliance on the host C standard library implies that Kefir needs to implement
  any built-ins or compiler extensions that appear in the library headers. The
  author has introduced a substantial number of built-ins into the compiler,
  however cannot guarantee completeness in that sense. Should any of standard C
  library functions be unusable on the supported platforms due to missing
  builtins or extensions, this will be treated as a bug.
* All relevant versions of the C language standard are officially available only
  at a substantial cost. When working on the compiler, the author has relied on
  publicly available drafts of the standard (see *Useful resources and links*).
  Should any of these drafts be in contradiction with the final standard, the
  author will be interested in hearing specific details.

### In practice

Several practical considerations users of Kefir might need to take into account:

* Kefir cannot directly compete with well-established major compilers such as
  GCC or Clang in terms of raw performance, portability or breadth. The purpose
  of Kefir project is producing an independent C17/C23 compiler with
  well-rounded architecture and well-defined scope that is feasible for
  implementation by a single developer.
  * Especially, in terms of raw performance, Kefir might be lacking compared to
    more performance-focused projects. The author still sees many low-hanging
    fruits in register allocation, optimization passes, instruction selection,
    scheduling, etc.
  * In terms of GNU C compatibility, Kefir implements sufficient amount of
    extensions and builtins to be practically useful. Exhaustive list of
    builtins is available at
    `source/tests/end2end/supported_builtins1/lib.asmgen.expected`.
* After taking into account certain quirks (see *Implementation quirks*), Kefir
  can be used as a near-drop-in replacement for host C compiler to compile and
  successfully run major well-known C projects, as demonstrated by *Testing and
  validation*. 
* In terms of C17/C23 compatibility, the author's intention is close-to-near
  compability with language standards. Any behavioral divergence not documented
  in the *Implementation quirks* shall be considered a bug in the compiler.

## Testing and validation

### Own test suite

The own test suite of the Kefir compiler is maintained by the author as part of
the project code base. As a general rule, own test suite is extended to cover
any changes made in the compiler. Exceptions are made when existing tests
already cover the change, or when a change cannot reasonably be tested (e.g.,
reproducing a specific bug would require a prohibitively long case). Own test
suite includes the following categories of tests:

* Partial tests -- exercise individual components, subsystems or subsystem
  combinations. Rely on hand-crafted initialization:
  * Unit tests -- Kefir implements custom unit testing library.
  * Integration tests -- uses snapshot testing techniques (i.e. comparison with
    expected output) to test individual subsystem.
  * System tests -- initializes Kefir submodules to output assembly code,
    assembles and links it with a counterpart module built by host C compiler.
    The counterpart module provides a harness to test Kefir outputs.
* end2end tests -- cover the complete compiler pipeline, consist of a set of
  `*.kefir.c` and `*.host.c` modules, which are built by Kefir and host C
  compiler respectively, linked together and executed. Typically, host modules
  provide a harness to test Kefir similarly to system tests. Optionally, end2end
  tests might also include snapshot (`asmgen`) tests.
  * Selected test cases generated by CSmith are also included into end2end test
    suite.

Historically, development relied mainly on partial tests before the compiler
pipeline was complete. Today, most new work is validated primarily with end2end
tests.

In continuous integration environment on Linux glibc and FreeBSD platforms, own
test suite is executed with Valgrind and undefined behavior sanitizer.
Furthermore, on all supported platforms special "self-test" run is executed,
where Kefir acts as host compiler.

Consult `ubuntu.yml`, `ubuntu-musl.yml`, `ubuntu-self.yml`, `freebsd.yml`,
`freebsd-self.yml`, `openbsd.yml`, `netbsd.yml` from `.builds` directory for
detailed setup for own test suite execution on the platform of choice.

### Bootstrap test

On all supported platforms, Kefir also executes reproducible bootstrap test:

0. Kefir is built with host C compiler normally. This build is referred to as
   `stage0`.
1. `stage0` kefir builds itself to produce `stage1`. All intermediate assembly
   listings are preserved.
2. `stage1` kefir builds itself to produce `stage2`. All intermediate assembly
   listings are preserved.
3. Assembly listings from `stage1` and `stage2` shall be identical. Furthermore,
   sha256 checksums for `kefir` executable and `libkefir.so` library from
   `stage1` and `stage2` shall be identical too for bootstrap test to succeed.
  
Bootstrap test is performed within fixed environment (i.e. standard library,
assembler, linker versions are not changed during the test), and demonstrates
that Kefir is able to produce identical copies of itself. On Ubuntu, bootstrap
is performed using both GNU As and Yasm as target assemblers.

Consult `ubuntu-other.yml`, `ubuntu-musl.yml`, `freebsd.yml`, `openbsd.yml`,
`netbsd.yml` from `.builds` directory for detailed setup for bootstrap test
execution on the platform of choice.

For practical purposes, Kefir can be bootstrapped by specifying itself as a `CC`
compiler:
```bash
make CC=$(which kefir) -j$(nproc)
```
This form of bootstrap does not verify reproducibility, it simply rebuilds the
compiler using itself.

#### Portable bootstrap

Portable Kefir bootstrap procedure as described in the *Installation* section is
also used in a role of an additional test. The portable bootstrap omits
bit-precise reproducibility check, but performs iterative rebuild of complete
toolchain (musl libc, GNU As, GNU ld) at each stage. Therefore, it ensures that
Kefir is capable of producing a self-sustaining development environment.

### c-testsuite and gcc-torture suites

On all supported platforms, Kefir executes the following external test suites:

* c-testsuite -- a smaller test suite, relies on compiling and executing test
  cases, and comparing their output to an expected snapshot. Out of 220 tests, 3
  test files rely on non-standard extensions and are skipped, the rest shall
  pass.
* GCC Torture suite -- a test suite imported from gcc 15.1.0, consists of
  independent test cases that perform self-testing in a form of assertions. The
  test suite heavily relies on gcc-specific features, therefore higher degree of
  failures is expected. As of current version, out of 3663 tests, Kefir fails
  455 and skips 29. Note that the exact number of failed tests might slightly
  vary depending on the target platform and hardware performance (due to
  enforced 10 second timeout for execution). Reported number is the best-case
  result on Ubuntu glibc. Furthermore, note that in order for test to succeed,
  none of the failures shall be caused by fatal issues, aborts, segmentation
  faults or caught signals, either on runtime or in compile time.
* GCC test suite `_BitInt` bits -- a separate set of 71 tests imported from gcc
  15.1.0 to ensure correct implementation of bit-precise integers from the C23
  standard. All tests from this suite shall run successfully.

### Lua basic test suite

On all supported platforms, Kefir is used to build Lua 5.4.8 and execute its
basic test suite, which should pass completely. Purpose of this test is
demonstration that Kefir is able to successfully build non-trivial software on
the target platform. Technically, this is a part of the external test suite (see
below), and its inclusion into the general test runs has happened for historical
reasons.

### Fuzz testing

Starting from the release 0.4.1, each Kefir pre-release testing includes
differential testing against the most recent version of gcc with at least 10000
randomly generated test cases (via csmith). For all test cases that can be
compiled and executed by both kefir and gcc within given timeout, outputs shall
be identical. All failing cases are fixed and added to the own test suite.

### External test suite

This is a suite of 80 third-party open source projects that are built using
Kefir with subsequent validation: for most projects, their test suite is
executed; where this is not possible, a custom smoke test is performed; for the
minority, the fact of a successful build is considered sufficient. Purpose of
the external test suite is:

* Establishing correctness and real-world applicability of the Kefir compiler.
  Compiling many well-known software projects, such as GNU binutils, coreutils,
  Ruby, Python, Perl, OpenSSH, OpenSSL, zsh and others demonstrates of Kefir
  capabilities.
* Tracking regressions during the development cycle. The external test suite is
  diverse enough to be sensitive to possible regressions, quickly exposing many
  newly introduced deficiencies. This helps author to resolve problems quickly
  and establish confidence.
* Document resolution of challenges arising when building real-world software
  with Kefir on Linux glibc. Such challenges include mitigating glibc-related
  issues, fixing build system assumptions, etc.

Except for Lua, the external test suite is executed exclusively in Linux glibc
environment as defined by `dist/Dockerfile`. Primary reason for that is resource
constraints. Execution of the external test suite is fully automated:

```bash
make .EXTERNAL_TESTS_SUITE -j$(nproc)
make .EXTERNAL_EXTRA_TESTS_SUITE -j$(nproc) # only for zig-bootstrap, see below
```

The external test suite (except for zig-bootstrap) is executed on a daily basis
on current development version of Kefir, as well as at pre-release stage.

All source archives of third-party software included in the external test suite
are mirrored at [project's
website](https://kefir.protopopov.lv/archive/third-party/) for reproducibility
and completeness purposes. This mirror contents track the current state of
`master` branch along with recent additions to the `develop` branch. The author
intends to provide separate archival versions for each released Kefir version
starting from the version 0.5.0. By default, all external tests still use the
original upstream links to the third-party software sources, however these can
optionally be replaced with an archival version.

#### Limitations

* Generally, most of the source code is built and executed "as-is" without any
  modifications. However, due to multitude of reasons (bypassing glibc quirks,
  fixing hard-coded compiler assumptions, replacing exotic non-standard idioms,
  ignoring deliberately suppressed test case, etc.) patches might be applied.
  The author has established the following principle: any such patch shall be
  trivial and exceedingly small, non-trivial changes are never considered.
* Individual test cases of some projects might get suppressed. Reasons for that
  typically include too strong assumptions about the compiler or the
  environment, there do exist several test cases where the author is unsure
  about the exact reason of instability. However per author's estimate >99% of
  individual tests do pass successfully unmodified.
* Test suite of this size might exhibit certain degree of flakiness naturally.
  During some of the daily builds, the author has observed failures that were
  unrelated to any of the compiler changes, but due to such reasons as: network
  failures, calendar date changes, CPU throttling and related slowdowns (certain
  tests rely on timings). Such failures typically are one-off and are never
  observed repeatedly. Note that running the suite on different hardware might
  possibly expose some failures that were not observed by the author.

The author believes that outlined limitations do not undermine purpose and
utility of the external test suite. 

#### Structure of the external test suite

The software included into the external test suite can be broadly grouped as
follows. Provided software list is not exhaustive, please look up the
`source/tests/external` for complete details and specific versions. As a general
rule, the author performs upgrades for most packages prior to each Kefir
release.

* Widely used software packages -- GNU Bash, Binutils, Bison, Coreutils, Curl,
  GNU Awk, Git, Guile, Gzip, ImageMagick, libraries such as
  expat/gmp/jpeg/png/uv/xml2, Lua, GNU Make, Memcached, Musl, Nano, Nasm, Nginx,
  OCaml, OpenSSH, OpenSSL, PCRe2, Perl, PHP, PostgreSQL, Python, Redis, Ruby,
  SQLite, tar, Tcl, Vim, Wget, xz, zlib, zsh, zstd, and a few others. This is
  the largest group, and it serves all purposes outlined above: correctness,
  regression tracking, and documenting real-world build capability and
  challenges. GCC 4.0.4 bootstrap procedure also belongs to this group.
  * zig-bootstrap technically belongs to this group too, but due to unreasonable
    CPU time and memory requirements it is executed less frequently.
* Problem-specific software -- c23doku, jtckdint and couple of other small early
  adopters of C23. The main role of this group is testing some specific aspects
  of Kefir (e.g., checked arithmetic builtins for jtckdint, C23 feature support
  for others).
* "Reciprocal" projects -- hummingbird, libsir, oksh, slimcc, tin. These are
  projects that have acknowledged Kefir existence in some way, often early on.
  As a gesture of reciprocity, they are included in the external test suite.

#### Execution schedule and environment

Due to resource limitations, the external test suite is executed outside of
normal continuous integration pipeline on a dedicated hardware. The computer has
an Intel i5-6500 CPU with 4 cores, 8 GB of RAM and the same amount of swap
space. All external tests except for zig-bootstrap are executed in a container
as defined by `dist/Dockerfile`. Zig-bootstrap is excluded from the
containerized runs and is executed on author's personal development machine due
to its higher memory requirements.

The external test suite is executed daily on a `develop` branch of Kefir,
provided there have been any changes to that branch. The execution schedule is
manually managed by the author. While the logs and any other artifacts from each
run are not published, the author intends to publish complete set of external
test logs before each Kefir release.

### Pre-release testing

Starting from the version 0.5.0, each Kefir release will be accompanied with the
following artifacts:

* A complete set of Linux tests as specified by `scripts/pre_release_test.sh` in
  the environment as specified by `dist/Dockerfile`. These tests include own
  test suite in all configurations (with glibc & musl gcc/kefir host, clang
  host), reproducible bootstrap test in all configurations (GNU As & Yasm
  targets with glibc & musl libc), portable bootstrap run, run of the external
  test suite (with exception for zig-bootstrap), a build of web playground. A
  complete set of logs for the whole run will be preserved and published, along
  with an archive of external test sources and a container image with the test
  execution environment and all build artifacts and intermediate files.
* zig-bootstrap external test will be performed in the same environment on a
  different machine. A complete set of logs and a container image will be
  published too.
* A set of logs produced by the SourceHut build service for all continuous
  integration manifests from `.builds` directory. These builds include test runs
  for Linux, FreeBSD, OpenBSD and NetBSD in accordance with the requirements
  outlines in *Supported environments* section.

All artifacts will be published in auditable form along with release source code
at [Kefir website](https://kefir.protopopov.lv) and signed with author's [PGP
key](https://www.protopopov.lv/static/files/jprotopopov.gpg).

## Optimization and codegen

### Intermediate representations
Kefir uses multiple different intermediate representations (IR) throughout the
compilation pipeline:

* Stack-based IR -- is a complete intermediate representation that separates the
  AST translation part from any layer below and abstracts out target machine
  details. The stack-based provides facilities to encode type information,
  function signatures and properties, function bodies, inline assembly fragments
  into a unified module. Function bodies are represented by unstructured
  instruction flow. Each instruction operates on a virtual stack, and may have
  immediate parameters. The virtual stack has no fixed element type or width,
  scalar and complex values can be stored uniformly; aggregate values are
  operated on only by reference. The AST translator emits exclusively
  stack-based IR modules.
* Optimizer IR -- is a single static assignment (SSA) based representation that
  is constructed from the stack-based IR before any further transformations.
  This IR encodes stack-based instructions into a SSA form with explicit basic
  block structure, partially defined schedule within the basic block and
  explicit data dependencies. The optimizer IR shares all other aspects (type
  information, function signatures) with the stack-based IR, and thus modules of
  these two IR are tightly coupled after construction. Furthermore, most
  instruction opcodes have one-to-one correspondence between these two IRs, with
  an exception for virtual stack and local variable management instructions. \
  **Note on the partially defined schedule:** by design, within each basic block
  only the instructions that produce side effects (e.g. function calls, certain
  memory accesses, inline assembly, control flow) have strict order. All other
  instructions are left "floating" in the basic block and are scheduled only at
  code generation stage. This design ensures flexibility in optimizations and
  code generation, while preserving simple control flow structure within a
  function. In addition, floating instruction scheduling happens only if they
  are direct or transitive dependencies of the side effect instructions, which
  brings dead code elimination at the code generation stage for free.
* Three-address code -- the final intermediate representation that is used
  exclusively in the code generator. The three address code is referred to as
  "virtualized assembly", as its instructions mostly directly correspond to the
  target machine instruction set (ISA), with an addition of several special
  virtual instructions, whereas its operands typically involve virtual
  registers. Virtual registers have certain type (e.g. general purpose, floating
  point, spill space), and their number is unbounded. Upon generation of
  virtualized assembly from the optimizer IR, each optimizer IR instruction can
  freely use virtual registers to store outputs and intermediate results. When
  certain virtual register mapping to the physical registers is necessary (e.g.
  to uphold function calling convention, use ISA instruction with specific
  requirements, etc), the code generator might express such requirements on
  per-virtual register level. The virtualized assembly is constructed for each
  function individually and disposed immediately after function code generation.
  Register allocation runs on the virtualized assembly, assigning each virtual
  register a physical register or spill slot in accordance with explicit
  requirements, virtual register type and current physical register
  availability. Then the virtual assembly is "devirtualized" by replacing all
  virtual registers with respective physical operands, inserting spill code,
  etc. Certain peephole optimizations run on three-address code level, both
  before and after devirtualization.
  * After devirtualization, the devirtualized assembly is directly mapped into
    textual assembly form. Kefir supports three dialects of GNU As (AT&T, Intel,
    and Intel dialect with prefixes), as well as limited support for Yasm.

As a general rule, the author has completely separated target- and ABI-specific
concerns from the optimization pipeline. The optimizer IR is not aware of any of
these details, and all necessary information (e.g. type layouts) are
communicated from the target ABI module via clearly designed interface. While
this rule excludes or complicates some lower-level optimizations, the author
considers it good for concern separation and maintainability.

### Debugging information

Kefir supports generation of debugging information for GNU As target assembler.
Generated debug information is in DWARF-5 format, and includes mapping between
assembly instructions and source code locations, variable locations, type
information, function signatures. The author has made best-effort attempt to
preserve variable locations across the optimizer pipeline, however certain
optimizations at `-O1` level might disrupt debugging experience significantly.

### Optimization pipeline

Kefir includes the following optimization passes at `-O1` level:
* Function inlining -- the optimizer performs inlining early on in the pipeline.
  The inlining is guided exclusively by the annotations provided by the
  programmer at source code level. Kefir does not implement any heuristics for
  inlining a function, however some annotated function might not get inlined to
  avoid recursion and excessive inlining depth. The author does not view this as
  an optimization per se, but as faithful implementation of programmer's
  annotations.
* Local variable promotion to registers (`mem2reg`) -- the optimizer identifies
  scalar and complex local variables whose addresses never escape a function and
  never alias, and promotes these local variables into SSA registers. This is a
  cornerstone optimization that effectively enables many further analyses.
* Phi propagation -- the optimizer identifies SSA phi nodes that can be replaced
  by direct SSA instruction references.
* Constant folding -- the optimizer identifies all constant subtrees of SSA and
  folds them. Note that this optimization operates only on integral scalar
  values, floating-point constant folding can be implemented only after the
  frontend supports respective `STDC` pragmas.
* Simplification -- this optimization pass combines canonicalization,
  optimization and simplification of many diverse instruction shapes.
  Simplification is implemented as ad-hoc pattern matching upon the optimizer IR
  and runs until fixpoint is reached.
* Local allocation sinking -- the local variable allocations that have not been
  eliminated by the `mem2reg` pass are moved closer to their actual uses to
  make stack frame layout more dense.
* Global value numeric (`gvn`) -- the optimizer identifies instruction subtrees
  that are identical across the function code, and de-duplicates them. Where
  necessary, the de-duplicated subtrees are hoisted. The `gvn` pass only works
  on integral scalar instruction subtrees that do not contain side effects
  (including any memory loads not eliminated by `mem2reg`). Therefore, it is
  very conservative. The author views this as a reasonable trade-off in absence
  of generic alias analysis.
* Loop-invariant code motion (`licm`) -- the optimizer identifies loops and
  groups them into nests. Instruction subtrees within a loop that are not
  dependent on any of loop instructions are hoisted to the outer levels of loop
  nests or outside it. This optimization pass has the same limitations of `gvn`
  pass, as it operates only on side-effect free integral scalar instruction
  subtrees. The author views this pass as a precursor for future more aggressive
  loop optimizations, which are possible only once more sophisticated code
  generation optimizations (incl. rematerialization) are available along with
  better register allocation and target machine cost model.
* Dead code and allocation elimination -- the optimizer identifies and
  eliminates dead instruction and local variable allocations. While this pass is
  not necessary for code generation within current optimizer IR framework, it
  simplifies certain subsequent passes that do not need to consider dead code
  anymore.
* Block merging -- the optimizer identifies basic blocks that can be safely
  merged, does the merge and eliminates the control flow edge.
* Tail call optimization -- as the final part of the optimization pipeline, the
  optimizer identifies potential tail calls. It performs conservative escape
  analysis to verify that none of local variable adresses could have escaped the
  function. The optimization pass does not consider any target-specific aspects,
  therefore the final decision to perform a tail call is done at code generation stage.

In addition, Kefir implements several peephole optimizations at (de-)virtualized
assembly level.

All optimization passes as described above are strictly optional from code
correctness perspective. In addition to these passes, Kefir implements a
lowering pass as part of the pipeline. The lowering pass is necessary to
transform arbitrary-precision arithmetic instructions (used for implementing
`_BitInt` from the C23 standard) into either optimizer-native instruction
arithmetic instructions or supporting routine calls (see *Runtime library* below).

**Optimization levels:** at the moment, Kefir supports two optimization levels
`-O0` and `-O1` (anything else is considered equivalent to `-O1`). Both levels
include function inlining, local allocation sinking, dead code and dead
allocation elimination and lowering passes. In addition, `-O1` contains all
passes described above with some repetitions. Consult `source/driver/driver.c`
for the precise optimization pipeline, and consult the manual page for
command-line options to define the optimization pipeline passes explicitly.

Kefir focuses the optimizations predominantly on integral scalars limited to a
single function scope. Aggregate values are mostly optimized with respect to
redundant copy elimination upon function calls. Inter-procedural optimizations
are limited to function inlining. Both alias analysis and escape analysis are
very rudimentary, and are limited to the needs of very specific optimization
passes. The author sees no major obstacles for implementing more aggressive
optimizations within current optimizer IR framework, however current pipeline
has been characterized by the author as conservative as it does not change the
order of non-local memory accesses within a function.

#### Runtime library

With exception for non-native atomic operations which require `libatomic`, Kefir
generates self-contained assembly listings and requires no runtime library of
its own. Code generator typically inlines implementations for most of
instructions into the target function directly. The sole exception to this are
arbitrary-precision arithmetics instructions, that are necessary to support
`_BitInt` feature of the C23 standard. For these instructions, Kefir issues
function calls and appends necessary functions with internal linkage to the end
of the generated assembly listing.

## Goals and priorities

As a project, Kefir has the following goals, in order of priority:

* Independence. Within its scope, which is C17/C23 source code to assembly
  translation, Kefir shall be independent and do not rely on any parsing,
  compiler or code generation frameworks/libraries. Outside of the scope, Kefir
  shall integrate with system toolchain components.
* Correctness, compatibility and compliance. Kefir shall remain compliant with:
  1. C17 and C23 language standards. Any deviation from the standards, unless
     explicitly documented as a quirk, shall be considered a bug in Kefir.
  2. System-V AMD64 ABI. Kefir shall produce code that can be freely linked with
     object files or libraries produced by other compilers on the same platform
     without introducing any ABI issues.
  3. Other relevant documents (DWARF-5 standard, Thread-local storage models).
     Kefir shall comply where possible with other documents defining the
     platform binary interface or environment to facilitate complete
     compatibility.
  4. Popular C language extensions (GNU C extensions, gcc built-ins).
     Implementing complete set of language extensions is not a goal of Kefir,
     but a reasonable amount of extensions shall be supported in order to
     compile real-world software (such as included in the external test suite).
     Degree of support and compatibility with each particular extension might
     vary.
  5. Command-line interface. Kefir implements `cc`-compatible command line
     interface, extended with certain options supported by gcc or clang
     compilers. The goal is to serve as a drop-in replacement of `cc` in cases
     where limitations documented in *Implementation quirks* are observed.
* Limited scope. Kefir is focused on source-to-assembly translation and
  integration with other system tools. Implementation of other parts of
  toolchain (libc, assembler, linker) or other languages/dialects is currently
  not considered a goal for the project.
* "Well-roundness". Kefir shall exhibit reasonable architecture with all stages
  expected from a credible C compiler. Performance (both run and compile time)
  of certain stages might be lacking, however all stages shall be present and
  architecturally sound. Project architecture shall permit iterative approach at
  refining certain stages and improving the compiler.
* Performance. Once all other goals are reasonably observed, the project might
  include enhancements in performance, in the broadest sense. The notion of
  performance in this context includes both compiler performance and efficiency
  of produced code. Optimizer passes and code generated as described in the
  section *Optimization and codegen* are first and foremost defined to satisfy
  the correctess and well-roundness condition, with performance enhancements
  coming later.
* Portability. Kefir shall be portable across Unix-like x86_64 systems. Support
  for non-x86_64 or non-Unix platforms is currently considered non-goal, but
  that might change in future.

## History and future plans

The project has been in active development since November 2020. In that
time-span, the author has released several intermediate versions, with complete
descriptions available in the `CHANGELOG`. It shall be noted that the versioning
scheme is inconsistent, and can be characterized as "vibe-versioning" (i.e.
absence of strict versioning scheme and relying on author's personal feeling
about the release).

* 0.1.0 -- released in September 2022. Represents the first two years of working
  on the project, and provides basic C17 compiler with several omissions
  targetting Linux and BSD systems, capable of bootstrapping and building a few
  real projects. The compiler is based on threaded-code execution model and does
  not perform any optimizations.
* 0.2.0 -- released in July 2023. The project had introduced a new optimizer IR
  and code generator, however all optimizations are deferred to the subsequent
  releases.
* 0.3.0 -- released in August 2023. The project had been augmented with some
  optimizations and usability enhancements.
* 0.3.1 -- released in November 2023. The project had acquired a new code
  generator. At this point, project's architecture transforms into its current
  form.
* 0.4.0 -- released in September 2024. Implementation of missing C17 features,
  debug information generator, introduction of the external test suite.
* 0.4.1 -- released in February 2025. Major improvements in generated code
  correctness and compatibility, significant extension of the external test
  suite.
* 0.5.0 -- released in September 2025. Includes substantial refactoring and
  improvement of optimizer IR structure, new optimization passes, another
  significant extension of real-world compatibility.

After 0.5.0, the author plans to make a minor release 0.5.1, content of which is
subject to feedback received on 0.5.0. At this point, it shall be noted, that
due to the nature of work on the project, the author does not want to make any
explicit or implicit promises or commitments. In essence, any commit to the
project might be the last one without prior notice. Nevertheless, in the event
of terminating (or indefinitely pausing) development of Kefir, the author will
make an attempt to communicate it cleanly.

While not having a definite roadmap, future developments of Kefir might include:

* Improvements in optimization and code generation. 
* General refactoring and optimization of the compiler itself.
* Porting Kefir to non-Unix operating systems and non-x86_64 targets.
* Extending Kefir with a custom assembler and linker, tailored specifically to
  Kefir needs.
* Creating spin-off compilers that use Kefir as a platform.
* Refactoring Kefir to enable bootstrap in environments missing standard
  library.

The above list is neither complete, nor definitive, and shall serve just as an
informative note on the areas that the author considers worthy of exploration at
the moment.

## Distribution

Kefir is distributed exclusively as source code, which can be obtained from the
following sources:

* [Kefir project at Sourcehut](https://sr.ht/~jprotopopov/kefir/) -- the primary development repository.
* [Author's personal git repository](http://git.protopopov.lv/kefir) -- a mirror.
* [Kefir repository at Codeberg](https://codeberg.org/jprotopopov/kefir) -- a secondary mirror.

The author publishes release tarballs at the [project's
website](https://kefir.protopopov.lv). The author recommends to obtain the
source code from `master` branch of any of the official mirrors, as it might
contain more up-to-date code and each merge to that branch is tested as
thoroughly as releases.

In addition, the author maintains two `PKGBUILD` build scripts at ArchLinux User
Repository: [kefir](https://aur.archlinux.org/packages/kefir) and
[kefir-git](https://aur.archlinux.org/packages/kefir).

The author is aware of binary packages of kefir produced by the third parties.
The author is not affiliated with any of these package maintainers, so use at your own discretion:

* [Fedora package](https://packages.fedoraproject.org/pkgs/kefir/kefir/)
* [Alpine package](https://pkgs.alpinelinux.org/package/edge/community/x86_64/kefir)
* [Ubuntu PPA](https://codeberg.org/tkchia/ppa-de-rebus)

## License

The main body of the compiler code is licensed under GNU GPLv3 (only) terms, see
`LICENSE`. Please note the **only** part: Kefir does not include any "later
version" clause, and publication of new GNU GPL versions does not affect the
project.

The arbitrary-precision integer handling routines (`headers/kefir_bigint`) and
runtime headers (`headers/kefir/runtime`) are licensed under the terms of BSD
3-clause license. Code from these files is intended to be included into
artifacts produced by the compiler, therefore licensing requirements are
relaxed. Furthermore, when these files are used as part of normal compilation
pipeline with Kefir, their licensing can be treated as being in the spirit of
[GCC Runtime Library
exception](https://www.gnu.org/licenses/gcc-exception-3.1.html). In such cases,
the author does not intend to enforce redistribution clauses (#1 and #2) of BSD
license in any way.

For clarity, most source files in the repository include a license and copyright headers.

### External test suite exceptions
The author must note that there exist several files as part of the external test
suite glibc fixups (see, for instance,
`source/tests/external/nginx/include/sys/cdefs.h`) that are modified excerprts
of glibc source code. Kefir author makes no claims with respect to such files
and provides them only as means to successfully compile the respective external
test.

## Contributing

The author works on the project in accordance with extreme [cathedral
model](https://en.wikipedia.org/wiki/The_Cathedral_and_the_Bazaar). Any
potential external code contributions shall be discussed in advance with the
author, unless the contribution is trivial and is formatted as a series of short
commits that the author can review "at a glance". Any unsolicited non-trivial
merge requests that did not undergo prior discussion might get rejected without
any further discussion or consideration.

Nevertheless, the author welcomes non-code contributions, such as bug reports,
bug reproduction samples, references to relevant materials, publications, etc.

## Useful resources and links

Fundamental information:
* [C11 standard final working
  draft](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
* [C17 standard final working
  draft](https://files.lhmouse.com/standards/ISO%20C%20N2176.pdf)
* [Clarification Request Summary for
  C11](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2244.htm) - DRs
  included in C17 language standard as compared to C11.
* [The first C2Y draft](https://open-std.org/JTC1/SC22/WG14/www/docs/n3220.pdf)
  - shall be content-wise equivalent to the published C23 standard.
* [ELF Handling For Thread-Local Storage](https://www.akkadia.org/drepper/tls.pdf)
* [DWARF Version 5](https://dwarfstd.org/dwarf5std.html)
* [POSIX c99 specification](https://pubs.opengroup.org/onlinepubs/9699919799/)
* Platform-specific:
  * [System-V AMD64 ABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
  * [AMD64 instruction set
    reference](https://www.amd.com/system/files/TechDocs/24594.pdf)

Useful tools:
* [Compiler explorer](https://godbolt.org/)
* [Record and Replay framework](https://rr-project.org/)
* [CSmith](https://github.com/csmith-project/csmith)

Supplementary information:
* [C reference](https://en.cppreference.com/w/c)
* [x86 instruction set reference](https://c9x.me/x86/)
* [x86 and amd64 instruction reference](https://www.felixcloutier.com/x86/)

Kefir-specific links:
* [Author's personal website](https://www.protopopov.lv)
* [Kefir website](https://kefir.protopopov.lv)
* [Kefir Web playground](https://kefir.protopopov.lv/playground/)
* [Kefir external test archive](https://kefir.protopopov.lv/archive/third-party/)
* [Kefir project at Sourcehut](https://sr.ht/~jprotopopov/kefir/) -- the primary development repository.
* [Kefir mirror in author's personal git repository](http://git.protopopov.lv/kefir) --
  a mirror.
* [Kefir repository at Codeberg](https://codeberg.org/jprotopopov/kefir) -- a secondary mirror.
* [Author's PGP key](https://www.protopopov.lv/static/files/jprotopopov.gpg) --
  all git tags, releases and related artifacts will be signed with it until
  further notice.

Trivia:
* [Fermented milk drink](https://en.wikipedia.org/wiki/Kefir)
* [The Cathedral and the Bazaar](https://en.wikipedia.org/wiki/The_Cathedral_and_the_Bazaar)

## Acknowledgements

The author would like to acknowledge (in no particular order) many different
people that have influenced author's intention, motivation and ability to work
on Kefir:

* The original authors and designers of the C programming language and UNIX:
  Dennis Ritchie and Ken Thompson. Undoubtedly, any of the work on the Kefir is
  only possible because the author is standing on the shoulders of giants.
* The original authors of major C compilers -- Richard Matthew Stallman of GNU
  Compiler Collection, and Chris Lattner or Clang -- as well as Fabrice Bellard
  who is the author of Tiny C Compiler. Works of these people have inspired
  creation of Kefir.
* The authors and contributors of software projects that Kefir relies upon in
  any way in its build or development process: Linux kernel, FreeBSD, OpenBSD,
  NetBSD, GNU project, Clang, Musl libc, CSmith, and any other smaller projects.
* In particular, the author wants to emphasize the [Record and Replay
  framework](https://rr-project.org/) project. While less known, it has been
  instrumental in investigating failures in Kefir and software compiled by it,
  and this project had non-trivial impact onto Kefir's current state.
* Once again, Richard Matthew Stallman as an author of GNU General Public
  License. The author believes that GNU GPL has been the cornerstone in
  establishing free software movement.
* Authors and contributors of all projects used in the external test suite.
  These are too numerous to list here, so please refer to
  `source/tests/external`.
* Hsiang-Ying Fu, the developer of slimcc, who has compiled a wonderful
  [collection](https://github.com/fuhsnn/slimcc/blob/main/scripts/linux_thirdparty.bash)
  of C projects for compiler validation.
* Dr. Brian Robert Callahan, whose
  [blogpost](https://briancallahan.net/blog/20220629.html) has motivated the
  author to resume work on the compiler.
* Anybody else who has noticed and acknowledged Kefir development in early
  stages.
* Friends and relatives of the author, who over the years happened to be
  listeners to ramblings on C compiling topics.

## Author and contacts

The project has been architected, engineered and implemented single-handedly by
[Jevgenij Protopopov](https://www.protopopov.lv) (legal spelling: Jevgēnijs
Protopopovs), with the exception for two patches obtained from third parties:
* Brian Robert Callahan - [initial OpenBSD
  port](https://briancallahan.net/blog/20220629.html)
* remph - [Treat zero-length environment variables as unset](https://lists.sr.ht/~jprotopopov/public-inbox/patches/57368)

The author can be contacted [by email](mailto:jevgenij@protopopov.lv) directly,
or via the [mailing list](https://lists.sr.ht/~jprotopopov/public-inbox).

Development of the project has been conducted independently without external
sources of funding or institutional support.