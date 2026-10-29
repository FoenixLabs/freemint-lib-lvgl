# LVGL 9 GEM window backend implementation plan for A2560Me

Date: 2026-10-29. Audience: Codex implementing a new C application and reusable backend.

## Objective and completion boundary

Build a normal GEM application that renders LVGL 9 widgets inside the work area of an AES window on A2560Me. Support EmuTOS with a working AES and monochrome VDI, and FreeMiNT/XaAES with NVDI in monochrome, RGB565, and 24-bit colour carried in 32-bit pixels. Use software rendering, application-owned backing storage, and VDI raster presentation. The OS and graphics driver own the display, cursor, input devices, memory mappings and hardware.

EmuTOS is an OS, not this application's bootloader. TOSGRAPH is the hardware display path used during the boot/EmuTOS environment; a GEM application starts only after AES/VDI are available. Under FreeMiNT/XaAES, NVDI manages the GUI bitmap display. Launch the application separately in each configured environment. Do not implement an in-process handover from TOSGRAPH to the GUI engine or assume that starting FreeMiNT alone switches graphics modes.

MVP completion requires demonstrated redraw, resize, overlap recovery, mouse and keyboard operation in every required environment. Host tests alone do not establish NVDI or board compatibility. Driver changes, bootloader work, full-screen register access, indexed colour modes, DMA acceleration, and new NVDI drivers are outside the application MVP.

## Hardware facts and resulting constraints

The supplied hardware reference is revision 1.8, dated 2026-10-16, FPGA build 87, RevB3A. Keep it beside this plan as `docs/A2560Me_Hardware_Reference.pdf`. SHA-256: `d66f885b70a40cae3d437edc25ef7d139ef0d1fe9a8038681ddd257524abb1a2`.

| Hardware fact | Implementation consequence |
| --- | --- |
| MC68LC060, big-endian, 66.67 MHz; 8 MB CPU-bus SRAM and 1 GB physical DDR3 (overview, pp. 3–6) | Compile for 68060 with software floating point; allocate only memory supplied by the OS. Physical DDR3 capacity does not imply an equally large process heap. |
| TOSGRAPH uses SRAM and Atari interleaved bitplanes; 1 bpp is a plain MSB-first bit stream (pp. 68–69) | Monochrome presentation goes through VDI using a verified one-plane MFDB. Do not derive a screen pointer from a hardware register. Two- and four-plane colour modes remain unsupported. |
| GUI bitmap engine uses packed DDR3 pixels and configurable stride; supports 1/8/16/32 bpp (pp. 69–71) | Support the required 1/16/32 modes only. Discover VDI's actual format independently of the hardware specification. |
| 16 bpp is RGB565; 32 bpp is numeric `$AARRGGBB`, with alpha ignored by the engine (p. 70) | On the CPU, native 565 words are big-endian. A hardware-format longword has memory bytes AA RR GG BB. Explicitly convert LVGL output when its byte layout differs. |
| 24-bit colour uses 32-bit hardware storage (p. 70) | Never allocate three bytes per pixel merely because a mode offers 16.7 million colours. Packed 24 bpp is a separate optional VDI capability, requiring separate validation. |
| GUI modes include 640×480/512 and 1280×960/1024; 1280×960 hardware verified, 1024-line modes simulated in the manual (pp. 69–70) | Query runtime work area and resolution. Prioritize 1280×960 on hardware; record any untested 1024-line modes. No fixed resolution assumptions. |
| FPGA L2 is 64 KB, write-through; 68060 caches are not snooped (pp. 77–79) | Driver-mediated DMA must handle CPU cache ownership and ordering. L2 coherence alone is insufficient. |
| Build 87 has an FPGA Floating Math Block (pp. 4–5 and GAVIN chapter) | This is memory-mapped hardware, not a CPU FPU or ABI substitute. Do not use it from the GEM backend. |

The manual describes display registers for platform authors. The GEM application must not write TOSGRAPH/GUI registers, alter endian inversion, flip physical screen addresses, modify palettes globally, program DMA, change cache/MMU state, install interrupt handlers, or use supervisor mode to bypass the driver. Even diagnostic hardware reads are unnecessary for MVP; collect board/build identity from the operator or a documented OS service.

## Dependencies and reproducible source management

Use these verified upstream repositories only:

| Submodule path | Upstream | Exact commit | Selection |
| --- | --- | --- | --- |
| `third_party/lvgl` | https://github.com/lvgl/lvgl.git | `7f07a129e8d77f4984fff8e623fd5be18ff42e74` | Commit dereferenced from tag `v9.2.2`; deliberately fixed LVGL 9 baseline, not a claim that it is the newest release. |
| `third_party/gemlib` | https://github.com/freemint/gemlib.git | `68f94dabdffbcd7dc1bda762dc97bdccf1ad9488` | Upstream HEAD resolved on 2026-10-29; review and compile this snapshot before relying on it. Provides AES, VDI and extensions. |

Commit gitlinks and `.gitmodules` in the application repository; a tag name in prose is not a reproducible pin. GEMlib already covers AES/VDI bindings: no invented separate AES or NVDI repository is needed. NVDI is an installed runtime dependency, not a source submodule. EmuTOS, FreeMiNT and XaAES remain installed OS components; do not vendor them to build the app. The compiler, binutils, libc/libm and target startup files form an external toolchain; record versions, download provenance, hashes and ABI in `toolchain.lock` after verifying the actual installation. Add another source submodule only for an identified need with a verified upstream and commit.

Initial repository setup, to be performed during implementation:

```sh
git init
git submodule add https://github.com/lvgl/lvgl.git third_party/lvgl
git -C third_party/lvgl checkout --detach 7f07a129e8d77f4984fff8e623fd5be18ff42e74
git submodule add https://github.com/freemint/gemlib.git third_party/gemlib
git -C third_party/gemlib checkout --detach 68f94dabdffbcd7dc1bda762dc97bdccf1ad9488
git add .gitmodules third_party/lvgl third_party/gemlib
```

Do not run `git submodule update --remote` in builds. Bootstrap with `git submodule update --init --recursive`, then check both SHAs against `dependencies.lock`. Keep upstream files unmodified; prefer project configuration and adapters. If an upstream patch is required, store the patch and justification and document how it is applied reproducibly, or pin a clearly identified reviewed fork. Preserve license notices and record distribution obligations for every dependency and bundled asset.

## Proposed repository tree

```text
.
├── .gitmodules
├── .gitignore
├── Makefile
├── dependencies.lock
├── toolchain.lock
├── README.md
├── AGENTS.md
├── config/
│   ├── lv_conf.h
│   └── profiles.mk                 # mint060, emutos060, host-test
├── third_party/
│   ├── lvgl/                       # submodule
│   └── gemlib/                     # submodule
├── include/lvgl_gem.h              # public backend lifecycle and options
├── src/
│   ├── app/main.c
│   ├── app/demo_ui.c
│   ├── gem/aes_window.c            # lifecycle, messages, visible rectangles
│   ├── gem/vdi_present.c           # MFDB, clip, raster copy
│   ├── gem/vdi_probe.c             # supported format discovery
│   ├── port/lvgl_display.c         # LVGL 9 display and flush adapter
│   ├── port/lvgl_input.c           # queued pointer and keypad events
│   ├── port/platform_time.c
│   ├── render/backing_store.c
│   ├── render/pixel_convert.c
│   └── platform/diagnostics.c
├── tests/
│   ├── host/                      # conversions, clipping, state, failure tests
│   ├── target/vdi_probe.c          # independent diagnostic GEM executable
│   └── target/test_pattern.c
├── scripts/
│   ├── bootstrap.sh
│   ├── verify_dependencies.sh
│   └── check_binary.sh
├── docs/
│   ├── LVGL9_GEM_IMPLEMENTATION_PLAN.md
│   ├── A2560Me_Hardware_Reference.pdf
│   ├── DRIVER_COMPATIBILITY.md
│   ├── TEST_MATRIX.md
│   └── decisions/
└── build/                         # ignored, includes staged GEMlib installation
```

This tree describes future implementation. The plan deliverable does not initialize Git, install dependencies, or claim a working backend.

## Build and ABI contract

Start with a verified `m68k-atari-mint` GCC toolchain. Use C with normal 32-bit `int`, 16-bit `short` and 32-bit pointers on target. Do not enable `-mshort` or fastcall. AES/VDI parameter words must use GEMlib's declared types, not ordinary `int` arrays. Add target compile-time checks for type sizes and the MFDB layout expected by the pinned bindings.

Candidate application flags are `-m68060 -msoft-float -O2`, with warnings and dependency files. Validate compiler support and installed multilibs first. Ensure LVGL, GEMlib, startup code, libc, software math helpers and the final link all use a compatible calling convention. GEMlib's existing `68020-60` non-short target can be used if instruction and ABI inspection confirms compatibility; build it with software floating point as applicable. Avoid hand-maintaining a list of assembly binding sources; use the pinned upstream build and a private installation prefix. Record the exact upstream target, tools and overrides that worked.

Produce separate `emutos060` and `mint060` profiles even if they initially share object files. The EmuTOS profile must avoid mandatory MiNT-only syscalls and dynamic runtime dependencies. Verify its executable and libc startup on actual EmuTOS with AES; cross-linking with a MiNT toolchain alone does not prove compatibility. If the available libc cannot provide this, resolve a suitable verified toolchain/runtime in stage 1 and record it rather than mislabeling a MiNT-only binary.

Use software draw only: `LV_USE_OS = LV_OS_NONE`, one draw unit, `LV_BIG_ENDIAN_SYSTEM = 1`, `LV_USE_FLOAT = 0`, `LV_USE_MATRIX = 0`. Take the exact remaining options from the pinned `lv_conf_template.h`. Disable unused GPU engines, video/image codecs, filesystem backends, large demos and unnecessary fonts/widgets. Keep a small UI with a label, button, slider, editable text and scrolling list. Do not rely on LVGL 8 APIs such as `lv_disp_drv_t`; use `lv_display_t` and the version 9 display API.

Inspect the final executable and linked objects with target tools for unsupported CPU/FPU instructions, dependencies, sections and stack/heap settings. Software float calls are acceptable; 68881/68060 FPU instructions are not. Inspect libraries too, because application flags do not rebuild precompiled libraries. Record how the inspection distinguishes instructions from embedded data.

## Architecture and ownership

Use one AES event thread and one LVGL display for one window. All LVGL calls and AES/VDI calls occur on that thread. Application callbacks never reenter the renderer. The reusable backend exposes create, process event, service timers, resize and destroy functions; the demo owns the widget tree.

Data flow:

```text
AES messages and input -> event adapter -> LVGL input queues/widgets
clock -> LVGL timer handler -> software rendering into partial draw buffer
flush -> copy/convert into persistent client backing store -> dirty regions
present -> AES visible rectangles + VDI clip -> VDI screen raster copy
WM_REDRAW -> present persistent backing store without needing a new LVGL frame
```

The backing store is authoritative for window content, including content currently covered by other windows. MVP uses LVGL partial rendering into an RGB565 strip for all output modes, converting into a native backing MFDB. This reduces monochrome memory consumption and avoids depending on LVGL I1 drawing support. Native RGB565 needs only a row copy when its verified layout matches. Truecolour conversion expands 565 to 8-bit channels; this baseline limits rendered colour precision to 565 even on the 32-bit desktop. A subsequent acceptance task adds an XRGB8888 rendering profile for full 24-bit colour fidelity, after validating LVGL's actual big-endian byte layout. Keep the output format independent of the render format.

Suggested context fields: AES application/window IDs, VDI virtual handle, outer/work rectangles, display/input/group pointers, format descriptor, draw buffers, backing MFDB, allocation sizes, dirty-region set, input queues, tick state, active/iconified flags and ownership flags for cleanup. Keep geometry helpers and conversions independent of GEM for host testing.

### VDI discovery and presentation contract

1. Call `appl_init`, obtain the desktop/work geometry and physical workstation handle through documented AES APIs, then open a virtual VDI workstation with `v_opnvwk`. Do not open or close the desktop's physical workstation.
2. Query baseline capabilities using `vq_extnd`. Detect the documented EdDI/NVDI extension before using `vq_scrninfo`; the pinned GEMlib source documents its 272-word result. Allocate the full result, initialize it, and interpret fields only for the supported extension version.
3. Record layout class, plane count, channels/bit positions, byte order, line width, and extension/version identity. Do not dereference the returned screen address. Guard version-dependent fields and validate lengths/ranges. A depth of 16 does not distinguish RGB555 from RGB565.
4. Without EdDI, permit only a proven standard monochrome raster path. Unknown truecolour packing must produce a clear unsupported-mode message, not guessed writes. Reject indexed colour depths other than the explicitly supported monochrome case.
5. Prove memory-MFDB to screen `vro_cpyfm` behaviour independently of LVGL. Fill all MFDB fields, including `fd_stand`, `fd_nplanes`, dimensions, padded word width and reserved fields. Use screen destination MFDB with `fd_addr = NULL` as documented. Use the named source-copy raster operation.
6. For one-plane data, use documented standard/device conversion (`vr_trnfm`) if needed. For packed truecolour, accept device-dependent MFDBs only after the installed driver demonstrably supports that representation. Do not assume `vr_trnfm` converts arbitrary packed colour arrays. If the installed driver cannot copy an application-owned native MFDB, document that driver capability gap and resolve a documented offscreen facility or driver change as a separate prerequisite.

MFDB width is padded to a multiple of 16 pixels; `fd_wdwidth = (width + 15) / 16`. For a validated contiguous device layout, derive native row storage from that padded width and depth (mono: words × 2; 565: padded pixels × 2; 32 bpp: padded pixels × 4). Verify this against driver conventions; MFDB has no generic arbitrary-byte-stride field. Allocate its exact required storage and do not confuse its stride with the physical screen stride or LVGL draw-buffer stride.

### Flush and redraw rules

The LVGL flush callback receives a client-local inclusive area and a pixel pointer. Derive the source stride from the pinned LVGL buffer configuration, never from screen width. Validate the rectangle and checked allocation arithmetic. Copy/convert the rendered area into the persistent backing store even when the window is hidden or covered, add it to a bounded dirty-region set, and call `lv_display_flush_ready` exactly once after the source buffer has been consumed. An empty clipped area still completes the flush. Do not retain the source pointer after ready.

Presentation is separate from flush completion. Coalesce dirty rectangles, optionally using `lv_display_flush_is_last` as a scheduling hint. Enumerate the window's current visible rectangles with `WF_FIRSTXYWH`/`WF_NEXTXYWH`. Intersect each with the work area and translated dirty/redraw region. Apply the VDI clipping rectangle; copy only that intersection. Source coordinates are desktop intersection minus work-area origin. AES rectangles are position/size; VDI and LVGL endpoints are inclusive, so right = x + width - 1. Handle empty rectangles before subtracting one.

For `WM_REDRAW`, use the requested desktop rectangle and the stored pixels immediately. Cover/uncover must work even when LVGL has no invalidated widgets. Keep update locking short using balanced `wind_update(BEG_UPDATE/END_UPDATE)` around enumeration/presentation as required by AES. Preserve VDI clipping state. If mouse hiding is required by the driver, balance hide/show tightly around raster operations. Never call the LVGL timer handler or wait for another event while holding the update lock. Recalculate visibility for every presentation; never cache it across AES interactions.

### Pixel conversion

Use explicit byte/word stores and fixed-width arithmetic; test on host endianness and target endianness. `LV_BIG_ENDIAN_SYSTEM` must be enabled but is not proof that every LVGL pixel struct matches the hardware's numeric longword layout.

For 565, verify red `0xF800` -> F8 00, green `0x07E0` -> 07 E0, blue `0x001F` -> 00 1F, white `0xFFFF` -> FF FF on the native big-endian path. Expand channels with bit replication for the 32-bit output. For `$AARRGGBB` choose an opaque/unused AA value consistently, e.g. FF, after confirming the driver's convention. Hardware ignoring alpha does not imply AES provides transparent windows; composite widgets into an opaque window surface.

For monochrome, compute integer luminance, apply an explicit threshold, and pack the left pixel in bit 7 of each byte. Map white/black to the VDI device's actual bit polarity, which can differ from the TOSGRAPH reset palette. Keep conversion local; never change desktop colours. Optional ordered dithering must use absolute client coordinates so partial redraws match full redraws. Handle non-byte-aligned dirty regions with read/modify/write masks so neighbouring pixels and padding survive.

Direct LVGL I1 rendering is optional after MVP. If selected later, account for the pinned LVGL version's eight-byte palette prefix, byte-aligned expanded flush areas, stride/padding and bit polarity. Do not feed the palette bytes to VDI.

### Events and timing

Use `evnt_multi` for messages, mouse/button/keyboard changes and timer deadlines. Dispatch every returned event bit, since one result can contain multiple events. Mouse motion must be observable without a button click; use supported mouse-rectangle events or bounded timer sampling with `graf_mkstate`. Translate desktop coordinates to client coordinates and exclude decorations. Deliver press/release transitions in order; keep capture for an active drag through release, clamp coordinates as needed, and prevent a stuck press on focus changes. AES owns the system pointer.

Use a bounded input queue consumed by LVGL `lv_indev_t` read callbacks, with the version's continuation flag when applicable. Queue keyboard presses/releases, translate navigation to LVGL keys, and attach a focus group. Explicitly define a first keyboard encoding (ASCII plus documented Atari mapping); do not treat AES scan codes or Atari bytes as UTF-8. Handle click counts deliberately so an AES double-click does not create an extra unmatched press.

Service `lv_timer_handler` outside update locks. Use a monotonic millisecond tick source behind `platform_time.c`: a documented OS service if available, otherwise a verified read-only TOS timer facility with wrap-safe unsigned differences. A 200 Hz source has 5 ms granularity. Do not depend on MiNT-only timing in the EmuTOS profile, use wall clock for animation, install a timer interrupt, or busy-wait. Set the AES timeout to the bounded next LVGL deadline/input sampling interval; suspended/iconified windows may use a longer interval without blocking other AES work.

Handle `WM_MOVED`, `WM_SIZED`, `WM_FULLED`, `WM_TOPPED`, `WM_NEWTOP`, `WM_CLOSED`, termination requests, and iconify/uniconify only where the AES reports support. Ignore messages for other window IDs. Movement changes presentation origins, not LVGL content dimensions. Resize updates both.

### Resize and cleanup

Compute a proposed new work size with AES geometry APIs, checked arithmetic and explicit minimum/maximum limits. Allocate the replacement backing store and draw resources before freeing the old ones. At an LVGL idle boundary with no outstanding flush, replace buffers, update `lv_display_set_resolution`, invalidate the screen and render the new surface before presenting it. Swap ownership consistently; no callback may see freed buffers. On allocation failure, restore the old window geometry/storage or show a controlled error and close cleanly. Exposed pixels must never display uninitialized memory.

Cleanup must be safe after every partial initialization failure: stop processing callbacks, delete LVGL input/display objects before their buffers, close/delete an existing window, close the virtual VDI workstation, release application allocations, and call `appl_exit` when initialized. Repeated open/close must not leak handles, memory or update locks.

## Memory and performance budget

Start with one 32-row RGB565 draw strip; a second strip is optional and does not imply parallel rendering. Store one persistent native backing surface. Use checked `size_t` multiplication and bound dimensions before narrowing to AES short coordinates.

| Client dimensions | Mono backing with 16-pixel padding | RGB565 backing | 32-bit backing | One 32-row 565 strip |
| --- | --- | --- | --- | --- |
| 640×480 | 38,400 B | 614,400 B | 1,228,800 B | 40,960 B |
| 1280×960 | 153,600 B | 2,457,600 B | 4,915,200 B | 81,920 B |

These examples exclude LVGL heap, fonts, application state, allocator overhead and alignment. A full 32-bit surface is too expensive to assume available within an 8 MB SRAM environment. Set profile budgets from measured free OS memory; allow the mono path to work without any full RGB frame. Use ordinary OS allocation first. Request alternate memory only through verified TOS/MiNT allocation facilities and only if the driver can consume it; never allocate at fixed physical addresses or touch reserved boot/debug SRAM.

Measure conversion time, VDI copy time, LVGL service time, bytes updated, peak allocation and input latency. Bound dirty-region count and fall back to one bounding rectangle. Avoid full-window refresh for each tick. Set performance targets after a board baseline, with an initial goal of idle event responsiveness and no sustained busy loop; do not promise a frame rate from DDR3 bandwidth figures. Record median/p95 latency under active redraw and background application load.

## DMA and cache policy

The first implementation calls synchronous VDI operations only. NVDI may use DMA internally; the installed driver must provide a correct contract for ordinary application buffers. If testing exposes cache-related corruption, retain logs and escalate the driver integration issue instead of disabling caches globally from the app.

Any later acceleration requires a documented OS/driver API that owns DMA channels, validates addressability, controls cache mappings, arbitrates other users, fences completion and defines buffer lifetimes. Driver-side rules from the manual are:

- Before DMA reads CPU-written data, push dirty 68060 data-cache lines or use suitable write-through/cache-inhibited mappings. Handle posted-write ordering before hardware consumption.
- Before DMA writes a destination, resolve dirty CPU lines and partial-line sharing according to the OS DMA API; after completion, invalidate affected CPU lines before CPU access. Align/round maintenance ranges without corrupting neighbouring allocations.
- MDMA/VDMA DDR3 writes invalidate FPGA L2 entries before DONE; their DDR3 reads wait for the L2 write buffer. This does not maintain the CPU cache.
- SDMA touches SRAM and bypasses L2, so CPU source/destination cache maintenance remains necessary. MDMA covers SRAM/DDR3 transfers; VDMA is the DDR3 mover/blitter. Do not select an engine using only the apparent pointer range.

Do not put `CPUSH`/`CINV`, DMA-register writes, or physical framebuffer flips in the normal GEM app. An asynchronous driver API would require a reviewed completion/lifetime design before it can replace synchronous presentation. The copied LVGL strip can be released once copied into owned backing storage, but a backing surface still in driver use cannot be resized or freed until fenced.

## Staged implementation tasks and acceptance gates

### Stage 0 Establish source and environment

Create the source repository and exact submodules, record licenses, toolchain and target runtime versions, and add a README and AGENTS.md with build commands and the hardware-access boundary. Record EmuTOS/AES, FreeMiNT/XaAES, NVDI/board driver version, configured mode and cache policy per test session. Use operator-supplied FPGA build identity if no documented OS query exists.

Acceptance: a clean recursive clone resolves both pinned SHAs; bootstrap refuses mismatches; target compiler can compile/link a minimal executable; no speculative download URL or unpinned network fetch exists in the build.

### Stage 1 Prove GEM and raster capabilities without LVGL

Build a minimal GEM probe using pinned GEMlib. Open a window, log capabilities and paint known patterns from an owned MFDB. Implement visible-region redraw and clean teardown first. Validate EmuTOS executable compatibility separately. Save raw query results and visual results in DRIVER_COMPATIBILITY.md.

Acceptance: mono, RGB565 and 32-bit truecolour patterns have correct channels, polarity and stride; odd-width patterns do not wrap; overlapping windows are untouched; cover/uncover restores content. Unsupported modes fail clearly. If truecolour MFDB support is absent, resolve that prerequisite before stages 3–5 are claimed complete.

### Stage 2 Build LVGL and portable conversion primitives

Compile pinned LVGL with the reduced configuration, software draw and target ABI. Implement format descriptors, checked allocation helpers, rectangle conversion/intersection, mono/565/32-bit conversion and backing-store ownership. Add focused host tests and a target byte-dump test. Build the smallest static widget scene.

Acceptance: host tests pass with memory checking where available; target primary-colour bytes match the selected format; final target binary contains no required FPU instructions; mono allocation never includes a full RGB backing surface.

### Stage 3 Integrate monochrome EmuTOS window

Wire the LVGL 9 partial-buffer display, authoritative backing store, timer service and AES redraw. Implement mouse/key queues and focus group. Use a minimal two-colour style with legible text and visible keyboard focus.

Acceptance: button, slider, text edit and scroll list work under EmuTOS AES; obscured updates appear correctly when uncovered; idle exposure redraw works without a new widget update; closing returns to the desktop without residue, resource leak or locked mouse.

### Stage 4 Integrate NVDI colour modes

Enable the validated native RGB565 and 32-bit presentation paths. Reuse all AES and lifecycle code. Add a full-fidelity XRGB8888 LVGL draw profile with explicit format conversion for the 32-bit target, keeping the 565 profile for memory-sensitive use. Reject packed 24-bit storage unless separately implemented and verified.

Acceptance: FreeMiNT/XaAES tests pass at 16 and 32 bpp with correct RGB order. A gradient and selected 8-bit channel values demonstrate 24-bit fidelity in the XRGB8888 profile. Other GEM applications remain responsive during animation and raster updates.

### Stage 5 Complete lifecycle and robustness

Add resize/full-size/restore, supported iconification, focus loss, application termination and allocation-failure recovery. Instrument resource ownership and timing. Apply explicit maximum client sizes and bounded event/dirty queues.

Acceptance: resize and move stress passes in all required modes; each injected allocation failure leaves a usable desktop; twenty open/close cycles show no growing owned allocations or handle count; no stale pointer remains after resize; background widgets never draw outside current visible regions.

### Stage 6 Verify hardware and package

Run the matrix below on A2560Me, retaining build manifest, executable hashes, configuration, logs and photographs/screenshots. Document reproduction steps, mode limitations, measured memory and performance. Supply a runnable `.APP` (and resources if needed), licenses and launch instructions per profile.

Acceptance: all mandatory hardware rows have recorded pass evidence. Unavailable hardware or modes must be listed as untested, with the exact remaining procedure; do not label the backend hardware-validated from compilation or emulator runs.

### Stage 7 Optional optimization

Only after correctness, profile strip height, dirty coalescing, conversion tables and native draw formats. Consider direct I1 or a driver-provided offscreen/DMA API as independent changes. Repeat redraw, cache and ownership tests after each optimization. No hardware acceleration is required for MVP acceptance.

## Testing matrix and evidence

| Environment | Required coverage | Evidence |
| --- | --- | --- |
| Host with normal byte order | Converters, rectangle math, stride, queue ordering, allocation failure | Automated results, sanitizer output when supported |
| Big-endian target or suitable emulator | ABI, struct sizes, LVGL byte layout, soft-float linkage | Target assertions, pattern bytes, binary inspection |
| EmuTOS with AES and TOSGRAPH mono | Launch, input, all redraw/lifecycle scenarios | Version/mode log and target captures |
| FreeMiNT/XaAES/NVDI mono where configured | Shared mono path and scheduling | Version/mode log and captures; identify unavailable configuration explicitly |
| FreeMiNT/XaAES/NVDI RGB565 | Native MFDB copy and LVGL integration | Raw format report, primary-colour pattern and UI captures |
| FreeMiNT/XaAES/NVDI 32-bit storage | Pixel order and full colour fidelity | Raw format report, byte tests, gradient and UI captures |
| A2560Me with caches enabled under supported OS configuration | Repeated redraw and background load | Long-running stress results, corruption checks |
| Supported alternate cache configuration | Driver comparison if safely configurable by OS/operator | Configuration and results; no app cache toggling |

Portable cases: client widths 1, 7, 8, 15, 16, 17, 319 and 641; nonzero work origins; rectangles crossing every edge; empty rectangles; mono partial bytes; padded rows; all primaries; checked arithmetic at allocation limits; timer wraparound; simultaneous AES event bits; queue pressure with release delivery; failed replacement allocations. Use an independent slow reference converter to avoid tests reproducing the same implementation error.

GEM scenarios: overlap by another window while widgets update; uncover while idle; move partly offscreen; maximize/restore; repeated odd-size resize; minimize/restore where available; close during animation; keyboard focus transfer; drag outside the client then release; double-click; termination with queued events. Check cursor restoration and update-lock balance after every error path.

An emulator can establish generic AES/VDI behaviour only for the hardware and drivers it actually models. It does not establish A2560Me DDR3, cache, DMA or NVDI board-driver behaviour. No bootloader modification is required for any application test.

## Risks and decisions that must be recorded

1. Actual NVDI board-driver MFDB support and EdDI availability: stage 1 is the decisive gate; the hardware reference does not specify that software interface.
2. EmuTOS runtime compatibility of the selected libc/startup and available AES: prove launch before integrating LVGL.
3. LVGL big-endian output: test memory bytes for each chosen render format and use explicit converters; do not cast a pixel struct into a hardware longword.
4. Process memory availability and VDI-accessible memory: measure before choosing window limits; preserve the lean mono path.
5. Unknown installed driver or FPGA revision: record identity and test configuration instead of silently assuming build 87 behaviour.
6. Performance targets: select measured budgets after the first board baseline; keep correctness and desktop responsiveness mandatory.

Use short decision records for each resolved issue, including the tested environment, selected approach, rejected assumptions and follow-up tests.

## Codex handoff instructions

Implement this plan stage by stage in the existing workspace. First inspect repository instructions and current files; preserve user changes. Treat this document as the implementation specification. Build a regular GEM application and reusable LVGL backend. Never program display/DMA registers or alter boot flow from the app.

Start with stage 0 and the stage 1 GEM raster probe. Do not integrate LVGL until the native mono/colour copying contract has been demonstrated or its exact blocker recorded. Use the two verified upstream URLs and exact commits above, commit their gitlinks, and keep all configuration/adapters in the application repository. Verify installed toolchain/runtime compatibility instead of downloading from guessed locations.

For each stage, provide a focused commit with implementation and meaningful checks, update README/build commands and the test matrix, and report the concrete result. Run available host and cross-build checks autonomously. If hardware is unavailable, complete useful host/build work, supply the target probe and exact manual steps, and state which acceptance gates remain open. Do not fabricate device results. If a dependency capability blocks integration, document it with probe evidence and continue independent work.

Required final delivery: buildable sources, pinned submodules and dependency verifier, target profiles, reusable backend, demo app, automated conversion/geometry tests, standalone VDI probe, binary inspection procedure, compatibility matrix, hardware test record, licenses and concise run instructions. Completion means all required acceptance gates pass with evidence, or the report explicitly identifies remaining external prerequisites without claiming completion.

## Verified source references

- Supplied [A2560Me Hardware Reference](A2560Me_Hardware_Reference.pdf), revision 1.8, build 87: pp. 3–6 (CPU/memory), 68–71 (TOSGRAPH/GUI layouts), 77–79 (cache/coherency). Hardware facts above come from this document; architecture and stage choices are project decisions.
- [LVGL v9.2.2 pinned source](https://github.com/lvgl/lvgl/tree/7f07a129e8d77f4984fff8e623fd5be18ff42e74): `src/display/lv_display.h`, `src/misc/lv_color.h`, `lv_conf_template.h`, and `docs/porting/display.rst`. These define the selected version's display APIs, colour formats, big-endian/configuration options and I1 palette handling.
- [GEMlib pinned source](https://github.com/freemint/gemlib/tree/68f94dabdffbcd7dc1bda762dc97bdccf1ad9488): `n_vq_scrninfo.c`, `vq_extnd.c`, `vro_cpyfm.c`, `vr_trnfm.c`, `v_opnvwk.c` and `Makefile`. Bindings and comments document extension results and raster/workstation calls; installed driver support must still be tested.
- [FreeMiNT project](https://freemint.github.io/) identifies GEMlib as its GEM application bindings. No additional speculative binding repository is assumed.
