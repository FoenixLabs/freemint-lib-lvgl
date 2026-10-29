# Acceptance record — 2026-10-29

| Check | Result |
|---|---|
| Exact dependency SHAs and clean upstream files | Passed |
| Target ABI and MFDB assertions | Cross-compiled |
| Geometry, odd-width mono padding/masks, all RGB565 values | Host ASan/UBSan passed |
| Native format gate, RGB555/24/indexed/swapped rejection | Host passed |
| Eight-bit native32 and RGB565 explicit conversion | Host passed |
| Portable colour fills: all opacity/mask combinations, channels, padded rows | Host ASan/UBSan passed |
| Demo rendering, pointer tabs, physical text entry, viewport, no virtual keyboard, teardown | Host ASan/UBSan/leak check passed |
| GEMlib/LVGL/demo MiNT060 build and linked FPU inspection | Passed |
| A2560Me FreeMiNT/fVDI 1280×1024, packed32 independent MFDB probe | Passed: primaries, black/white, channel ramps, odd width |
| A2560Me native32 Widgets launch and colour fills | Passed visually after big-endian blend-hook correction |
| A2560Me physical text entry, full viewport, no virtual keyboard | Passed: A2560Me entered through USB keyboard HID |
| A2560Me final native32 Analytics navigation and colour gradient | Passed |
| A2560Me demo move/overlap/exposure and Escape close | Passed with earlier mono-on-colour build; native32 probe closes cleanly |
| Real depth1 EmuTOS or MiNT mode | Untested |
| Native RGB565 screen mode | Implemented; target untested |
| NVDI runtime | Untested |
| Resize/full-size/iconification/failure injection | Not implemented |
| Forced Profile scroll throughput, native32 | 24 frames:25.770s baseline →9.625s optimized; see below |
| Twenty close cycles, HID latency, cache stress | Not established |

## Target test method

Build `make probe demo MINTLIB=/absolute/path/to/built/mintlib`. Upload the two
executables to D:/APPS/LVGL after closing old instances. Verify SHA256 by FTP
readback. Use the RM10 skill for live HDMI screenshots and USB HID. Launch from
TeraDesk's Open Item dialog; verify the running /kern process executable path.
Run the independent colour probe before the Widgets demo in a new mode/driver.
Retain LVPROBE.TXT and captures outside Git. No driver, firmware, boot configuration,
cache setting or personal desktop setting is changed by this test.

This session established FreeMiNT 1.19 (October 17 build), 68060/no FPU, fVDI/EdDI
1.00 and packed32 RGB8 maps. The exact FPGA/driver revision and cache policy were
not identified. Serial capture on the established COM3/data TCP2002 endpoint was
refused; evidence uses HDMI/HID, target /kern reports and verified file readback.

Evidence is in the ignored system logs/rm10/20261029T132739Z-lvgl-widgets run.
Earlier corrupt colour-probe and blend outputs are retained alongside successful
retests. See DRIVER_COMPATIBILITY.md for the corrected MFDB stride and draw-blend
issues. Early and settled screenshots are distinct: snapshot HTTP success alone
is not a frame-freshness or latency measurement.

## Remaining acceptance

Repeat launch, input, cover/uncover while idle/updating, move partly offscreen,
resize/lifecycle and failure tests in each required driver/depth. Record configured
cache policy and run load/stress tests. The current successful native32 sample
is not full plan acceptance. See DEMO.md for launch commands.

## Scrolling performance follow-up

On the same A2560Me native32 mode, F8 renders 24 Profile scroll frames (8-pixel
steps up to96 and back) with no changes to the demo's visual effects:

| Cumulative change | Total ms | Conversion ms | Presentation ms |
|---|---:|---:|---:|
| Original renderer plus instrumentation | 25770 | 13055 | 70 |
| Row conversion and damage bounds | 15725 | 3090 | 50 |
| Compile demo/LVGL with O2 | 14410 | 3060 | 45 |
| Reuse opaque fill rows | 11660 | 3070 | 65 |
| Increase draw strip32→100 rows | 9625 | 3070 | 65 |

Every run converts5,112,000 pixels. Original presentation copied5,760,000 pixels;
damage bounds reduce that to5,112,000. A repeat of the final build took9670ms (conversion3095ms, presentation40ms).
Overall throughput is about2.7× baseline,
still only about2.5 forced scroll frames/s. Conversion remains about32% of total;
software drawing dominates the rest. A native-format renderer could remove the
conversion pass but would not alone deliver smooth scrolling.

Timings are elapsed target clock values at5ms resolution; phase accumulation is
approximate. This is a bounded synchronous rendering test, not HID latency or
normal animation FPS. Evidence, hashes and build logs are in the ignored system
logs/rm10/20261029-lvgl-scroll directory. ASan/UBSan span tests cover zero/odd widths,
unaligned buffers,16/32-bit output and untouched trailing bytes; opaque fill tests
cover padded multirow destinations. Headless demo smoke and linked FPU scans pass.

Final optimized build also passed RM10 mouse-drag scrolling, physical A2560Me
text entry without a virtual keyboard, and desktop-dialog cover/uncover with
typed text retained. The demo is left open on Profile. Broader scripts/check.py
source checks passed but its baseline staging verification failed with existing
staging mismatches; no baseline artifacts were rebuilt or replaced in this task.
