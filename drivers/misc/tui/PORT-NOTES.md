DRIVERS/MISC/TUI — port notes for the exynos7885 4.9 kernel
=========================================================

Provenance
----------
Copied from the live 4.4 A30s tree:

    /root/rom/crdroid16/kernel/samsung/exynos7885/drivers/misc/tui/

12 files, 987 lines (9 at the top level plus the 3-file
platforms/exynos7885/ subdirectory).  Modes normalised to 0644.

This is the *contemporaneous* generation of the driver, which is the right
one for a 4.9 base.  Worth recording because Samsung kept rewriting it
afterwards: the Galaxy A35 5G (SM-A356E) kernel of a much later vintage
still has drivers/misc/tui/ with the same file names, but a completely
different interface -- stui_open_touch() / stui_open_display() /
stui_close_touch() / stui_close_display() / stui_get_lcd_info() plus an
iwd_agent.c and a tuill_defs.h, replacing 4.4's stui_process_cmd() +
tui_hw_buffer ioctl surface.  So there is no "upstream 4.9 version" of this
driver to find; 4.4 is the only correct source for this port.

Live evidence that it is a real, load-bearing driver on the A30s
-------------------------------------------------------------
  * CONFIG_SAMSUNG_TUI=y in the running 4.4 config (running.config:1424),
    CONFIG_SAMSUNG_TUI_TEST explicitly off.
  * 27 stui* symbols in /proc/kallsyms, including get_stui_device (exported),
    stui_open, stui_release, stui_process_cmd, the seven exported
    stui_{inc,dec,get,set}_blank_ref / stui_{get,set,set_mask,clear_mask}
    accessors, stui_cancel_session, and the whole stui_*_video_space /
    stui_{prepare,finish}_tui / stui_i2c_protect HAL.

Why it was unbuildable before and is not any more
-------------------------------------------------
stui_inf.c:16 includes <tee_client_api.h> and the platform Makefile points
that at drivers/misc/tzdev/$(TEEGRIS_VERSION).0/include/tzdev.  The whole
drivers/misc/tzdev/ tree was absent from the 4.9 port; it landed in
cf3b1d01d919.  TUI is a pure consumer of it and needed nothing else.

And TUI's use of the TEE is the *standard* GlobalPlatform client contract,
not a private ABI: stui_inf.c:146-163 is TEEC_InitializeContext ->
TEEC_OpenSession -> TEEC_InvokeCommand(42) -> TEEC_CloseSession ->
TEEC_FinalizeContext, and the whole thing sits behind #ifdef
USE_TEE_CLIENT_API with a two-line `pr_err("not supported"); return -1;`
fallback.  Nothing in it depends on the Trustonic Kinibi blob.

Changes vs 4.4 — four, all mandatory
------------------------------------
 1. `Makefile`, the 7885 arm: the GlobalPlatform client-API gate.

      - ifneq ($(CONFIG_TEEGRIS_VERSION),2)
      + ifneq ($(CONFIG_TZDEV),)
      + ifneq ($(wildcard $(srctree)/drivers/misc/tzdev/3.0/Makefile),)

    4.4 gated -DUSE_TEE_CLIENT_API on "TEEGRIS_VERSION != 2", a proxy for
    "the 3.0 flavour is the one that gets built".  The proxy is false here
    and fails *silently*: CONFIG_TEEGRIS_VERSION's Kconfig default is 2
    (drivers/misc/tzdev/3.0/Kconfig:222), this port carries only the 3.0
    flavour, and the symbol is set by neither
    arch/arm64/configs/exynos7885-a30s_defconfig nor the built .config.  The
    4.4 test therefore evaluates false, the define is dropped, and
    stui_inf.c compiles its "not supported" stui_cancel_session() stub with
    no build diagnostic at all — you would only find out from a dmesg line
    that session cancel has been a no-op since boot.  Gate on the two facts
    that are actually true instead.  If CONFIG_TZDEV is off, the header
    include is dropped as well, so TUI still builds standalone and degrades
    exactly as 4.4 does when tzdev/ is missing.

    The -I is also made $(srctree)-absolute and pinned to 3.0 rather than
    $(CONFIG_TEEGRIS_VERSION).0, which is both correct for this tree (only
    3.0 exists) and valid from an O= out-of-tree build, which is the only
    way this tree is ever built.

 2. `platforms/exynos7885/Makefile`: added
    `-I$(srctree)/drivers/staging/android/ion`, needed by change 3.  The
    two existing -I lines were also made $(srctree)-absolute for the same
    out-of-tree reason.

 3. `platforms/exynos7885/stui_hal_display.c`: `ion_phys()` does not exist
    in this kernel's ION.

      - ion_phys(client, handle, (unsigned long *)&phys_addr, &dbuf->size);
      - if (!phys_addr)
      + if (stui_ion_phys(handle, &phys_addr, &dbuf->size))
              goto clean_share_dma;

    4.4's ion_phys() lived at drivers/staging/android/ion/ion.c:885 and
    dispatched to a per-heap ->phys callback declared at 4.4 ion.h:151.  The
    4.9-era ION rewrite dropped both halves: struct ion_heap_ops in this
    tree (drivers/staging/android/ion/ion_priv.h:297) is
    allocate / free / map_kernel / unmap_kernel / map_user / shrink, with no
    phys member, and neither ion.h nor ion_priv.h offers any other way to
    ask a buffer for a physical address.  `grep -rn ion_phys` over the
    ported ION returns nothing.

    Replaced with a local stui_ion_phys() that walks the buffer's sg_table
    and insists the entries form one physically contiguous run — which is
    precisely the condition 4.4's ion_phys() documented ("its output is only
    correct if a heap returns physically contiguous memory").  The walk is
    sound for this call site: the heap asked for is
    EXYNOS_ION_HEAP_VIDEO_STREAM_MASK, which
    drivers/staging/android/ion/exynos/exynos_ion.c:91 registers as
    ION_EXYNOS_HEAP_ID_VIDEO_STREAM and serves from ion_hpa_heap_allocate()
    (ion/exynos/ion_hpa_heap.c:45), which takes ION_HPA_DEFAULT_ORDER
    (order-4) high-order pages, sorts them by address, and installs
    buffer->sg_table unconditionally at ion_hpa_heap.c:103.  ion_share_dma_buf()
    has already run at the call site, so dmap_cnt is non-zero and the table
    is live.  A non-contiguous buffer returns -ENODEV, which makes
    stui_alloc_video_space() fail and stui_process_cmd() report
    STUI_RET_ERR_INTERNAL_ERROR rather than handing the secure world a
    plausible-looking but wrong address.

    NOTE: this reaches into ion_priv.h from outside ION, which is a
    layering violation and needs the extra -I from change 2.  The clean fix
    is to restore ion_phys() inside drivers/staging/android/ion/ (see the
    port report, section 5); it is deliberately NOT done here because that
    file is shared with six other agents and its only other consumer,
    drivers/misc/tzdev/3.0/ion_fd2phys.c, does not compile anyway.

 4. `platforms/exynos7885/stui_hal_touch.c`: stui_tsp_enter() /
    stui_tsp_exit() are unresolved symbols in this tree.

      - extern int stui_tsp_enter(void);
      - extern int stui_tsp_exit(void);
      + __attribute__((weak)) int stui_tsp_enter(void) { ... -ENODEV }
      + __attribute__((weak)) int stui_tsp_exit(void)  { ... -ENODEV }

    4.4 relied on the touchscreen driver to define them.  On this device
    that is drivers/input/touchscreen/zinitix/zinitix_zt75xx_ts.c:10515 and
    :10540 — the live config sets CONFIG_TOUCHSCREEN_ZINITIX_ZT75XX_TCLM=y,
    and the other four providers (melfas_mss100, sec_incell_ts, imagis
    ist4050, himax) are all "# ... is not set".  No zinitix driver exists in
    this tree yet and nothing else in it defines the pair, so the externs
    fail the final link and take the whole Image down.

    Weak definitions keep the driver linkable now and step aside later: the
    moment a real TSP port lands with strong symbols, ld binds to those and
    discards these.  They return -ENODEV rather than 0 on purpose.
    stui_i2c_protect() propagates a non-zero return, which sends
    stui_process_cmd() down clean_fb_prepare and reports
    STUI_RET_ERR_INTERNAL_ERROR.  Returning 0 would tell stui_process_cmd()
    that the touch controller had been handed to the secure world when it
    had not, and TUI would go on to protect the display and copy a
    framebuffer to userspace for a session that can never start.

 -Wall -Werror compliance (Makefile:392-397)
---------------------------------------------
One more change is a type correction rather than an API change.

 5. `stui_core.c:145`: `pr_err("... Unknown command %d\n", cmd)` where cmd is
    `unsigned int`.  Changed to %u, the correct conversion specifier for the
    type.  This tree builds with `-Wall -Werror` (Makefile:392-397) where 4.4
    did not, so a `-Wformat` diagnostic here would be fatal rather than
    cosmetic -- but I could NOT confirm that clang 24 actually emits one for
    this case.  Compilers are off-limits for this task, and a same-line
    survey of 3,462 already-built files under mm/, fs/, kernel/, net/ and
    drivers/{gpu/drm,media/platform}/exynos turned up zero instances of the
    pattern, which is equally consistent with upstream Linux being
    -Wformat-clean by now.  The change is correct either way and is a no-op at
    runtime.

The weak-stub file also gained an explicit `#include <linux/errno.h>` for
-ENODEV; it already reached printk.h transitively (it used pr_err) but not
errno.h.  Cosmetic, not an API change.

What was checked and turned out NOT to need a change
----------------------------------------------------
  * class_create(THIS_MODULE, STUI_DEV_NAME) — 2-arg in 4.9
    (include/linux/device.h).  4.4 is the same 2-arg form, so verbatim.
  * wake_lock_init(&tui_wakelock, WAKE_LOCK_SUSPEND, "TUI_WAKELOCK") —
    include/linux/wakelock.h:36, unchanged.
  * class_find_device(fb_class, NULL, NULL, _is_dev_ok) with the
    `const void *p` callback — include/linux/device.h:452, unchanged.  The
    file's own `#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,9,0)` shim
    already selects the right arm on 4.9.
  * dma_buf_put / dev_get_drvdata / alloc_chrdev_region / cdev_init /
    cdev_add / unregister_chrdev_region / device_destroy / class_destroy —
    all present unchanged.  dma_buf_put() is NULL-safe, which matters because
    stui_free_video_space() is only ever reached on paths where the dbuf is
    valid.
  * decon.h, DEFAULT_BPP, decon_drvdata[], decon_tui_protection() and
    decon_lcd all exist in the ported dpu_7885 (decon.h:58, :44, :1479 and
    decon_core.c:309) — the DECON/DPP/DSIM wave already landed them.
  * ion_client_create / ion_alloc / ion_share_dma_buf / ion_free /
    ion_client_destroy / exynos_ion_client_create /
    EXYNOS_ION_HEAP_VIDEO_STREAM_MASK — all present in this tree's ION.
  * CONFIG_SOC_EXYNOS3250-only code in stui_inf.c:196-233 and the
    CONFIG_SAMSUNG_TUI_TEST-only kmap() block in stui_core.c:180-198 are
    preprocessed out; the TEST block would also be fine on 4.9 anyway
    (kmap() is a static inline at include/linux/highmem.h:56).
  * stui_get_blank_ref / stui_set_blank_ref / stui_get_mode /
    stui_set_mode / stui_set_mask / stui_clear_mask are EXPORT_SYMBOL'd and
    consumed by the framebuffer and touchscreen drivers.  Those consumers
    are not in this tree yet, so the exports are currently unused — the
    symbols are still emitted, so nothing breaks either way.

Runtime reality check — what this driver is worth on this port
--------------------------------------------------------------
TUI is the "Trusted User Interface" path: it hands the secure world a
framebuffer carved out of the HPA video-stream heap, switches the DECON into
protected mode, and hands the touch controller's I2C bus over to the secure
world, so that a TEE application can draw on the real screen and read the
real touch controller without the normal world ever seeing the pixels or the
coordinates.  Samsung's own TEEGRIS documentation lists Trusted User
Interface as a first-class TEEGRIS capability alongside Trusted Storage, so
the concept is exactly what it says on the tin.

But the TEEGRIS secure kernel that services the session is Trustonic Kinibi,
and the blob is not in this tree (drivers/misc/tzdev/PORT-NOTES.md).
Without it, `/dev/tuihw` is created and every STUI_HW_IOCTL_START_TUI fails:
the weak TSP stub returns -ENODEV at the very first step, before any ION or
DECON work, and stui_process_cmd() reports STUI_RET_ERR_INTERNAL_ERROR.

That is the correct failure mode — a trusted-UI session can never start, and
the normal-world display and touch stack is left completely untouched.  It
is NOT worth enabling until the secure-world blob is available.  Enable
CONFIG_SAMSUNG_TUI only alongside CONFIG_TZDEV.
