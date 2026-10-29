# lib-lvgl
Component sources belong here. Never patch dependency sources during builds.
Use the exact dependency gitlinks; never update remotes during builds.
Use AES/VDI only: no display registers, DMA, supervisor calls or cache changes.
Run `make test` and `make probe MINTLIB=/absolute/path/to/mintlib`.
Hardware gates require recorded target evidence; cross-linking is not validation.
