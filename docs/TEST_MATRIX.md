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
| Twenty close cycles, measured latency/performance, cache stress | Not established |

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
