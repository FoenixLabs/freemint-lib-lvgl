# Acceptance record — 2026-10-29

| Check | Result |
|---|---|
| Exact dependency SHAs and clean upstream files | Passed locally |
| Normal-int target ABI and 20-byte MFDB assertions | Cross-compiled |
| Mono masks, odd widths 1/7/8/15/16/17/319/641, padding | Host ASan/UBSan passed |
| All RGB565 values to explicit 565/ARGB bytes | Host passed (replication within 1 of rounded scaling) |
| Geometry intersection and invalid size/depth rejection | Host passed |
| GEMlib private upstream build, 68060 soft float | Passed |
| LVGL static library, 68060 soft float | Passed |
| GEM raster and byte diagnostics link | Passed |
| EmuTOS launch / mono polarity / overlap | Untested |
| FreeMiNT/XaAES/NVDI mono/16/32 MFDBs | Untested; colour disabled |
| Upstream Widgets demo and fixed-size GEM adapter | Cross-built; target untested |
| Native preview rendering, pointer tab navigation, teardown | ASan/UBSan/leak check passed |
| Bayer dither phase/endpoints and partial-byte preservation | Host passed |
| Queue pressure, resize/failure recovery | Not implemented |
| Full 24-bit LVGL fidelity | Not implemented |
| Hardware caches, performance, twenty close cycles | Untested |

## Target procedure

1. Record board/revision, FPGA build, OS/AES/VDI/NVDI versions, mode dimensions,
   bit depth, and OS-managed cache policy. Hash both executables on the host.
2. Run LVBYTES.TTP. Expected 565 pairs: f8 0, 7 e0, 0 1f, ff ff.
   ARGB rows: ff ff 0 0; ff 0 ff 0; ff 0 0 ff; ff ff ff ff.
   LVGL native red565 must be f8 0 on the big-endian target.
3. In mono GEM launch LVPROBE.APP from a writable folder. Retain LVPROBE.TXT.
   Check the 319-pixel odd-width checkerboard for wrapping and polarity.
4. Cover/uncover while idle, move partly offscreen, top another app, close using
   Escape and closer, and repeat twenty times. Check desktop/cursor restoration.
5. In 16/32-bit modes confirm clean rejection; this is not colour acceptance.
   Implement/validate EdDI and colour MFDB probe before proceeding with integration.
6. Record pass/fail per scenario and retain captures and logs outside Git.
   After the backend exists add widget, drag-release, focus, resize, full-size,
   iconification, termination, allocation failure and background-load scenarios.

Superproject `python3 scripts/check.py` source checks passed up to its staging
verification, which failed because existing staging differs from frozen baseline.
This change does not modify staging or the default 58-file baseline build.

See [DEMO.md](DEMO.md) for sample executable and desktop preview commands.
