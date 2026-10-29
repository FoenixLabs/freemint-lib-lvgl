# lib-lvgl — LVGL 9 for GEM

Initial bring-up component for A2560Me. This repository currently supplies pinned
LVGL/GEMlib, a software-renderer static library, a standalone GEM raster
probe, pixel/geometry primitives, host tests and a target byte diagnostic.
It also supplies the unmodified upstream Widgets demo with a fixed-size GEM
window adapter and physical-keyboard-only input. See [demo instructions](docs/DEMO.md).
The supplied implementation plan is the roadmap; its hardware acceptance gates
remain open. A2560Me FreeMiNT/fVDI 32-bit raster results are recorded in the compatibility notes;
other drivers and modes remain open.

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

Copy LVPROBE.APP to a writable target directory and launch from GEM. It writes
LVPROBE.TXT and shows 319×193 RGB bands/ramps in validated native colour formats,
or a checkerboard at depth 1. Unknown layouts fail with an alert. Check movement
and cover/uncover while idle. Close via the closer or Escape. Run LVWIDGET.APP
for the upstream demo, with physical text entry and no virtual keyboard.

Do not treat a cross-link, byte test, or hardware pixel specification as proof of
NVDI raster support. Native colour requires validated EdDI channel maps; 32-bit fVDI has passed the
independent target raster probe. The app must use AES/VDI only; never write
framebuffer/DMA registers, enter supervisor mode, or change caches or boot flow.

## Status and next work

Initial native 32-bit target validation is underway; full matrix acceptance remains open.
See docs/DRIVER_COMPATIBILITY.md for blockers and docs/TEST_MATRIX.md for coverage.
The demo now has a basic reusable fixed-size window/display/input adapter, native32
rendering into its owned backing buffer and conservative Profile scroll caching.
The A2560Me forced-scroll test improved from9.625s to2.930s for24 frames;
see docs/DEMO.md for controls, measurements and limitations.
The native 32-bit raster probe now passes on A2560Me. Complete other-mode/driver tests,
resize/lifecycle robustness and measured performance before full acceptance.
