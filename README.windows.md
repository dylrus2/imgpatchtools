# IMG Patch Tools — Windows Build

Build instructions and notes for building `imgpatchtools` on Windows with MSYS2 / MinGW-w64.

See [README.md](README.md) for what the tools do and how to invoke them.

## Prerequisites

Install [MSYS2](https://www.msys2.org/), then from the MSYS2 shell:

```sh
pacman -S \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-zlib \
    mingw-w64-x86_64-bzip2 \
    mingw-w64-x86_64-pkgconf
```

A separate `make` (e.g. ezwinports) on `PATH` will also work, as will the MSYS2 shell `make`.

## Build

From a Git Bash, MSYS2, or compatible shell in the repository root:

```sh
PATH="/c/msys64/mingw64/bin:$PATH" make
```

The `PATH` prefix is **required**. Without it, `cc1plus.exe` may load a conflicting runtime DLL from elsewhere on `PATH` and exit silently — make will report `Error 1` with no compiler diagnostics.

Adjust the path if you installed MSYS2 somewhere other than `C:\msys64`.

Build outputs:

```
bin/ApplyPatch.exe
bin/BlockImageUpdate.exe
bin/BlockImageVerify.exe
bin/imgdiff.exe
bin/scriptpatcher.sh
```

## Running

Usage is the same as the Linux/macOS build — see [README.md](README.md). Example:

```powershell
./bin/ApplyPatch.exe boot.img - <tgt_sha1> 33554432 <init_sha1> boot.img.p
```

The tools run under PowerShell, `cmd.exe`, Git Bash, or MSYS2.

## Windows-specific changes

These build under `__MINGW32__` guards and have no effect on Linux/macOS:

| Area | Behavior on Windows |
|---|---|
| `err()` / `errx()` | Polyfilled in [applypatch/include/err.h](applypatch/include/err.h) |
| `fsync()` | Aliased to `_commit()` in [otafault/ota_io.cpp](otafault/ota_io.cpp) |
| `O_BINARY` | OR'd into all `ota_open()` calls so binary I/O isn't mangled by CRLF translation |
| Patch file reads | Opened with `"rb"` instead of `"r"` in [ApplyPatch.cpp](ApplyPatch.cpp) |
| `chown()` | No-op stub (Windows has no POSIX UID/GID) |
| `O_SYNC` | Defined to `0` (no-op flag) |
| `mkdir(path, mode)` | Calls `mkdir(path)` — file modes don't apply to NTFS |
| BLKDISCARD / eMMC TRIM | Skipped (`SUPPRESS_EMMC_WIPE` is auto-defined) |
| Directory `fsync()` | Skipped (no NTFS equivalent) |
| `FreeSpaceForFile()` | Compiled out (uses `<sys/statfs.h>`) — free-space checks are skipped |
| `<sys/wait.h>`, `<sys/ioctl.h>` | Not included (unused under the relevant guards) |

## Known limitations

These tools were designed to run on an Android device or a Linux host doing OTA work. On Windows they're useful for:

- **Generating** patches with `imgdiff` from one image to another
- **Applying** patches to standalone files with `ApplyPatch` (e.g. patching a `boot.img` extracted from an OTA)
- **Validating** block-image OTAs offline with `BlockImageVerify`

They are **not** suitable for running against a live block device or partition on Windows — code paths that touch raw block devices, `chown`, `BLKDISCARD`, free-space checks, and directory `fsync()` are stubbed out. Use the Linux build for any in-place partition work.

## Troubleshooting

**`make` exits with `Error 1` and no compiler output**
The `PATH` problem above. Re-run with `PATH="/c/msys64/mingw64/bin:$PATH" make`.

**`bz error -4` when applying a patch**
Symptom of a binary file opened in text mode. All known sites are fixed; if you see this on a new code path, find the `fopen(..., "r")` or bare `open(..., O_RDONLY)` and add `"rb"` / `O_BINARY`.

**`ApplyPatch.exe` crashes on multiple patches**
[ApplyPatch.cpp:64](ApplyPatch.cpp#L64) pushes a pointer to a stack-local `Value` into a vector inside the loop. Single-patch invocations work; multi-patch invocations have undefined behavior. This is a pre-existing upstream bug, not Windows-specific.
