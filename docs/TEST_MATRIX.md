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
| Forced Profile scroll throughput, native32 | 24 frames:25.770s original →9.625s previous →2.930s native/cache; see below |
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

## Wheel events follow-up

Headless ASan/UBSan tests cover WM_WHEEL direction/count decoding, hover without
prior click, both axes, zero/invalid packets, outside points, boundaries, nested
scroll selection and chain-disabled behavior. Existing keyboard/no-virtual-keyboard
and upstream Widgets smoke tests also pass. Target build and FPU inspection pass.

On A2560Me, AES_WINDOW reported0x7ff3 and WF_WHEEL readback confirmed enabled1,
mode0 (WHEEL_MESAG). A small diagnostic GEM sender used appl_write to deliver a
WM_WHEEL message to the actual LVWIDGET app/window at client coordinates inside
Profile. Down3 moved the content96 pixels; signed up3 restored the original view. This confirms the real AES-to-LVGL
message path, not physical wheel delivery. RM10 HID wheel attempts in both
directions produced no visible scroll; the repository's three-byte A2560 EmuTOS
mouse forwarding path suggests missing platform wheel transport. Firmware/input
bridge investigation is still needed for end-to-end wheel acceptance.

Evidence and diagnostic sender source are in ignored system
logs/rm10/20261029-lvgl-wheel. Serial2002 remained refused; no service, firmware,
boot or desktop settings changed. The demo is left running in native32 mode.

## Native rendering and Profile cache follow-up

Native32 now renders FF/R/G/B directly into the owned 972,800-byte MFDB, with
no separate draw strip and no final conversion. Original assets and intermediate
layers retain their upstream byte layout. Host ASan/UBSan tests compare fills and
images against the original renderer across opacities, masks, RGB888/XRGB8888/
ARGB8888 and L8 fallback, padding, unaligned destinations and guard bytes.
Initial native and reference smoke-test images match byte for byte. All 24 cached
scroll frames match forced full redraws across the complete padded allocation;
extra invalidation and a top-layer popup correctly prevent cache reuse.

Target F8 results: full-window RGB888 8870ms; native32 6760ms; native32 with
constant Profile scrollbar but cache disabled 4975ms; cache enabled 2930ms;
optional reduced effects 2815ms. The default retains shadows and style effects,
except Profile's scrollbar appearance stays constant to avoid full-subtree
restyling at scroll boundaries. Cache hits were 24/24 enabled and 0/24 disabled.
Conversion pixel count is zero. Copy volume remains5,112,000 pixels, clipped by
AES; measured presentation was25–60ms. The conversion-time field may include a
5ms tick of flush bookkeeping even though no pixels are converted.

Default cached rendering measured solid fills380ms, masked fills210ms and
images5ms; profiling buckets have only5ms clock resolution. Treat these as rough
cost indicators. Optional reduced effects remains off by default because its
additional gain was small. F6/F7/F8 controls and the cache's conservative
Profile-only scope are described in DEMO.md. This is about3.3× the previous
100-row version on the forced scroll test, not a measured HID frame rate.

Component tests, probe cross-build, both headless smoke paths and compiler/linked
FPU inspection passed. The 325 compiler assembly files and129 external runtime
text ranges contained no FPU instructions. Source dependencies remain unchanged.
Evidence and verified deployment hashes are in ignored system
logs/rm10/20261029-lvgl-render. SerialCOM3/data TCP2002 refused; no serial capture
was active. Native16 direct rendering and target16/mono validation remain open.
Physical wheel investigation remains paused at the user's request.

The final MiNT binary repeated2930ms with24 cache hits and default effects. RM10
checks passed mouse-drag scrolling, physical A2560Me text entry with no virtual
keyboard, desktop-dialog cover/uncover with text retained, and Analytics gradient
navigation. The demo was left running on Profile. System source-contract checks
passed; baseline staging verification still reports the existing staging
mismatches. No baseline rebuild was performed.

## Application helper and platform abstraction (historical multiwindow experiment)

The `lvgl_app.h` API now owns initialization, the default surface, multiple LVGL
windows and the event loop. GEM is implemented; MCP is an empty backend that
returns a clear initialization error without accessing hardware. There is no
NetBSD/wsdisplay application backend yet. See APPLICATIONS.md and examples/.

ASan/UBSan API tests passed initialization failure/retry, singleton rejection,
window geometry, active-window routing from child widgets, unhandled/refused and
recursive close requests, callback destruction, direct LVGL object deletion,
last-window survival, explicit quit and cleanup/reinitialization. MCP stub tests
passed. Existing native-render/cache/keyboard Widgets smoke checks also passed.
`make test`, `make probe`, and the examples/demo cross-build/FPU scans passed.

A2560Me RM10 checks established that Escape leaves LVWIN running; a native closer
request destroys only the active Tools window, and a subsequent Main close
callback explicitly quits. Final-build checks passed title-bar dragging and the
LVGL Tools closer without exiting Main, then Main's LVGL closer exited through
lg_app_quit. Executables were verified by FTP SHA256 readback; /kern process paths
identified the launched applications. Evidence is in ignored system
logs/rm10/20261029-lvgl-app (local October29, UTC October30).

COM3/data TCP2002 still refused connections, so no serial capture was available.
System source-contract checks passed; existing baseline staging mismatches remain.
No firmware, driver, boot or cache configuration changed. Runtime native16/mono,
forced OS termination, modal/resize policy and long-duration multiwindow stress
remain unverified or outside this first API implementation.

The final Hello bootstrap also launched correctly and remained open after Escape;
/kern/56/fname confirmed LVHELLO.APP. It is left running on the target. The migrated
Widgets entry point cross-built and passed host rendering checks; its new close
callback was not separately exercised on hardware in this run.


## Single native surface revision

The multiwindow experiment above is superseded. The helper now exposes only one
native GEM window/client surface per application. Content attaches directly to
lg_app_root(), and LV_USE_WIN is disabled. The lg_window_* API and windows.c
example are removed. Simplified init/run and callback-controlled shutdown remain;
MCP is still an empty stub. See APPLICATIONS.md for the current API.

Host ASan/UBSan tests passed direct-root/no-extra-objects, surface dimensions,
initialization failure/retry, singleton rejection, unhandled/refused/reentrant
close requests, explicit quit and cleanup. MCP failure-path tests and native
Widgets renderer/cache smoke passed. make test/probe and both target builds/FPU
scans passed. Symbol checks found no lv_win_* or lg_window_* in either executable.

On A2560Me, the updated Hello template showed only the native GEM frame and a
label centered in the full client area. Escape kept it open; the native closer
invoked its explicit quit callback and removed the process. It was relaunched
and left running. FTP SHA256 readback and /kern/59/fname confirmed the deployed
LVHELLO.APP. Evidence is in ignored logs/rm10/20261029-lvgl-single. Widgets was
cross-built/deployed and host-tested, not relaunched during this revision.
