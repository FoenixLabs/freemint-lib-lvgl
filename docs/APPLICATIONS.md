# Portable application helper

Include `lvgl_app.h`; it includes LVGL and exposes no AES types. Start with
`examples/hello.c`, or `examples/windows.c` for independently closable windows.

```c
#include "lvgl_app.h"
int main(void)
{
    lg_app_t *app = lg_app_init("My application");
    if (!app) return 1;
    lg_window_t *window = lg_window_create(app, "Main");
    if (!window) { lg_app_close(app); return 1; }
    lg_window_on_close(window, lg_app_quit_callback, app);
    lv_obj_t *label = lv_label_create(lg_window_content(window));
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

Copy `build/mint060/hello-gem/LVHELLO.APP` or
`build/mint060/windows-gem/LVWIN.APP` from the component to the target.
Edit the Hello template and rebuild with:

```sh
python3 components/lib-lvgl/scripts/build_demo.py --example hello \
    --mintlib "$PWD/components/mintlib"
```

Each target example directory also contains `liblvgl-app.a` (helper/backend),
`libwidgets.a` (the pinned LVGL archive using the supplied configuration), a link
map and inspected compiler assembly. Use the build script's configuration and
flags consistently; the earlier reduced-widget `liblvgl.a` configuration does
not enable `lv_win` and is not interchangeable. No dependency files are patched.
The standalone helper must link its native blend adapter with the renamed pinned
LVGL blend definitions, as the build script does.

## Windows and lifecycle

There is one application and one default rendering surface per process. GEM
creates a fixed 600x400 native host window; `lg_app_init_size()` accepts explicit
client dimensions instead. Inside it, `lg_window_create()` creates an LVGL window.
The first fills the surface; subsequent windows default to a smaller overlapping
rectangle. `lg_window_set_geometry()` sets their position and size. Drag a title
bar to move a window; clicking a window or its descendants raises and activates
it. Interactive resize, minimize and modal-window policy are not provided.

The LVGL closer and GEM native closer deliver close requests. GEM AP_TERM is also
a request, not an unconditional exit. Native requests go to the active managed
window. A missing callback leaves the window/application open. Closing the last
window does not stop the event loop. Escape is delivered to LVGL as LV_KEY_ESC;
it is not an application exit shortcut. An OS may still forcibly terminate a
process independently of this library.

Use `lg_app_quit_callback` to request application shutdown, or
`lg_window_destroy_callback` to destroy only that window. Custom callbacks may
refuse closure, save data, or explicitly call `lg_app_quit()`. Recursive close
requests on the same window are ignored. The event loop returns after the current
backend iteration; it does not destroy resources during callback dispatch.

Call `lg_app_close()` after `lg_app_run()` returns. Do not call it inside any LVGL
callback. All calls belong on the event-loop thread. Destroyed window handles and
their content objects must not be reused. App cleanup destroys remaining windows.
A second initialized application is rejected. Initialization errors are available
through `lg_app_error()`; MCP reports that it is not implemented.

For a full-surface upstream demo, use `lg_app_root()` and register an application
close callback with `lg_app_on_close()`. That callback is used only when no managed
window exists. The Widgets demo follows this path to preserve its original layout
and Profile-cache assumptions. F6/F7/F8 remain Widgets-only controls; they do not
intercept keys or enable caching in ordinary applications.

## Backends

`src/platform.h` is the private boundary. A backend initializes LVGL, the display,
clock and inputs; provides the surface root; pumps a bounded event-loop iteration;
and releases resources. It reports native close requests through a callback and
never decides to quit. Common code manages LVGL windows and application lifetime.

- `platform_gem.c`: implemented through the AES/VDI adapter, validated native colour
  gate, owned backing MFDB and clipped presentation. Application code uses no AES.
- `platform_mcp.c`: empty stub. Initialization returns NULL without touching hardware.
  A future implementation can use a full-screen display root and poll devices in
  `step()` without requiring threads, a window manager or an OS event wait.

Backend selection is at build time (`--platform gem`, the default). `--platform mcp`
currently only cross-links a MiNT stub for development; it is not an MCP executable
format or hardware port. No NetBSD/wsdisplay or SDL application backend is supplied.
The existing SDL Widgets preview remains a separate development aid.

`make app-test` runs ASan/UBSan lifecycle tests with a fake headless backend and
checks the MCP failure path. Tests cover active-window routing from descendant
clicks, refused/reentrant close requests, callback destruction, survival after the
last window closes, explicit quit, cleanup and reinitialization.
