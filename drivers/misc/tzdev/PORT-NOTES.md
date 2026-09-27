TZDEV / TEEGRIS 3.0 — port notes for the exynos7885 4.9 kernel
============================================================

Provenance
----------
Copied verbatim from the live 4.4 A30s tree:

    /root/rom/crdroid16/kernel/samsung/exynos7885/drivers/misc/tzdev/3.0/

70 files, 8,970 lines.  Only `3.0/` was taken, NOT the sibling `tzdev/`
root, because `drivers/misc/Makefile` in the 4.4 tree selects the TEEGRIS
flavour:

    ifneq ($(CONFIG_TEEGRIS_VERSION),2)
    obj-$(CONFIG_TZDEV) += tzdev/$(CONFIG_TEEGRIS_VERSION).0/
    else
    obj-$(CONFIG_TZDEV) += tzdev/
    endif

and the live A30s config sets `CONFIG_TEEGRIS_VERSION=3`.  The `tzdev/`
root is the TEEGRIS 2.0 layout and is dead code on this device (7,836
lines that never compile).  It was deliberately not ported.

The 3.0 flavour is self-contained: it has its own `lib/` (circ_buf) and
`teec/` (GlobalPlatform client) subdirectories, and references no 2.0-only
file (`tz_transport.c`, `tz_telemetry.c`, `tz_wormhole.c`,
`tz_shmem_validator.c`, `tz_iwnotify.c` are all 2.0-only and absent).

Changes vs 4.4
--------------
Exactly two, both inside this directory:

 1. `3.0/sysdep.c`, `sysdep_vfs_getattr()`

      -       return vfs_getattr(&p, stat, STATX_SIZE, KSTAT_QUERY_FLAGS);
      + #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
      +       return vfs_getattr(&p, stat, STATX_SIZE, KSTAT_QUERY_FLAGS);
      + #else
      +       return vfs_getattr(&p, stat);
      + #endif

    4.9 declares `int vfs_getattr(struct path *, struct kstat *)` in
    `include/linux/fs.h:3003` and has no `KSTAT_QUERY_FLAGS` anywhere
    (`grep -rn KSTAT_QUERY_FLAGS include/` returns nothing).  The macro
    arrived with the 4.11 STATX rework.  Without this the port does not
    compile.  The function currently has no caller (`sysdep_vfs_getattr`
    is referenced only from its own definition and its prototype in
    `sysdep.h`) because `CONFIG_TZ_NWFS` is off, but it must still compile.

 2. `3.0/Kconfig`, `TZ_NWFS` default

    `default y` -> `default n`, with a comment.  Samsung's drop ships
    `tz_fsdev.h` but NOT `tz_fsdev.c`, while `3.0/Makefile` contains
    `obj-$(CONFIG_TZ_NWFS) += tz_fsdev.o`.  Leaving the upstream default
    alone breaks the build the instant `CONFIG_TZDEV=y` is set, because
    Kconfig resolves `CONFIG_TZ_NWFS=y` and asks for a nonexistent object.
    The live A30s config explicitly carries `# CONFIG_TZ_NWFS is not set`,
    so the device was never affected.

Everything else is byte-identical to 4.4 — verified with
`diff -ru --exclude=startup.tzar` over all 70 files.

The secure-world blob is deliberately NOT in this tree
------------------------------------------------------
`CONFIG_TZDEV_DEPLOY_TZAR=y` is what pulls the blob in, via
`3.0/tz_deploy_tzar.c:32-38`:

    __asm__ (
      ".section .init.data,\"aw\"\n"
      "tzdev_tzar_begin:\n"
      ".incbin \"" KBUILD_SRC "/drivers/misc/tzdev/3.0/startup.tzar\"\n"
      "tzdev_tzar_end:\n"
      ".previous\n"
    );

That is an assembler-level `.incbin`, i.e. the blob is embedded into
`.init.data` at COMPILE time.  So the build/run split is:

  * `CONFIG_TZDEV_DEPLOY_TZAR=n`  -> the blob is never referenced.  The
    shim compiles and links with no secure-world blob present at all.
  * `CONFIG_TZDEV_DEPLOY_TZAR=y`  -> `tz_deploy_tzar.o` will NOT assemble
    without the file.  GNU as and clang IAS both fail hard on a missing
    `.incbin` target; there is no graceful degradation.

Reference location and identity of the blob in the 4.4 tree:

    path   drivers/misc/tzdev/3.0/startup.tzar
    size   5,416,310 bytes
    git    tracked, blob 602d27d53ec7e2e75844304b80df46595a0606f6
           added by fc1ba4a0377d ("Eureka-Kernel R24U sources ...")

Note the spelling: the file is `startup.tzar` and `.incbin` also says
`startup.tzar`.  The Kconfig *help text* says `startup.tazar` (extra `a`);
that is cosmetic only and does not affect the build.

Do NOT commit the 5.4 MB blob.  Recommended handling, matching how this
project already treats the oversized A30s prebuilt APKs (gitignored, kept
as a tarball under /root/backup/):

  * keep it at
      /root/port49_prep/tzdev_blob/exynos7885-a30s/startup.tzar
  * and either
      - stage the shim with `CONFIG_TZDEV_DEPLOY_TZAR=n`, which is the
        only combination that builds without the blob, or
      - repoint `3.0/Makefile`'s
        `ccflags-$(CONFIG_TZDEV_DEPLOY_TZAR) += -D"KBUILD_SRC=..."`
        at that path, which keeps the blob outside the repo.

A second closed blob exists in the 4.4 tree and is likewise not ported:
`firmware/five/ffffffff000000000000000000000072.tlbin` (Samsung FIVE
trustlet, git-tracked).  Nothing in this directory references it.

Config symbols that must stay off (missing sources, same defect class)
------------------------------------------------------------------------
  * `CONFIG_TZ_NWFS`      -> needs tz_fsdev.c, absent (default fixed above)
  * `CONFIG_TZDEV_DEBUG`  -> `3.0/Makefile` ends with
                            `obj-$(CONFIG_TZDEV_DEBUG) += tests/` and
                            there is no `tests/` directory at all
  * `CONFIG_MSM_SCM`      -> pulls tz_msm_platform.c, which is
                            Qualcomm-only (`soc/qcom/scm.h`)

`TZDEV_CMA`, `TZLOG`, `TZTUI`, `TZDEV_HOTPLUG`, `TZ_SHMEM_VALIDATOR`,
`TZ_WORMHOLE`, `TZDEV_EARLY_SWD_INIT`, `TZ_TRANSPORT` are all `default n`
and all resolve to files that ARE present, so they are safe to enable on
request.  The live A30s config keeps them off.

Shared-memory / SMC ABI note for the runtime (not a build issue)
---------------------------------------------------------------
`tzdev_smc_check_version()` (tzdev.h:132) passes `LINUX_VERSION_CODE` to
the secure kernel on every init:

    tzdev_smc_cmd(TZDEV_SMC_CHECK_VERSION, LINUX_VERSION_CODE,
                  TZDEV_DRIVER_CODE, 0, 0, 0, 0)

`TZDEV_DRIVER_CODE` is unchanged (3.0.0) but `LINUX_VERSION_CODE` now
reports 4.9.219 instead of 4.4.302.  Whether the shipped TEEGRIS SK
accepts an unexpected normal-world version is a runtime question the
kernel cannot answer; if `tzdev_smc_sysconf()` or the IWI_PANIC interrupt
misbehaves after this port, this constant is the first thing to look at.
