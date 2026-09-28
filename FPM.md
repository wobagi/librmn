# Building and using librmn with fpm

[librmn](README.md) can be built with the [Fortran Package Manager
(fpm)](https://fpm.fortran-lang.org) in addition to its historical CMake
build.  This document describes how to build it with fpm, how to use it from
another fpm project, and the limitations that come with it.

Everything needed is already in the repository:

```
fpm.toml                     the librmn package
fpm/libapp/                  fpm package wrapping the App submodule
fpm/include/                 hand written headers (see "Generated files")
tests/legacy/                legacy test programs moved out of src/
```

Only tested with `fpm 0.13.0-alpha` and `gfortran 16.2.1` (gcc 16.2.1).


## Building

```bash
fpm build
fpm install                  # optional, see "Installation"
```

This produces the static library `build/<compiler>_<hash>/librmn.a` together
with `libApp.a` and the Fortran modules of librmn (`rmn_gmm`, `mod_msg`, ...).


## Using librmn as an fpm dependency

Add the dependency to your `fpm.toml`:

```toml
[dependencies]
rmn = { git = "https://github.com/ECCC-ASTD-MRD/librmn.git", tag = "v20.0.0" }
# a fork, for instance:
rmn = { git = "https://github.com/wobagi/librmn.git", tag = "v20.0.0" }
# or, for a local checkout:
rmn = { path = "../librmn" }
```

fpm clones the repository once into `build/dependencies/rmn` of the *consuming*
project (a shallow clone, detached `HEAD`, no tags) and reuses that clone
afterwards.

Always use a `tag`, and **bump it for every new librmn release**.  fpm only
fetches again when the `tag` value changes:

* new `tag` in the manifest: fpm fetches it, `tag v20.0.2 -> FETCH_HEAD`, and
  the build uses the new commit;
* unchanged `tag`, or no `tag` at all, with new commits on the branch: fpm
  re-reads its own cached manifest, fetches nothing, and the build silently
  uses the **old** commit.  The same happens with `branch = "..."` instead of
  `tag = "..."`.

A `tag` or `branch` that does not exist in the remote is not reported as such:
fpm creates an empty repository and stops with the confusing message
`fatal: your current branch 'master' does not have any commits yet`.

If the dependency is missing, or if the clone was made when the `tag` did not
exist yet, the clone has to be removed to get a fresh one:

```bash
rm -rf build/dependencies/rmn
```

Note that the `App` and `cmake_rpn` submodules are never needed by fpm: the
fpm build of librmn is self contained, see "The App dependency" below.

and **add the `legacy` compiler options of librmn to your own manifest**:

```toml
[features]
legacy.flags = "-fconvert=big-endian -fcray-pointer -frecord-marker=4 -fno-second-underscore -fallow-argument-mismatch -ffixed-line-length-none -DF_C_STRING_AVAILABLE"
legacy.c-flags = "-std=c99 -include rmn_config.h"
legacy.link-time-flags = "-pthread"

[profiles]
default = ["legacy"]
```

`-include rmn_config.h` works because fpm adds the include directories of
librmn, including `fpm/include`, to the compile command of the whole build.
`-DF_C_STRING_AVAILABLE` is needed as soon as your project uses the module
`f_c_strings_mod`, see "Known limitations".  The gfortran options of the
`legacy` feature of `fpm.toml` are the complete list for that compiler; the
manifest has the equivalent options for ifort, ifx, flang, nvfortran and
nagfor.

A project can then `use` the modules of librmn and call its routines:

```fortran
program example
   use rmn_gmm
   use app
   implicit none
   character(len=256) :: version_string
   interface
      subroutine rmnlib_version(version_string, prnt)
         character(len=*), intent(out) :: version_string
         logical, intent(in) :: prnt
      end subroutine rmnlib_version
   end interface

   call rmnlib_version(version_string, .false.)
   print '(a)', trim(version_string)
end program example
```

### Why the options have to be repeated

fpm keeps a single set of compiler options for the whole build and only takes
it from the *root* package of the project (the one in the current directory).
The options of a dependency, including the ones requested with
`features = [...]`, are **not** applied when fpm compiles that dependency.
librmn cannot be compiled correctly without the options above (it uses fixed
form sources, big endian unformatted files, Cray pointers, ...), so every
project that builds it has to declare them.  This is a limitation of
fpm 0.13.0-alpha, not something the manifest of librmn can work around.

Note that this is the same requirement as with CMake, where these options are
`PUBLIC` link/compile options of the `rmn` target and are therefore inherited
by client applications.


## What is in fpm.toml, and why

| Setting | Reason |
| --- | --- |
| `[build] auto-executables/examples/tests = false` | The library has no fpm targets; the top level `tests/` and `src/interpv/test` are not part of the library. |
| `[library] type = "static"` | The RPN link line of the other RPN libraries is built for static archives.  It also keeps fpm from pruning objects it thinks are unused. |
| `[library] include-dir` | Same include directories as the CMake build: `include`, `src`, `src/PUBLIC_INCLUDES` and `src/PUBLIC_INCLUDES/rmn`, plus `fpm/include`. |
| `[fortran] source-form = "default"`, `implicit-typing`, `implicit-external` | librmn is fixed form legacy code that relies on implicit typing and implicit external interfaces. |
| `[preprocess.cpp] suffixes = ["F90", "F"]` | Makes fpm treat the preprocessed sources as Fortran and adds `-cpp` to their compile command. |
| `[features] legacy` | The RPN compiler options, see above.  They are in the `default` profile so that `fpm build` works out of the box. |
| `[dependencies] App = { path = "fpm/libapp" }` | librmn uses the `app` module and calls `App_*` routines (see below). |

The compiler options of the `legacy` feature mirror the
`GNU_FFLAGS`, `Intel_FFLAGS`, `Flang_FFLAGS` and `PGI_FFLAGS` variables of
`CMakeLists.txt` and the GNU and Flang presets of
`cmake_rpn/modules/ec_compiler_presets`:

| Option | Meaning |
| --- | --- |
| `-fconvert=big-endian`, `-Mbyteswapio`, `-convert big_endian` | unformatted files are written big endian |
| `-frecord-marker=4`, `-assume byterecl` | 4 byte record markers in unformatted files |
| `-fcray-pointer` | hidden arguments for `TYPE(C_PTR)` and character pointers |
| `-fno-second-underscore` | symbol names have a single trailing underscore |
| `-align array32byte` | Intel array alignment of the RPN memory words |
| `-fallow-argument-mismatch`, `-allow argument_mismatch` | legacy call sites pass the wrong type |
| `-ffixed-line-length-none` | fixed form sources are not truncated at 72 columns |
| `-std=c99 -include rmn_config.h` | C99 as with `CMAKE_C_STANDARD 99`, plus the definitions of the CMake compiler presets |
| `-pthread` | matching compile and link options |


## Generated files

The CMake build generates two headers that fpm cannot generate, because it has
no build script.  They are replaced by small static files:

* `fpm/include/rmn_build_info.h` for `include/rmn_build_info.h`.  Only
  `src/primitives/rmnlib_version.F90` uses it, to build its version string.
* `fpm/libapp/include/App_build_info.h` for the header of the App submodule.
  `App/src/App.c` includes it but uses none of its values.

`rmn_config.h` in `fpm/include` is force included into every C source with
`-include`.  It defines `_GNU_SOURCE` and `Little_Endian`, which the CMake
build takes from `add_compile_definitions()` and
`cmake_rpn/modules/ec_compiler_presets`.  `Little_Endian` selects the native
bit field layout of the packed formats (`include/bitPacking.h`); unlike the
CMake presets it is derived from `__BYTE_ORDER__`, so big endian platforms are
handled correctly.

The `WhiteBoard.hf` header of the WhiteBoard library is generated by a `sed`
call in `CMakeLists.txt`.  It is only used by the tests in `tests/`, so it is
not part of the fpm build; a client that needs it should use the CMake build.


## The App dependency

librmn includes the `app` module (63 `use app` statements in 39 files under
`src/`) and calls the `App_*` C routines for logging.  Upstream, `App/` is a
git submodule built by CMake, and it has no `fpm.toml` of its own, so
`fpm/libapp/` provides a minimal fpm package for it:

* `fpm/libapp/fpm.toml`, with the include directory that holds the static
  `App_build_info.h`.
* `fpm/libapp/src/`, which contains copies of the non-MPI sources of the
  submodule: `App.c`, `App.f90`, `str.c` and the headers `App.h`,
  `App_Timer.h` and `str.h`.

`MPMD.c` and `MPMD_Module.F90` are left out: they need MPI, and fpm has no MPI
support.  The CMake build produces them only in the "ompi" flavour
(`-DWITH_OMPI=yes`), so this is the equivalent of a serial build.

The files are **copies, not symlinks**, and they are the only vendored files of
the whole package.  fpm clones its dependencies with `git clone`, which does
not initialise submodules, so symlinks into `App/` would dangle in every build
of a project that depends on librmn (that is, in every build that is not
performed in a submodule aware working copy).  fpm has no submodule support and
no way to declare a manifest for a package inline, hence the copy.  Refresh it
with

```bash
cp App/src/{App.c,App.f90,App.h,App_Timer.h,str.c,str.h} fpm/libapp/src/
```

after `git submodule update`.  The submodule itself is not modified.  The
licence of the copied files is the licence of the submodule,
`LGPL-2.1-or-later`, which is the licence of librmn itself.


## Legacy test programs

`src/fstd98/tests`, `src/packers/tests`, `src/primitives/tests` and
`src/readlx/tests` contain standalone test and demonstration programs.  The
CMake build never compiled them (its globs only look at `src/*/*`), and they do
not compile as part of the library either, so they were moved to
`tests/legacy/<component>/`.  Nothing else references them.

`src/base/unused` is compiled by both build systems and was left alone.


## Installation

`fpm install` copies the archives to `~/.local/lib` and the Fortran modules to
`~/.local/include`.  C headers are **not** installed by fpm, and the installed
modules are not usable as a dependency by themselves.  Use a `git` or `path`
dependency instead, as described above.


## Known limitations

* The compiler options of a dependency are ignored by fpm 0.13.0-alpha, see
  "Why the options have to be repeated".
* fpm has no pre-build script and no way to exclude files, so anything under
  `src/` is part of the library.
* fpm only compiles the module sources of a dependency that are reachable from
  the project (`f_c_strings_mod` and `serializer_jar_mod` are left out unless
  they are used), and it links the objects of a dependency directly into the
  executable instead of building an archive.  Both are harmless, but do not
  expect a complete `librmn.a` in the build directory of a client project.
* `-DF_C_STRING_AVAILABLE` is hard coded for gfortran.  `CMakeLists.txt` uses
  `check_source_compiles()` to find out whether the compiler provides
  `f_c_string` in `iso_c_binding`; gfortran 15 and later do, and
  `src/primitives/f_c_strings_mod.F90` then has to use it.  Compilers without
  the intrinsic simply do not export `f_c_string` from that module; no other
  librmn source uses it, and a client has to compile that module with the same
  definition.
* Absolute paths in a `path` dependency do not work with fpm 0.13.0-alpha
  (`Model error: './././//path/to/librmn/fpm.toml' could not be found`).  Use a
  relative path.
* `features = ["legacy"]` on the librmn dependency has no effect, and asking for
  it in the `default` profile of librmn does not change the options used for a
  dependency either.
* fpm only knows the compiler names of its own table, and matches them as
  substrings, so a `pgfortran` feature is applied to gfortran builds.  The
  manifest therefore uses the `nvfortran` key for the PGI/NVHPC options.
* MPI (`-DWITH_OMPI=yes`) and the "ompi" flavour of App are not available.
