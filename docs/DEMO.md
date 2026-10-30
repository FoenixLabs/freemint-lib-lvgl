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
Text input currently accepts ASCII. The native closer requests exit through the demo's explicit application callback.
Escape is passed to LVGL and does not exit. Drag the title
bar to move the window. The Widgets sample remains fixed-size; the helper supports optional native resizing
for applications that select LG_APP_RESIZABLE. Full-size/iconification remain future work.
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

In native32 mode LVGL now renders directly into the owned backing MFDB in
FF/R/G/B byte order. A port adapter intercepts the two pinned RGB888 blend entry
points; only destinations inside that backing allocation use native channels.
Assets and intermediate layers retain upstream BGR/BGRA layout. Common fills,
text masks and images have native paths; uncommon formats/blend modes temporarily
reverse the clipped destination bytes and delegate to the original renderer.
The build renames the original definitions with per-source compiler flags; it
does not patch dependency files. There is no final pixel-conversion pass.

RGB565 and real depth1 retain RGB888 rendering and explicit conversion, now with
a full-window draw buffer. RGB565 avoids CPU-dependent bitfields; mono uses stable
ordered dithering. Native16 drawing remains future work. The existing portable
custom-blender hooks also keep intermediate-layer colour correct on big-endian CPUs.

MFDB `fd_wdwidth` is padded pixel width divided by 16, independently of depth.
The 600×400 native 32-bit backing is 972,800 bytes with no separate draw strip, and the LVGL
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
components/lib-lvgl/build/host-demo/lvwidgets --native-smoke-test /tmp/native.ppm
cmp /tmp/widgets.ppm /tmp/native.ppm
```

Requires SDL2 development headers and pkg-config. The smoke test checks rendering,
pointer tab navigation, physical text entry, no keyboard object, full viewport
height across focus/ready/cancel, and teardown under ASan/UBSan. It does not launch
a Linux GUI unless run without a headless option. The native test also compares
all 24 cached frames with full redraws and checks invalidation/overlay fallbacks.
Renderer tests compare opacity/masks, image formats, padded/unaligned destinations
and guard bytes with the original renderer under ASan/UBSan.

## Scroll measurement

F8 runs a synchronous diagnostic: it selects Profile, scrolls by 8-pixel steps
from 0 to96 and back, and forces 24 rendered/presented frames. Input waits until
the diagnostic completes. It appends elapsed, conversion and presentation times
and pixel counts to LVBENCH.TXT in the working directory. Run with the window
fully visible and without other interaction. The file write is outside timing.
MiNTlib clock() uses the elapsed 200Hz clock, so timings have 5ms granularity;
summing short strip timings is approximate. This measures forced scroll render
throughput, not end-to-end HID latency or normal animation frame scheduling.

Native32 Profile scrolling reuses the unchanged pixels in that owned backing
buffer, then renders the exposed strip and the right-hand area containing fixed
controls. It requires a scroll-only invalidation batch, unchanged geometry and
flat ancestors, and falls back to full drawing on other invalidation, overlays
or unsupported geometry. This optimization is specific to the pinned Widgets
Profile page, not a general scrolling API. Its scrollbar appearance stays constant
to avoid restyling the entire subtree at each scroll start/end. AES exposure and
visible-rectangle clipping still use the authoritative backing buffer.

F6 toggles this cache for comparison. F7 enables optional reduced effects for the
current run: no shadows, style transitions or animated scroll starts on existing
objects. Restart restores the default effects. F8 also records solid-fill,
masked-fill and image timing/pixel counts, fallback calls, cache hits and mode.
These buckets are approximate: the 5ms clock cannot resolve every short call.

A2560Me native32 measurements for 24 forced frames:

| Configuration | Elapsed |
|---|---:|
| Previous 100-row RGB888 draw strip | 9.625–9.670s |
| Full-window RGB888 buffer | 8.870s |
| Native32 direct drawing | 6.760s |
| Native32 with constant Profile scrollbar, cache disabled | 4.975s |
| Native32 with Profile cache, default effects | 2.930s |
| Same, optional reduced effects | 2.815s |

The default improves this test about 3.3× over the previous build. It does not
establish HID latency or animation FPS. Native16 and depth1 remain target-untested.

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

The GEM entry point now uses `lvgl_app.h` for initialization and the event loop.
See APPLICATIONS.md for the single native-window application template.
