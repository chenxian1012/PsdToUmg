# Files removed from upstream MolecularMatters/psd_sdk

Snapshot from https://github.com/MolecularMatters/psd_sdk at commit 6132139.

We vendor the full `src/Psd/` directory and remove only platform-specific
files that don't compile on Win64 (the only supported PSD2UMG platform).

## Removed translation units

- `PsdNativeFile_Linux.cpp` / `PsdNativeFile_Linux.h` — Linux-only (uses POSIX `aio.h`).
- `PsdNativeFile_Mac.h` / `PsdNativeFile_Mac.mm` — Mac-only (Objective-C++, requires `<Foundation/Foundation.h>`). The `.mm` was never copied; the `.h` was deleted.
- `src/Psd/CMakeLists.txt`, `src/Samples/`, `samples/`, `bin/`, top-level `CMakeLists.txt`, `build/` — not vendored. UE Build.cs replaces upstream CMake build system.

## Build adjustments (no upstream source edits)

- `Build.cs` adds both `Includes/Psd` and `Source/Psd` to `PrivateIncludePaths` so upstream `.cpp` files can resolve flat includes like `#include "PsdPch.h"` (upstream layout had `.cpp` and `.h` co-located in `src/Psd/`).
- Upstream `PsdPlatform.h` already `#define NOMINMAX` before including `<windows.h>`, so we do NOT add `NOMINMAX=1` via `PublicDefinitions` — it triggers `-Wmacro-redefined` under Clang in UE.

## Notes
- No third-party libraries required; psd_sdk has its own miniz, no STL/RTTI/exceptions.
- 8/16/32 bit PSDs supported. Smart Object pixel extraction is NOT supported
  (only smart-object layers are detected as a layer type). PSD2UMG uses the
  `#linkedpsd(...)` naming convention instead — see spec section 4 and section 5.
- PSB (>2GB) is not supported. PsdReader detects and reports an error.
