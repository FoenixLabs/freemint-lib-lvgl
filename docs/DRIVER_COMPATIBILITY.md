# Driver compatibility and open gate

No target session has been performed. Board identity, FPGA build, AES, EmuTOS,
FreeMiNT/XaAES, NVDI/board-driver versions, display mode and cache policy are unknown.

The implemented probe opens a virtual VDI workstation, records all 57 baseline
`vq_extnd(handle,1)` words, and permits only depth 1. It transforms an owned
standard one-plane MFDB with `vr_trnfm`, then presents a device MFDB through
`vro_cpyfm(S_ONLY)` clipped to current AES visible rectangles and client bounds.
It does not dereference a screen address or change palette/physical workstation.
Mono bit polarity is still a visual acceptance question.

Exact colour blocker: no verified, non-supervisor extension discovery path or
installed driver evidence is available. The probe deliberately does not issue
`vq_scrninfo` or guess packing from depth. Implement documented EdDI detection,
validate its version and full 272-word result, then independently demonstrate
RGB565 and 32-bit memory-MFDB raster copies before enabling those paths.
Portable conversion support alone is not a usable native presentation path.

EmuTOS blocker: available external MiNTlib/startup has not been launched with
EmuTOS AES. Separate output profiles are candidate builds, not runtime claims.

Binary inspection initially found real FPU helpers from printf's external libc
path. Diagnostics now use integer-only GEMDOS file logging. Final executable
inspection and compiler assembly inspection of LVGL avoid confusing font bitmap
bytes in a.out .text with instructions. This is an FPU check, not complete CPU
instruction or runtime certification. Review link maps/disassembly and library
provenance before target acceptance.
