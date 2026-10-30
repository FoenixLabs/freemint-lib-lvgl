# Single-surface application helper

Include `lvgl_app.h`; it includes LVGL and exposes no AES types. Initialization
creates one native GEM window. Build your widgets directly on its client surface
using `lg_app_root()`. There is no lv_win, nested title bar or LVGL closer.

Start with `examples/hello.c`:

```c
#include "lvgl_app.h"
int main(void)
{
    lg_app_t *app = lg_app_init("My application");
    if (!app) return 1;
    lg_app_on_close(app, lg_app_quit_callback, NULL);

    lv_obj_t *label = lv_label_create(lg_app_root(app));
    lv_label_set_text(label, "Hello from LVGL!");
    lv_obj_center(label);

    int result = lg_app_run(app);
    lg_app_close(app);
    return result;
}
```

## Build

From the system workspace:

```sh
make -C components/lib-lvgl examples MINTLIB="$PWD/components/mintlib"
make -C components/lib-lvgl app-test
```

Copy `components/lib-lvgl/build/mint060/hello-gem/LVHELLO.APP` to the target.
Edit the Hello template and rebuild with the examples target. The Widgets demo
also uses this API; build it with `make demo MINTLIB=/absolute/path/to/mintlib`.

The target example directory contains `liblvgl-app.a` (helper/backend),
`libwidgets.a` (pinned LVGL with the supplied configuration), a link map and
inspected compiler assembly. Use the build script's configuration and flags
consistently; the earlier reduced-widget `liblvgl.a` is not interchangeable.
The native blend adapter links with the renamed pinned LVGL blend definitions,
as the build script arranges. Dependency files remain unmodified.

## Surface and lifecycle

There is one application and one native surface per process. GEM creates a fixed
600x400 client area; `lg_app_init_size()` accepts explicit dimensions instead.
GEM owns the title bar, movement, closer, visible rectangles and exposure. LVGL
owns only the content. Native resize/full-size/iconification remain unsupported.
The helper does not create any LVGL window objects; LV_USE_WIN is disabled.

The native closer and GEM AP_TERM deliver close requests through
`lg_app_on_close()`. A missing callback leaves the application open. Use
`lg_app_quit_callback`, or a custom handler that saves data, refuses closure or
calls `lg_app_quit()`. Recursive close requests are ignored. Escape is passed to
LVGL as LV_KEY_ESC and never automatically quits. An OS can still forcibly
terminate a process independently of this library.

`lg_app_run()` returns after an explicit quit request and the current backend
iteration. Call `lg_app_close()` afterward to release the surface, its LVGL content
and platform resources. Do not call close inside callbacks. All calls belong on
the event-loop thread. A second initialized application is rejected. Errors are
available through `lg_app_error()`.

F6/F7/F8 remain Widgets-demo controls only. Ordinary applications do not inherit
its Profile cache, benchmark or reduced-effects shortcuts.

## Backends

`src/platform.h` is the private boundary. Backends initialize LVGL, the display,
clock and inputs; provide the surface root; pump a bounded iteration; report close
requests; and release resources. Common code owns the application lifecycle.

- GEM is implemented using AES/VDI, the validated native colour gate, an owned
  backing MFDB and clipped presentation.
- MCP remains an empty stub. Initialization returns NULL with a clear error and
  never accesses hardware. A future implementation can provide a full-screen
  surface and poll devices without a window manager or OS event wait.

Backend selection is at build time (`--platform gem`, the default).
`--platform mcp` only cross-links a MiNT stub for development; it is not an MCP
hardware port or executable format. No NetBSD/wsdisplay or SDL application backend
is supplied. The existing SDL Widgets preview remains a separate development aid.

`make app-test` uses a fake headless backend under ASan/UBSan to test the direct
surface root, dimensions, initialization/retry, close refusal/reentrancy, explicit
quit and cleanup, and separately checks the MCP failure path.

## Migration from the multiwindow experiment

The `lg_window_*` API and the two-window example were removed. Replace window
creation/content lookup with `lg_app_root(app)` and register the close handler
with `lg_app_on_close()`. `lg_app_quit_callback` now takes an application handle.
Initialization and event-loop entry remain `lg_app_init()` and `lg_app_run()`.
