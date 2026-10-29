# Driver compatibility

## A2560Me session,2026-10-29

Runtime evidence: FreeMiNT 1.19, October 17 kernel build;68060, no FPU, 66.3 MHz reported by
/kern/cpuinfo;1280×1024 at 60 Hz live RM10 HDMI. fVDI and EdDI cookies are present.
EdDI function 0 returns1.00. `vq_scrninfo` reports packed pixels, software CLUT,
32 planes,R/G/B widths8/8/8 at bit positions16..23/8..15/0..7, no alpha/genlock,
and eight unused high bits. Exact installed driver revision, FPGA revision and
cache policy were not independently identified; they are not inferred from the
hardware manual or source checkout.

The native MFDB probe uses319×193 pixels with 20 words per row in MFDB terms,
1,280bytes per row at32 planes. RGB/white/black bands and full-channel ramps were
visually correct. An earlier probe used rowbytes/2 for fd_wdwidth, producing
corruption. That application bug was corrected to ceil(pixelwidth/16), and the
same test passed. No driver modification was needed.

EdDI 1.0 does not version the newer stride/address/organization fields. Discovery
uses baseline depth and channel maps, and permits this native contract only for
identified fVDI. A documented Ssystem cookie query avoids supervisor access.
The application never dereferences the returned screen address. Unknown truecolour
formats fail with an alert. NVDI, native 16-bit and EmuTOS paths remain untested here.

The earlier mono demo established launch, tab switching, ASCII text entry, movement,
cover/uncover by a desktop dialog and clean Escape close in this same colour
desktop. It did not establish a real depth 1 screen mode. Early immediate captures
showed no change after short clicks; later captures showed delayed updates. The
adapter now preserves AES button snapshots, removing a code-level press-loss race;
these observations do not isolate that race from rendering latency quantitatively.

## Binary checks

The demo's compiler assembly and linked external-runtime text pass the FPU check.
Embedded font/image data in a.out .text is excluded by checking project assembly
and link-map ranges separately. This is an FPU check, not complete CPU/runtime
certification. Physical launch on a no-FPU target supplies separate evidence.

## LVGL colour rendering and platform keyboard policy

The native 32-bit Widgets scene exposed a separate pinned-LVGL issue: opacity/mask
fills treated the bytes of numeric AARRGGBB as B,G,R, producing purple text and
striped button edges on the big-endian CPU. The project now uses the documented
custom software-blender hooks to supply explicit B,G,R channels for RGB888/XRGB
fills and opaque BGRA layers. Host tests cover all opacities, masks, padded rows
and alpha preservation. Target retesting shows normal blue accents, neutral text
and uniform button fills. Image/font assets remain the original upstream bytes.

The platform wrapper removes the demo keyboard's event handlers before deleting
its instance. Pointer focus and physical USB HID text entry preserve the full
400-pixel client height; A2560Me entry was confirmed with no virtual keyboard.

## Native32 rendering follow-up

The Widgets backend now uses native FF/R/G/B rendering into its own MFDB for the
validated packed32 contract. It preserves upstream BGR/BGRA assets/layers and
adapts blend destinations only inside the registered native backing allocation.
Profile-only cached scrolling reuses backing pixels, with fallback on unrelated
invalidation or overlays. Neither optimization accesses the screen address or
changes the driver, cache policy or framebuffer format. Headless reference
comparisons and A2560Me F8 measurement passed; see TEST_MATRIX.md. Native16 still
uses explicit conversion and has not been validated in a target16-bit mode.
