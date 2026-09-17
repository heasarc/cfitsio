# Using JPEG-LS-enabled CFITSIO in an existing C project

This guide is for someone who already has a C project that links against
stock CFITSIO and wants to read FITS files that were compressed with
JPEG-LS (`ZCMPTYPE = 'JPEGLS_1'`). It walks through building the
JPEG-LS-enabled fork and swapping it into an existing build, on macOS.

You will **not** need to change any of your existing C source code.
`fits_open_file`, `fits_read_img`, `fits_read_pix`, etc. all decompress
JPEG-LS tiles automatically, the same way they already handle RICE or
GZIP. The only change is *which* CFITSIO library your project links
against.

## Two ways to do this

- **Option A — repoint your build files.** Build the fork somewhere on
  disk and update your project's `-I`/`-L` flags to point at it.
  Doesn't touch anything outside your project; easy to undo. Requires
  a small build-file edit.
- **Option B — install in place over your existing CFITSIO.** Build the
  fork with the same install prefix your current CFITSIO uses, and
  `make install` overwrites it. Your project's build files need only
  the CharLS link flags added. Simpler day-to-day, but it mutates a
  shared system location, so read the caveats before choosing it.

Steps 1–3 (get the code, build CharLS, build CFITSIO) are the same for
both options. They diverge at Step 4.

## Prerequisites

- macOS with Xcode command line tools (`xcode-select --install`)
- `cmake` (`brew install cmake` if you don't have it)
- `git`

## Step 1: Get the code

The fork's one dependency, CharLS (the JPEG-LS codec library), is
included as a git submodule, so a recursive clone fetches both. Clone
wherever you like; this guide uses `~/dev`:

```bash
mkdir -p ~/dev && cd ~/dev
git clone --recursive https://github.com/rithwiksud/cfitsio_advanced_compression.git
cd cfitsio_advanced_compression
```

Already cloned without `--recursive`? From inside the repo, run:

```bash
git submodule update --init
```

Either way you should now have CharLS inside the repo:

```bash
ls charls/include/charls/charls.h
```

## Step 2: Build CharLS

CharLS is a small, self-contained library. From the repo root, build it
as a static library:

```bash
cmake -S charls -B charls/build -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=OFF -DCHARLS_BUILD_TESTS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build charls/build -j
```

When this finishes you should have `charls/build/libcharls.a`:

```bash
ls charls/build/libcharls.a
```

## Step 3: Build the JPEG-LS-enabled CFITSIO

Still in the repo root. The Makefile detects `charls/` automatically and
adds the CharLS include/link flags itself, so no extra configure
arguments are needed:

```bash
./configure
make -j
```

**If `make` dies on `testf77` with `ld: library 'curl' not found`**, add
`--without-fortran` and re-run:

```bash
./configure --without-fortran
make -j
```

This happens on macOS when `gfortran` comes from Homebrew GCC: it does not
search the macOS SDK where `libcurl.tbd` lives, while clang does. Only the
Fortran *test program* fails — the C library, `fpack` and `funpack` are all
built fine — but the failure stops the build, and it stops `make install`
from installing anything at all. Nothing in JPEG-LS needs Fortran.

This produces the library your project needs, along with `fpack`/`funpack`
command-line tools:

```
~/dev/cfitsio_advanced_compression/.libs/libcfitsio.a       (static)
~/dev/cfitsio_advanced_compression/.libs/libcfitsio.dylib   (shared, if built)
~/dev/cfitsio_advanced_compression/fitsio.h                 (same API as stock CFITSIO)
```

Sanity-check that JPEG-LS support actually compiled in, using any FITS
image you have:

```bash
./fpack -j test.fits    # -j = compress with JPEG-LS
```

If that produces `test.fits.fz` without errors, the build is good. (If
CharLS was missing at build time, CFITSIO still builds, but `-j` fails at
run time.)

## Step 4A: Option A — point your project at the new library

In your existing project's build (Makefile, CMakeLists.txt, Xcode
project settings, etc.), find wherever it currently references CFITSIO
and change **both** the include path and the library path:

**Before:**
```
-I/path/to/old/cfitsio
-L/path/to/old/cfitsio -lcfitsio
```

**After:**
```
-I$HOME/dev/cfitsio_advanced_compression
-L$HOME/dev/cfitsio_advanced_compression/.libs -lcfitsio
```

You also need to link CharLS on the final link line, since the fork's
`libcfitsio` calls into it. CharLS is written in C++, so the C++ runtime
must be linked too, after `-lcharls` (`-lc++` on macOS, `-lstdc++` on
Linux):

```
-L$HOME/dev/cfitsio_advanced_compression/charls/build -lcharls -lc++
```

**Linking is not enough: the program also has to *find* `libcfitsio` at run
time.** `make` builds a shared library in `.libs/`, but the path recorded in
your program points at the *install* directory (`<prefix>/lib`), which does
not exist unless you ran `make install`. So a program linked as above can
build cleanly and then fail to start:

```
# Linux
libcfitsio.so.10: cannot open shared object file: No such file or directory
# macOS
dyld: Library not loaded: <prefix>/lib/libcfitsio.10.dylib
```

Pick one of these two fixes.

**Recommended: link CFITSIO statically.** Name the static archive directly
instead of using `-lcfitsio`. There is no shared library to locate at run
time, so there is no rpath and no `LD_LIBRARY_PATH` to manage, and CharLS is
already a static archive anyway:

```bash
gcc myprogram.c \
  -I$HOME/dev/cfitsio_advanced_compression \
  $HOME/dev/cfitsio_advanced_compression/.libs/libcfitsio.a \
  -L$HOME/dev/cfitsio_advanced_compression/charls/build -lcharls -lc++ \
  -lm -lcurl -lz -o myprogram
```

(`-lcurl` and `-lz` are needed here because a static CFITSIO no longer
carries its own dependencies. Drop `-lcurl` if your CFITSIO was configured
without curl.)

**On Linux, the alternative is `-Wl,-rpath`**, which records where to look
so no environment variable is needed:

```bash
gcc myprogram.c \
  -I$HOME/dev/cfitsio_advanced_compression \
  -L$HOME/dev/cfitsio_advanced_compression/.libs -lcfitsio \
  -Wl,-rpath,$HOME/dev/cfitsio_advanced_compression/.libs \
  -L$HOME/dev/cfitsio_advanced_compression/charls/build -lcharls -lstdc++ \
  -lm -o myprogram
```

**This does not help on macOS**, and it is worth knowing why. Libtool gives
`libcfitsio.dylib` an install_name that is the *absolute install path*
(`<prefix>/lib/libcfitsio.10.dylib`), not `@rpath/libcfitsio.10.dylib`.
Check it yourself with `otool -D .libs/libcfitsio.10.dylib`. dyld follows
that absolute path and never consults the rpath, so adding `-Wl,-rpath`
changes nothing: the program still looks in `<prefix>/lib`. Until you run
`make install`, that path does not exist and the program aborts at startup.

So on macOS, either link statically (above), or run `make install` first and
link against `<prefix>/lib` as in Option B — after which no rpath is needed
at all, because the install_name already points there.

Setting `DYLD_LIBRARY_PATH` (Linux: `LD_LIBRARY_PATH`) on each invocation
also works, but it has to be re-set by every user, script and service that
runs the program, and it changes library resolution for the whole process,
so prefer static linking or a real install for anything you intend to keep.

If your project uses a Makefile with a variable like `CFITSIO_DIR`,
just repoint that variable at `~/dev/cfitsio_advanced_compression`
and add the CharLS flags alongside it.

Skip to Step 5 if you're using this option.

## Step 4B: Option B — install in place over your existing CFITSIO

If your project just does `#include <fitsio.h>` and links `-lcfitsio`
with no hardcoded path to a specific CFITSIO checkout, you can instead
overwrite your existing install with the JPEG-LS-enabled build.

**1. Find where your current CFITSIO is installed.**

```bash
# if installed via Homebrew
brew --prefix cfitsio
# otherwise, find it directly
find /usr /opt /usr/local -name "fitsio.h" 2>/dev/null
```

This gives you a prefix, e.g. `/opt/homebrew` or `/usr/local`. Call it
`<prefix>` below.

**2. Install CharLS and CFITSIO to that prefix.**

From the repo root (after Steps 1–2):

```bash
# CharLS, installed to the same prefix
cmake --install charls/build --prefix <prefix>

# CFITSIO fork, rebuilt and installed to the same prefix
# (add --without-fortran if make dies on testf77; see Step 3)
make clean
./configure --prefix=<prefix>
make -j
make install
```

**The `make clean` is not optional on macOS.** If you already built the
fork once (Step 3, without `--prefix`), the object files are unchanged, so
`make` will not relink `libcfitsio.dylib` — and `make install` then copies a
library whose install_name still points at the *old* location. Everything
that links it, including the newly installed `fpack`, aborts at startup with
`Library not loaded:` pointing at a path in your source tree. `make clean`
forces the relink and the install_name comes out correct. Verify with:

```bash
otool -D <prefix>/lib/libcfitsio.10.dylib   # should print <prefix>/lib/...
```

`make install` overwrites `fitsio.h` and `libcfitsio.*` at `<prefix>`
with the JPEG-LS-enabled versions. Any project that already builds
against `-I<prefix>/include -L<prefix>/lib -lcfitsio` picks up the new
library.

**3. Usually you don't have to change anything else.**

If your project links the **shared** library (the default `-lcfitsio`), no
build-file change is needed at all: CharLS is a static archive that gets
absorbed into `libcfitsio.dylib`/`.so` when the fork is built, so there are
no leftover CharLS symbols for your program to resolve. This was verified on
macOS with a project whose Makefile was left completely untouched — it
picked up JPEG-LS support purely from the reinstalled library.

You only need to add `-lcharls -lc++` (macOS) / `-lcharls -lstdc++` (Linux)
if you link CFITSIO **statically**, where those symbols are still
unresolved. `libcharls.a` is installed into `<prefix>/lib` alongside
`libcfitsio`, so no new `-L` path is needed:

```
-lcfitsio -lcharls -lc++
```

Adding the flags when linking shared is harmless, so if you are unsure,
add them.

**Caveats before choosing this option:**

- **Homebrew will fight you.** If `<prefix>` is Homebrew-managed
  (`/opt/homebrew` or `/usr/local`), a later `brew upgrade` or
  `brew reinstall cfitsio` will silently overwrite your JPEG-LS build
  back to stock CFITSIO. Homebrew has no idea you replaced its files.
  Either avoid touching a brewed prefix, or be ready to re-run this
  install after any brew upgrade.
- **Architecture must match** (arm64 vs. x86_64) — same as swapping any
  native library in place.
- **It's shared, system-wide state.** Any other project on the machine
  linking against that same `<prefix>` now gets the JPEG-LS build too.
  Usually harmless (it's a superset of stock CFITSIO's behavior), but
  worth knowing.
- **Harder to undo** than Option A — reverting means reinstalling stock
  CFITSIO over it (`brew reinstall cfitsio`, or rebuilding upstream
  CFITSIO with the same prefix).

If none of that is a concern, this is the simpler day-to-day setup.

## Step 5: Verify it works

Run your program against a JPEG-LS-compressed FITS file (one produced
by `fpack -j`, or by Astropy using the `JPEGLS` codec). Your existing
read calls should just work — no source changes needed.

If you don't have a test file handy, make one:

```bash
~/dev/cfitsio_advanced_compression/fpack -j some_image.fits
# produces some_image.fits.fz — hand this to your program
```

## Can I just hand someone a prebuilt `libcfitsio` dylib?

Tempting — it is what you would do for embedded firmware — but a macOS
dynamic library is much less portable than a static firmware image, and a
plain "drop this file in" hand-off is fragile. Facts from a library built
on this machine (`otool`/`lipo` output):

- **Architecture.** It came out `arm64` only. On an Intel Mac it will not
  load at all. Distributing one file for everyone means a universal binary:
  build with `-arch arm64 -arch x86_64`, or `lipo -create` two builds.
- **install_name.** The library records the absolute path it expects to live
  at (`otool -D`). Drop it somewhere else and every program that links it
  aborts with `Library not loaded:`. A distributable library must be built
  with an install_name of `@rpath/libcfitsio.10.dylib` (the CMake build does
  this; the autotools build does not), or patched afterwards with
  `install_name_tool -id`.
- **Minimum OS.** The build stamps `minos` from the SDK it was built
  against. A library built on macOS 26 will be refused by older systems.
  Build with `-mmacosx-version-min=` set to the oldest macOS you support.
- **Gatekeeper.** A locally built library is only ad-hoc ("linker") signed.
  Once it is downloaded from the internet it carries a quarantine attribute
  and may be refused. Real distribution means signing with a Developer ID
  and notarizing, or telling users to run `xattr -d com.apple.quarantine`.
- **The header.** Shipping only the library gives existing programs the
  ability to *read* JPEG-LS files, which needs no API change. Calling
  `fits_set_jpegls_maxerr()` needs the matching `fitsio.h`, so ship both.
- **Dependencies.** This build needs only `libSystem`, `libcurl`, `libz`
  and `libc++`, all of which ship with macOS — so no third-party runtime
  needs to travel with it. That part is genuinely portable. (If you build
  against Homebrew's curl or zlib instead, it stops being portable.)

If you want a true single-file hand-off, prefer the **static** library
(`libcfitsio.a`, built universal): no install_name, no rpath, no dyld, and
CharLS is already inside it. The trade is that users relink rather than
swap a file.

## Common pitfalls

- **`fatal: Remote branch 3.0.0 not found`** — an older version of this
  guide cloned CharLS separately with `--branch 3.0.0`, a tag that
  doesn't exist. Use the submodule as in Step 1 instead.
- **`charls/` is empty** — the fork was cloned without `--recursive`.
  Run `git submodule update --init` from the repo root.
- **"Undefined symbols for architecture..." at link time** — you're
  missing `-lcharls`, the `-L` path to `charls/build`, or the C++
  runtime (`-lc++`, which must come after `-lcharls`). The fork's
  `libcfitsio` depends on CharLS; all must be linked.
- **`make` fails with `ld: library 'curl' not found` on `testf77`** — a
  Homebrew `gfortran` that can't see the macOS SDK's libcurl. Re-run
  `./configure --without-fortran`. Note this also silently prevents
  `make install` from installing anything, even with `make -k`.
- **Builds fine, but fails to start**: `libcfitsio.so.10: cannot open
  shared object file` (Linux) or `dyld: Library not loaded:
  <prefix>/lib/libcfitsio.10.dylib` (macOS) — the program can't find the
  shared library at run time, because the recorded path is the *install*
  directory and you haven't run `make install`. On macOS `-Wl,-rpath`
  will not fix this (see Step 4A); link statically or install first.
- **Program still fails to read JPEG-LS tiles / says "unknown
  compression type"** — your project is probably still picking up a
  system-installed CFITSIO (e.g. from Homebrew, `/usr/local/lib`, or
  `/opt/homebrew/lib`). Check `otool -L myprogram` after building — if
  it lists a `libcfitsio` path that isn't the fork's `.libs` directory,
  fix your `-L` order (put the fork's path first) or use static linking
  to avoid ambiguity.
- **Different machine/OS** — on Linux the steps are identical, except
  link `-lstdc++` instead of `-lc++`.
