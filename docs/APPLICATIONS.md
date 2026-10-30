# Single-surface application helper

Include `lvgl_app.h`; it includes LVGL and exposes no AES types. Initialization
creates one native GEM window. Build your widgets directly on its client surface
using `lg_app_root()`. There is no lv_win, nested title bar or LVGL closer.

Start with `examples/hello.c`:

```c
#include "lvgl_app.h"
int main(void)
{
    lg_app_t *app = lg_app_init("My application", 640, 480, LG_APP_RESIZABLE);
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

There is one application and one native surface per process. Supply the initial
client width and height and flags directly to `lg_app_init()`:

```c
lg_app_init("Fixed", 600, 400, 0);
lg_app_init("Resizable", 640, 480, LG_APP_RESIZABLE);
```

These are alternatives, not two simultaneously initialized apps. The compatibility
function `lg_app_init_size(title, width, height)` selects a fixed-size window.
Unknown flags and zero dimensions fail initialization; resizable windows require
at least 64x48 client pixels. The initial decorated window must fit the desktop.

GEM owns the title bar, movement, closer, size grip, visible rectangles and
exposure. LVGL owns only the content. With LG_APP_RESIZABLE, dragging the native
bottom-right size grip changes the client surface and LVGL display resolution.
Root SIZE_CHANGED and display RESOLUTION_CHANGED events follow the LVGL update;
percentage/flex/aligned layouts can adapt, while absolute coordinates remain the
application's responsibility. The Hello label stays centered after resizing.

Resize requests are clamped to a minimum64x48 client area and the desktop's outer
size. New backing/draw buffers are prepared before changing geometry. If allocation
or preparation fails, the old window size and buffers are retained. Existing
widgets and their state survive; cached scroll pixels are discarded and the
surface is redrawn. Both old and new buffers briefly coexist, so growth needs
additional memory. Full-size/maximize and iconification remain unsupported.
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

## Native AES menus

`lg_app_run()` installs a native AES menu bar. Its first title uses the executable
basename without spaces or its extension (`LVHELLO.APP` becomes `LVHELLO`).
The GEM backend reads MiNT’s process filename, falling back to the C runtime
invocation name and finally `LVGL` when no executable identity is available.
The dropdown contains the application name and a simple About alert; on
MultiTOS/XaAES the application also registers that name in the task list. File contains Quit, whose default callback calls
`lg_app_quit()`. This is independent of the native window close callback.

Add menus and items after initialization and before the first event loop:

```c
static void clear(lg_app_t *app, void *label)
{
    (void)app;
    lv_label_set_text(label, "");
}

/* After creating app and label: */
int edit = lg_menu_add(app, "Edit");
int clear_item = lg_menu_add_item(app, edit, "Clear", clear, label);
if (!edit || !clear_item) { /* inspect lg_app_error(), clean up, return */ }
```

Use `LG_MENU_FILE` to add items to File and `lg_menu_add_separator(app, menu)`
for a disabled separator. Menu IDs and item IDs are separate namespaces;
positive IDs indicate success, zero indicates failure. Labels are copied and
must contain 1–63 bytes with no control characters. The app name is truncated
to 63 bytes for the menu. The initial limits are eight menus including File
and 64 items including Quit and separators. The resulting native menu must fit
the screen; installation failure returns -1 from `lg_app_run()`.

Menu callbacks use the same `(app, user_data)` signature as close callbacks.
They run on the application thread and can update LVGL widgets. Call
`lg_app_quit()` to leave the loop, then clean up with `lg_app_close()` after run
returns. `lg_menu_on_item(app, item, callback, user_data)` can replace a callback
while running. To intercept Quit for saving or confirmation, replace the
callback for `LG_MENU_QUIT`; it must explicitly call `lg_app_quit()` to exit.
A NULL callback leaves an item without an action.

Menu structure is fixed once the event loop starts. Runtime insertion/removal,
keyboard shortcuts, submenus and checked items are not provided yet. Menus use
AES, without LVGL window objects. MCP remains unimplemented. The Hello template
shows Actions → Greet updating its label. `make menu-test MINTLIB=/absolute/path/to/mintlib`
checks the AES object tree with the pinned GEMlib declarations under ASan/UBSan;
`make app-test` also checks menu dispatch, copied labels and default Quit.

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
The newer init signature takes `(title, width, height, flags)`; replace former
one-argument calls with explicit dimensions and `0` or `LG_APP_RESIZABLE`.
