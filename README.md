# lib-lvgl — LVGL 9 for GEM

Initial bring-up component for A2560Me. This repository currently supplies pinned
LVGL/GEMlib, a software-renderer static library, a standalone monochrome GEM raster
probe, pixel/geometry primitives, host tests and a target byte diagnostic.
It also supplies the unmodified upstream Widgets demo with a fixed-size GEM
window adapter and a Linux/SDL colour preview. See [demo instructions](docs/DEMO.md).
The supplied implementation plan is the roadmap; its hardware acceptance gates
remain open. No board or emulator results are claimed.

## Build

```sh
git clone --recursive https://github.com/FoenixLabs/freemint-lib-lvgl.git
cd freemint-lib-lvgl
sh scripts/bootstrap.sh
make test
make probe bytes MINTLIB=/absolute/path/to/built/mintlib
make inspect
make demo MINTLIB=/absolute/path/to/built/mintlib
```

Requires Python 3.12+, make, host C compiler with ASan/UBSan, and the recorded
`m68k-atari-mint` toolchain. MiNTlib must provide `include/`,
`build/m68020/crt0.o` and `build/m68020/libc.a`. In a2560me-system use
`MINTLIB="$PWD/../mintlib"` from this component. See `toolchain.lock` for the
installation actually checked. No toolchain or dependencies are fetched by builds.
GEMlib is exported to ignored build storage and built with its upstream rules;
third_party remains unmodified. Binary outputs are not committed.

Default `PROFILE=mint060` outputs `build/mint060/LVPROBE.APP`, `LVBYTES.TTP` and
`liblvgl.a`. `PROFILE=emutos060` has a separate output directory with the same
candidate ABI; launch under EmuTOS remains an explicit acceptance gate.
The LVGL configuration derives from the pinned 9.2.2 template: software draw,
one draw unit, big endian, no OS, float or matrix support, reduced widgets.

## Run the first gate

Copy LVPROBE.APP to a writable target directory and launch from GEM in a 1 bpp
mode. It writes LVPROBE.TXT, shows a 319×193 checkerboard, and redraws saved pixels
on exposure. Move and cover/uncover it, including while idle. Close via the closer
or Escape. Other depths display an unsupported-mode alert. Run LVBYTES.TTP in the
same directory and inspect LVBYTES.TXT. See docs/TEST_MATRIX.md for exact evidence.

Do not treat a cross-link, byte test, or hardware pixel specification as proof of
NVDI raster support. Colour modes remain disabled until EdDI discovery and the
native MFDB contract are demonstrated. The app must use AES/VDI only; never write
framebuffer/DMA registers, enter supervisor mode, or change caches or boot flow.

## Status and next work

Stages 0–2 have build/test foundations; stage 1 target validation is pending.
See docs/DRIVER_COMPATIBILITY.md for blockers and docs/TEST_MATRIX.md for coverage.
The demo now has a basic reusable fixed-size window/display/input adapter.
Next validate its mono-to-colour VDI path on target, implement native-colour probes,
and complete resize/lifecycle robustness and board acceptance.
