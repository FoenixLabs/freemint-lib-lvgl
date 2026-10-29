# Upstream Widgets demo

The demo calls the pinned LVGL 9.2.2 `lv_demo_widgets()` without editing upstream
sources or assets. Its Profile, Analytics and Shop tabs are included.

## GEM executable

From a2560me-system:

```sh
make -C components/lib-lvgl demo MINTLIB="$PWD/components/mintlib"
```

Copy `components/lib-lvgl/build/mint060/widgets/LVWIDGET.APP` to the target and
launch from the GEM desktop. It is standalone; images/fonts are linked in.
Use a desktop of at least 640×480. The fixed work area is 600×400; drag its title
bar to move it. Use the mouse to switch tabs, scroll, edit fields and drag sliders.
Tab/Shift-Tab traverse keyboard focus; Enter activates; arrows and Backspace are
translated. Text input currently accepts ASCII. Escape or the closer exits.

The candidate `PROFILE=emutos060` build uses the same ABI and separate outputs.
Neither profile has been launched on target during this change.

The GEM backend owns a padded one-plane backing store, draws LVGL into a 32-row
RGB888 strip and applies stable 4×4 ordered dithering. RGB888's B,G,R bytes avoid
relying on target RGB565 bitfield layout for this sample. This is a deliberate
demo-profile departure from the initial plan's 565 strip. It presents through
`vrt_cpyfm(MD_REPLACE)` with VDI black/white colour indices: the pinned GEMlib
binding documents mono-to-colour expansion for all VDI versions. This can display
monochrome content on colour desktops without EdDI or a guessed native packing.
Actual installed-driver operation remains untested.

Redraw copies the authoritative saved bitmap through current AES visible regions.
All rendering/input runs on the AES thread. Timer sampling is bounded to 10 ms;
MiNTlib's recorded `clock()` implementation reads the 200 Hz TOS tick facility.
Mouse state is consumed immediately outside rendering; pointer capture persists
until release or loss of the top window. Window resize/full-size/iconification and
native-colour output are not part of this fixed-size sample.

The demo has a 1 MiB LVGL heap, a 30,400-byte mono surface and a 57,600-byte draw
strip, plus executable/static data, stack and allocation overhead. No full RGB
framebuffer is allocated on target. Build output contains its link map and compiler
assembly. `scripts/inspect_demo.py` checks project compiler instructions and the
linked external code ranges, excluding embedded image/font data in a.out .text.

## Desktop preview

Requires SDL2 development headers and pkg-config:

```sh
make -C components/lib-lvgl demo-host
components/lib-lvgl/build/host-demo/lvwidgets
```

This opens the original colour demo in an SDL window on Linux, with pointer input
and ASCII keyboard navigation. It uses the same upstream code and RGB888 draw
format. It does not emulate GEM or validate the A2560Me driver.

For a headless preview or repeatable interaction test:

```sh
python3 components/lib-lvgl/scripts/build_demo.py --host --sanitize
components/lib-lvgl/build/host-demo/lvwidgets --smoke-test /tmp/widgets.ppm
components/lib-lvgl/build/host-demo/lvwidgets --snapshot /tmp/widgets-colour.ppm
components/lib-lvgl/build/host-demo/lvwidgets --snapshot-mono /tmp/widgets-mono.ppm
```

The smoke test renders the initial scene, activates Analytics and Shop with pointer
press/release events, checks the active tab, and tears down. The mono snapshot uses
the same dither function as GEM, with conventional black/white colour mapping; it
is a host rendering, not a target capture.

## Results recorded for this change

* MiNT060 demo cross-linked; compiler assembly and linked runtime text pass the FPU
  instruction check. Target ABI checks pass at compile time.
* Host renderer and tab navigation pass ASan/UBSan and leak checking.
* Ordered dither phase, endpoints and partial-byte preservation pass host tests.
* Target launch, performance, mono raster polarity, overlap, input and repeated
  close cycles remain to be tested on the board.
