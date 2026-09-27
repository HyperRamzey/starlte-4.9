# Exynos7885 (A30s) GPU port notes

Working notes for porting the A30s GPU onto this 4.9.219 Samsung tree.
Everything below was measured against the two local trees, not taken from
vendor prose.

Reference trees used for all measurements:

| role | path |
| --- | --- |
| 4.4 A30s kernel (read-only reference) | `/root/rom/crdroid16/kernel/samsung/exynos7885` |
| 4.9 port tree (this directory's parent) | `/root/port49/ksu-4.9` |

> **VERDICT REVERSED 2026-09-27. The "BLOCKED - do not port" verdict below is
> WRONG and is retracted.** Its section 3.1 rests on a factually false premise
> (it claims no public Midgard r26p0 DDK exists - one does), and its section
> 3.2 argues an ioctl-ABI mismatch against `tMIx/r5p0` that is irrelevant,
> because the r26p0 driver carries its own `mali_kbase_ioctl.h` and its own
> dispatch and never touches this tree's GPU framework. The measurements in
> sections 1, 2 and 5 are all still correct and were re-confirmed; section 4 is
> superseded. Read section 3A first.

## 1. The A30s GPU

* 4.4 driver: `drivers/gpu/arm/b_r26p0` (ARM Mali **r26p0**, "mimir" family -
  the DDK generation in which ARM merged Midgard and Bifrost into one kbase).
  Selected by `CONFIG_MALI_TMIX` + `CONFIG_MALI_TMIX_R26P0`
  (`drivers/gpu/arm/Kbuild:12-16`, `drivers/gpu/arm/Kconfig:38-40`).
* 4.4 inventory: **324 files, 104,089 source lines** (166 `.h` + 126 `.c` +
  4 `.bp` + Kbuild/Kconfig/Makefile/Mconfig), **zero prebuilt blobs** - no
  `.o`/`.a`/`.ko`/`.so`/`.bin`, no ELF/MZ/PK magic anywhere. It is a full kbase
  source drop, so this is a source-port question, not a packaging question.
  Re-measured: `find . -name '*.c' -o -name '*.h' | xargs wc -l` = 104,089.
* Size split, which turns out to be the whole story:

  | part | files | lines | share |
  | --- | --- | --- | --- |
  | ARM kbase core | 256 | 94,657 | 91% |
  | `platform/exynos` Samsung glue | 44 | 9,432 | 9% |

## 2. What this 4.9 tree actually contains

Three top-level generation trees, all ARM DDK drops:

| dir | files | src lines | bytes | Kconfig symbol | product line |
| --- | --- | --- | --- | --- | --- |
| `b_r19p0` | 268 | 90231 | 2809907 | `MALI_BIFROST_R19P0_Q` | Bifrost / Heimdall |
| `tHEx` (8 generations: r5p0 r7p0 r8p0 r9p0 r10p0 r12p0 b_r15p0 b_r16p0) | 2073 | 680408 | 20511250 | `MALI_THEX` | Bifrost / Heimdall |
| `tMIx` (r5p0) | 235 | 76427 | 2262812 | `MALI_TMIX` | mimir r5p0 |

Total 2576 files / 847100 src lines / 25.6 MB.

`MALI_MIDGARD` is **not** a Midgard/architecture switch. It is byte-identical
boilerplate ("Mali Midgard series support") copy-pasted into the per-generation
Kconfig of *every* drop, including the Bifrost ones
(`b_r19p0/Kconfig:23`, `tHEx/r9p0/Kconfig:24`, `b_r26p0/Kconfig:23`, ...).
Treating its presence as evidence of anything is a false positive. It is also
duplicated across per-SoC-gated Kconfig files all over this tree, which is safe
only because those gates are mutually exclusive.

## 3. ~~Verdict: BLOCKED - do not port~~ **RETRACTED, see 3A**

*(the two "independent blockers" claimed here were both wrong; kept only so the
retraction is auditable)*

**3.1 "No Midgard r26p0 anywhere in this tree / not an ARM-published DDK" -
FALSE.** ARM *did* publicly ship a Midgard r26p0 DDK, under ARM's own
`TX011-SW-99002-rXXp0-YYrelZ` naming. It is mirrored at
`LibreELEC/mali-midgard`, branch `TX011-SW-99002-r26p0-01rel0` (siblings exist
for r27p0 and r28p0). So the "find it open-sourced on the web" question is
moot: we already have it in full as source, and it is public anyway.

**3.2 "The ioctl ABI is a different paradigm" - TRUE BUT IRRELEVANT.** The
table is accurate: 4.4 `b_r26p0` has 46 `KBASE_IOCTL_*` defines and
`KBASE_IOCTL_TYPE 0x80` with a plain `switch (cmd)`, while `tMIx/r5p0` has no
`mali_kbase_ioctl.h` at all and dispatches on a `uk_header.uk_id` field
numbered from `UK_FUNC_ID = 512`. But the conclusion drawn from it is wrong:
`b_r26p0` is a **self-contained** kbase. It ships its own
`mali_kbase_ioctl.h`, its own `__kbase_ioctl()` `switch`, its own MMU, its own
job manager. It links against **nothing** in `tMIx/`, `tHEx/` or `b_r19p0/`.
Nobody has to make two DDK generations agree with each other. The only real
question is whether 4.4's r26p0 source compiles against 4.9 headers - which is
ordinary source porting, and the answer is in 3A.

## 3A. VERDICT: PORT IT. Kernel-API drift is 3 items, not "enormous".

### 3A.1 The driver is a *modern* DDK that already targets 4.9

The single most important measurement. `b_r26p0` contains **211
`LINUX_VERSION_CODE` sites across 49 files**, and the thresholds they test span
**3.1 through 5.6**:

| threshold | sites in 4.4 `b_r26p0` | sites in 4.9 tree `b_r19p0` |
| --- | --- | --- |
| `< 4.10.0` | 33 | 31 |
| `>= 4.10.0` | 7 | 8 |
| `< 4.11.0` | 7 | 7 |
| `>= 4.11.0` | 4 | 4 |
| `< 4.9.0` | 3 | 5 |
| `>= 4.17.0` | 1 | 1 |
| `>= 4.4.28` | 1 | 1 |
| `>= 4.7.0` | 1 | 1 |
| `< 4.14.0` | 1 | 1 |
| `>= 5.5.0` / `>= 5.6.0` | 2 / 3 | 0 / 0 |

This is **not** a 4.4-era DDK back-compat shim; it is a 2019-2020 ARM DDK
(copyright headers read `2011-2015, 2017, 2019-2020 ARM Limited`) that was
backported *down* to 4.4 by Samsung. It carries 33 explicit `< 4.10.0` branches,
so 4.9 is a first-class, deliberately supported configuration of this drop.
And the conditionals are near-identical to the ones in the 4.9 tree's own
`b_r19p0` - same DDK generation, already building on this kernel.

### 3A.2 The drift count

Method: extract every identifier the driver calls (2,836 raw -> **855
external**), subtract what the driver defines itself, then intersect with the
union of identifiers in the 4.4 tree's `include/ arch/arm64/include/ fs/ mm/
kernel/ drivers/` (1,080,733 words) and the same for the 4.9 tree (1,113,423
words). Anything the driver calls, that 4.4 provides, that 4.9 does not, is
drift.

> **855 external symbols called; 4.4 provides all 855; 4.9 fails to provide 35;
> of those 35, 32 are driver-internal, 1 is version-gated, and 2 are a single
> removed upstream header. NET BREAKS = 3.**

| # | area | 4.4 usage | 4.9 requirement | files affected | count |
| --- | --- | --- | --- | --- | --- |
| 1 | `sysfs_emit()` | present via **Samsung backport** `include/linux/sysfs.h:304` | **absent** (upstream v4.10) | `mali_kbase_core_linux.c:4409,4424`; `context/mali_kbase_context.c:51,62` | 1 symbol / 4 sites |
| 2 | `linux/sysfs_helpers.h` | present (`include/linux/`), supplies `read_into()` + `sanitize_min_max()` | **header removed upstream in v4.9** | `platform/exynos/gpu_custom_interface.c:22,334,353,360` | 1 header / 2 symbols |
| 3 | `linux/exynos-busmon.h` | present (Samsung, `CONFIG_EXYNOS_BUSMONITOR`) | absent (not yet ported; lives under `drivers/` in this tree) | `platform/exynos/gpu_notifier.c:36,486` | 1 header |
| | | | | **TOTAL** | **3** |

### 3A.3 Everything the brief told me to fear is measurably clean

All present and signature-identical in 4.4 **and** 4.9: `get_user_pages`
(the `pin_user_pages` split is 5.8/5.9 - 4.9 still has the old 5-arg form the
driver calls), `remap_pfn_range`, `vm_operations_struct`, `vm_fault`,
`vm_page_prot`, `pgprot_noncached`, `pgprot_device`, `vm_insert_pfn_prot`,
`mm_context_t`, `get_task_mm`, `do_mmap`, `tlb_gather_mmu`, `kthread_run`,
`wait_event_interruptible_timeout`, `wait_event_killable`, `single_open`,
`show_fdinfo`, `idr_alloc`/`idr_for_each`/`idr_destroy`,
`percpu_counter_init`/`_add`, `pm_runtime_get_sync`/`_put`/`_set_active`,
`clk_prepare_enable`, `clk_disable_unprepare`, `regulator_enable`/`_get`,
`debugfs_create_file`/`_dir`, `alloc_pages`, `__get_free_pages`, `vzalloc`,
`vmalloc`, `kzalloc`, `smp_call_function_single`, `EXPORT_SYMBOL[_GPL]`,
`hrtimer_*`, `dma_alloc_attrs`/`dma_free_attrs`.

Absent in *both* trees, i.e. already version-gated by the DDK and dead on 4.9:
`pin_user_pages`, `unpin_user_pages`, `seq_read_iter`, `vmf_insert`,
`vm_account_error`, `set_page_prot`, `dma_map_attrs`, `get_user_pages_long`,
`ktime_get_raw_ts64` (guarded by `#if (4,17,0) > LINUX_VERSION_CODE`),
`vm_flags_set` (5.13), and the `u64_t`/`u32_t` typedefs (5.18).

### 3A.4 4.9 is the *easy* target - independent corroboration

`register_shrinker()` in this tree is **one-arg** (`include/linux/shrinker.h:70`),
`vm_flags_set` has **0** hits, `add_mm_counter` is present, `get_user_pages` has
the pre-5.8 signature. Every adaptation needed to build this DDK on a modern
kernel is therefore a *post-4.9* change and does not apply here:

| upstream fix needed for 6.5 | lands in | needed on 4.9? |
| --- | --- | --- |
| `register_shrinker(x, "name")` | 5.4 | no |
| `vma->vm_flags \|=` -> `vm_flags_set()` | 5.13 | no |
| `mm->rss_stat.count[]` -> `percpu_counter_add()` | 5.5 | no |
| `get_user_pages()` arg list | 5.8/5.9 | no |
| `MA_STATE()` maple tree in `mali_kbase_mmap.c` | 6.1 | no |
| `devm_request_irq`, `platform_get_irq` | stylistic | no |

The DDK's *unmodified* forms are the correct ones for 4.9. A known-good
midgard-kbase-on-6.5 patchset is 8 files / 47 insertions / 126 deletions, and
none of it touches 4.9.

### 3A.5 A trap specific to this tree: the fence compat header

`mali_kbase_fence_defs.h` carries a Samsung `MALI_SEC_INTEGRATION` hack that
**comments out** the upstream `>= 4.9.68` branch and forces `>= 4.10.0`, with
the note *"Should check status in LT(4.9) otherwise fence timeout occur
frequently"*. The 4.9 tree's own `b_r19p0` copy has the upstream line active.

**For this port the Samsung form is the correct one and must be kept.** This
4.9.219 tree is based on a 4.9 that predates the 4.9.68 sync-fence rework: it
has `include/linux/fence.h` with `struct fence::status`, and has **no**
`include/linux/dma-fence.h` and no `dma_fence_is_signaled`. Taking the upstream
`>= 4.9.68` branch would select `(a)->error`, which does not exist here. Do not
"fix" this to match `b_r19p0`. (Relatedly, `b_r19p0` is never compiled for
Exynos7885 - it is gated on `SOC_EXYNOS9810` - so its fence path is untested in
this tree and is not a usable reference.)

## 3B. What to do instead ~~(retracted)~~

Retracted. The port is in the tree, behind `CONFIG_MALI_TMIX_R26P0`, which
defaults to `n` so it cannot affect the in-flight build. See section 6 of the
port report for what is still needed to switch it on.

## 4. ~~Alternative donors~~ **none needed, but recorded**

| candidate | what it is | DDK revision | ioctl ABI | kernel versions | verdict |
| --- | --- | --- | --- | --- | --- |
| **mainline `drm/panfrost`** | the only mainline ARM Mali driver (Linux 5.15+, Rob Herring / Tomeu Vizoso / Alyssa Rosenzweig) | not a kbase at all - a native reimplementation | **none**; speaks DRM + Mesa Panfrost, not `KBASE_IOCTL_*` | 5.15+ | **unusable.** Different userspace entirely; the A30s' 4.4 UMD would find no driver. Upstream rejected kbase precisely because it has no DRM ABI. |
| LibreELEC/`5schatten` mali-midgard `r26p0` | ARM `TX011-SW-99002-r26p0-01rel0`, public | Midgard r26p0 | modern `KBASE_IOCTL_*` | 4.9 - 5.16 per maintainer | **same DDK revision as what we already own.** Useful only as a cross-check / for the missing `platform/` glue. |
| this tree's `b_r19p0` | Samsung Bifrost r19p0 | r19p0 | modern, 45 defines | already builds on 4.9 here | wrong silicon; already present |
| this tree's `tMIx/r5p0` | Samsung mimir r5p0 | r5p0 | legacy `UK_FUNC_ID` | builds on 4.9 here | wrong revision + wrong ABI paradigm |

**Mainline is a dead end and nobody should check it again:** the reason kbase
was never upstreamed is that it does not implement the DRM ABI, so the
mainline replacement is a different driver with a different userspace
interface. There is no mainline kbase.

## 5. `drivers/gpu/arm/built-in.o` link failure (fixed here, unchanged)

`drivers/gpu/Makefile:5` descends into this directory unconditionally
(`obj-y += drm/ vga/ arm/ exynos/`). `scripts/Makefile.lib:42` rewrites every
`obj-y` entry of the form `dir/` into a hard `dir/built-in.o` prerequisite, and
`scripts/Makefile.build:87-89` only defines `builtin-target` when the directory
has at least one object:

```make
ifneq ($(strip $(obj-y) $(obj-m) $(obj-) $(subdir-m) $(lib-target)),)
builtin-target := $(obj)/built-in.o
endif
```

Every `obj-y += <generation>/` line in this directory's `Kbuild` is guarded by
`CONFIG_MALI_TMIX` or `CONFIG_MALI_THEX`, and `drivers/gpu/arm/Kconfig` only
defined those two when `SOC_EXYNOS8895` / `SOC_EXYNOS9810` is set.
`arch/arm64/configs/exynos7885-a30s_defconfig` sets `CONFIG_SOC_EXYNOS7885=y`
and explicitly unsets both of the others, so for the A30s no generation is
selectable, this directory builds nothing, `builtin-target` is never defined,
`drivers/gpu/arm/built-in.o` is never created, and the link of
`drivers/gpu/built-in.o` fails with:

```
ld.bfd: cannot find drivers/gpu/arm/built-in.o: No such file or directory
```

Fix: when no generation is selected, add one placeholder object so the
aggregate always exists. A correctly configured build (any `MALI_TMIX` /
`MALI_THEX` generation selected) is unaffected and never compiles the stub.
The guard keys off the same `$(obj-y)` list that `MALI_TMIX_R26P0` now feeds, so
it keeps working unchanged.

## 6. How this port is layered

`b_r26p0` dropped in as source, wired behind a new `MALI_TMIX_R26P0` choice arm
that reuses the 4.4 tree's own symbol names, so the A30s defconfig lines port
across verbatim. Default `n`: the directory is never descended, so the stub
guard still supplies `built-in.o` and the running build is byte-for-byte
unaffected.

Adaptations actually applied (see `hm_mod/port49_gpu/patch_r26p0_49.py`, which
is idempotent and re-runnable):

1. `mali_kbase_defs.h` - `sysfs_emit()` -> `sprintf()` shim under
   `< KERNEL_VERSION(4,10,0)`. Placed here because all 120 includers get it.
2. `platform/exynos/sysfs_helpers.h` - **new file**, `read_into()` +
   `sanitize_min_max()` vendored verbatim out of the removed
   `include/linux/sysfs_helpers.h`. `sanitize_min_max` must stay a mutating
   macro: its call sites pass an array element and expect it clamped in place.
3. `platform/exynos/gpu_notifier.c` - `linux/exynos-busmon.h` gated on
   `__has_include` so the port stands alone regardless of whether the busmon
   driver gets ported.
4. `Kbuild` / `Kconfig` - wiring only, additive, default off.

## 7. Still needed before `CONFIG_MALI_TMIX_R26P0=y` can build

These are the known-unknowns a compiler would have caught. Everything above is
*statically verified*; the rest needs an actual build.

* `CONFIG_MALI_DMA_FENCE=y` needs checking: both this 4.9 tree and the 4.4 tree
  have the **old** `fence_*` API and no `dma-fence.h`, and kbase's compat header
  aliases them. The 4.4 defconfig sets it, so it should be settable here too,
  but the `sync_file.h` include that pairs with it is `>= 4.9.68`-gated and
  this tree does have `linux/sync_file.h` - untested combination.
* `soc/samsung/{bts,exynos-pd,cal-if}.h` are included by the Exynos glue (4 + 3
  + 2 sites). They are not at those paths in **either** tree, so they resolve
  through an include path that the 4.9 tree must also provide.
* `linux/ems.h` (`CONFIG_SCHED_EMS`) and `linux/bus_logger.h`
  (`CONFIG_MALI_BUSLOG`) must stay **off**; both headers exist in neither tree.
* `b_r26p0/tests/` (kutf) has its own Kbuild/Kconfig and references
  `../mali_kutf_clk_rate_trace_test.h`; keep `CONFIG_MALI_KUTF` off until
  checked.
* The A30s DT uses `compatible = "arm,mali"` (generic), while the driver's
  `of_match` table lists `arm,mali-midgard` and `arm,mali-bifrost` only - so a
  DT match may need adding to the 4.9 `exynos7885-mali_common.dtsi` port, or
  the generic string added to `of_match`.
* Zero-error link is unproven. Per section 15.9 of the project notes, the two
  worst G1 bugs were silent-until-link (missing `lib/string.c:stpcpy`,
  clang-dropped FIPS `.init.text` anchors). `nm`-check suspicious objects rather
  than waiting.
