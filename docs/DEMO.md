# Upstream Widgets demo on GEM

This builds the pinned LVGL 9.2.2 `lv_demo_widgets()` with its original Profile,
Analytics and Shop tabs and assets. Upstream sources remain unmodified. The
platform wrapper removes the virtual keyboard and its associated demo handlers;
A2560Me text entry uses the physical keyboard only.

## Build and launch on A2560Me

From a2560me-system:

```sh
make -C components/lib-lvgl test
make -C components/lib-lvgl probe demo MINTLIB="$PWD/components/mintlib"
```

Copy `build/mint060/LVPROBE.APP` and `build/mint060/widgets/LVWIDGET.APP` from this
component to `D:\APPS\LVGL\`. Both are standalone GEM applications. Launch the
probe first in a new driver/mode and check the RGB/white/black bands and ramps.
It writes LVPROBE.TXT in its launch directory. Then launch LVWIDGET.APP.
The fixed work area is 600×400, requiring at least a 640×480 desktop.

Use the mouse for tabs, drag scrolling, text-field focus and sliders.
On XaAES with wheel input, the wheel scrolls the container under the pointer by
32 pixels per notch, without a prior click or changing keyboard focus. Vertical
and horizontal WM_WHEEL orientations are supported. Movement is immediate and
bounded; nested containers chain at their limits only when LVGL permits it.
Wheel events outside the client or during a captured button drag are ignored. Tab/Shift-Tab
traverse keyboard focus; Enter activates; arrows and Backspace are translated.
Text input currently accepts ASCII. Escape or the closer exits. Drag the title
bar to move the window. Resize/full-size/iconification remain future work.
The virtual keyboard never appears when focusing a text field; focus also keeps
the full window height available. The unused keyboard widget support is compiled
because the unmodified upstream demo references it, but the wrapper deletes its
instance after safely detaching handlers that reference it.

## Native colour contract

The backend queries the VDI depth and the documented EdDI cookie, then validates
the full 272-word `vq_scrninfo` descriptor. It accepts packed native RGB565 and
32-bit storage with eight-bit R/G/B at bits 16/8/0. It rejects indexed colour,
RGB555, swapped byte order, packed24 and unknown maps. EdDI 1.1+ uses its versioned
byte-order field. The reviewed fVDI1.0 native logical MFDB contract is accepted
only with the fVDI cookie present and exact channel maps; its newer extra fields
are not interpreted as an EdDI 1.1 promise. Cookie lookup uses MiNT Ssystem, with
no application supervisor fallback or direct hardware access.

LVGL renders into a 100-row RGB888 strip. Its explicit B,G,R bytes avoid the pinned
version's CPU-dependent RGB565 bitfields and preserve full eight-bit colour.
The project supplies byte-explicit C fill hooks through LVGL's custom-blender
interface to correct the pinned version's little-endian assumptions in opacity/
mask fills and opaque ARGB layers. No dependency sources are patched.
Conversion writes native big-endian 565 or FF/R/G/B into the owned backing MFDB.
32-bit output is not quantized through 565. For genuine depth 1, the existing mono
path uses stable ordered dithering and VDI black/white expansion.

MFDB `fd_wdwidth` is padded pixel width divided by 16, independently of depth.
The 600×400 native 32-bit backing is 972,800 bytes, draw strip 180,000 bytes, and the LVGL
heap1 MiB, plus code, fonts/assets,128 KiB stack and allocation overhead. No screen
pointer or physical memory is accessed. Presentation uses synchronous VDI raster
copies clipped to current AES visible rectangles. WM_REDRAW reuses saved pixels.
Only the union of changed client rectangles is presented after a render pass;
WM_REDRAW still copies any requested exposed area from the authoritative backing.
Colour conversion operates on row spans, selecting depth once per row. Opaque
fills generate one row and copy it to the remaining rows, avoiding repeated blend
decisions. The demo uses `-O2` rather than `-Os`, retaining `-m68060 -msoft-float`.
All rendering and input run on the AES thread. Mouse button events retain their
AES snapshot rather than being overwritten by a later live-state sample.

## Recorded target evidence

2026-10-29, A2560Me through RM10 HDMI/HID and verified FTP application deployment:
FreeMiNT 1.19 (October 17 build), CPU68060/no FPU, 1280×1024 HDMI, fVDI/EdDI 1.00,
packed 32-bit with RGB8 bit maps16/8/0. The independent 319-pixel-wide native pattern
has correct primaries, white/black and gradients after the MFDB word-width fix.
The full native 32-bit Widgets scene has correct colour fills after the blend-hook
fix; physical A2560Me text entry works with no virtual keyboard. Native Analytics
navigation and its gradient were also confirmed. The demo was left running.
See DRIVER_COMPATIBILITY.md and TEST_MATRIX.md for the current acceptance record.
Raw captures, serial-connection failure and deployment hashes remain in the ignored
system logs/rm10 directory, not in this source repository. No firmware, driver,
boot configuration, cache setting or desktop setting was changed.

## Optional Linux development checks

These are headless development aids, not the A2560Me acceptance path:

```sh
python3 components/lib-lvgl/scripts/build_demo.py --host --sanitize
components/lib-lvgl/build/host-demo/lvwidgets --smoke-test /tmp/widgets.ppm
```

Requires SDL2 development headers and pkg-config. The smoke test checks rendering,
pointer tab navigation, physical text entry, no keyboard object, full viewport
height across focus/ready/cancel, and teardown under ASan/UBSan. It does not launch
a Linux GUI unless run without a headless option.

## Scroll measurement

F8 runs a synchronous diagnostic: it selects Profile, scrolls by 8-pixel steps
from 0 to96 and back, and forces 24 rendered/presented frames. Input waits until
the diagnostic completes. It appends elapsed, conversion and presentation times
and pixel counts to LVBENCH.TXT in the working directory. Run with the window
fully visible and without other interaction. The file write is outside timing.
MiNTlib clock() uses the elapsed 200Hz clock, so timings have 5ms granularity;
summing short strip timings is approximate. This measures forced scroll render
throughput, not end-to-end HID latency or normal animation frame scheduling.

Selecting RGB565 or XRGB8888 by depth does not by itself permit direct drawing
into the native MFDB. The pinned LVGL renderer's BGR/BGRA byte layout differs
from the target's big-endian RGB565 and X/R/G/B layouts. A future native renderer
must cover fills, text masks, images, gradients and intermediate layers, with
validation in each screen mode. No direct framebuffer access is needed.

## AES wheel input

The backend queries AES_WINDOW bit9 and enables WF_WHEEL with enable=1 and
mode=WHEEL_MESAG, as implemented by the pinned XaAES. WM_WHEEL uses its recorded
screen coordinates translated to client coordinates; orientation supplies the
direction (including AES reverse-wheel policy), and the signed low-byte count is
normalized to a magnitude. No synthetic mouse press or keyboard event is used.
The pinned LVGL pointer enc_diff path targets the last pressed object, so it is
unsuitable for desktop hover-wheel behavior; this backend instead hit-tests LVGL
layers and calls bounded scroll on the nearest eligible ancestor.

A2560Me test limitation: injected AES WM_WHEEL messages scroll the live demo;
RM10 USB HID wheel commands did not scroll it. The checked-in EmuTOS A2560 mouse
path consumes three-byte PS/2 packets and forwards only movement/buttons; it
contains no wheel-field forwarding. This suggests a lower input-path limitation,
not a verified physical-wheel implementation. No firmware was changed or flashed.
Other AES implementations and physical horizontal wheel delivery remain untested.
