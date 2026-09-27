# Exynos7885 (A30s) GPU port notes

Working notes for porting the A30s GPU onto this 4.9.219 Samsung tree.
Everything below was measured against the two local trees, not taken from
vendor prose.

Reference trees used for all measurements:

| role | path |
| --- | --- |
| 4.4 A30s kernel (read-only reference) | `/root/rom/crdroid16/kernel/samsung/exynos7885` |
| 4.9 port tree (this directory's parent) | `/root/port49/ksu-4.9` |

## 1. The A30s GPU

* 4.4 driver: `drivers/gpu/arm/b_r26p0` (ARM Mali **Midgard r26p0**).
  Selected by `CONFIG_MALI_TMIX` + `CONFIG_MALI_TMIX_R26P0`
  (`drivers/gpu/arm/Kbuild:12-16`, `drivers/gpu/arm/Kconfig:38-40`).
* 4.4 inventory: 324 files, 104090 source lines, 3.26 MB, **zero prebuilt
  blobs** (no `.o`/`.a`/`.ko`/`.so`/`.bin`, no ELF/MZ/PK magic anywhere).
  It is a full kbase source drop, so this is a source-port question, not a
  packaging question.

## 2. What this 4.9 tree actually contains

Three top-level generation trees, all ARM DDK drops:

| dir | files | src lines | bytes | Kconfig symbol | product line |
| --- | --- | --- | --- | --- | --- |
| `b_r19p0` | 268 | 90231 | 2809907 | `MALI_BIFROST_R19P0_Q` | Bifrost / Heimdall |
| `tHEx` (8 generations: r5p0 r7p0 r8p0 r9p0 r10p0 r12p0 b_r15p0 b_r16p0) | 2073 | 680408 | 20511250 | `MALI_THEX` | Bifrost / Heimdall |
| `tMIx` (r5p0) | 235 | 76427 | 2262812 | `MALI_TMIX` | **Midgard / mimir** |

Total 2576 files / 847100 src lines / 25.6 MB.

`drivers/gpu/arm/Kconfig` only ever sources:

* `tMIx/Kconfig` under `SOC_EXYNOS8895`
* `tHEx/Kconfig` under `SOC_EXYNOS9810`

`MALI_MIDGARD` is **not** a Midgard/architecture switch. It is byte-identical
boilerplate ("Mali Midgard series support") copy-pasted into the per-generation
Kconfig of *every* drop, including the Bifrost one
(`b_r19p0/Kconfig:23`, `tHEx/r9p0/Kconfig:24`, `tMIx/r5p0/Kconfig:17`, ...).
Treating its presence as evidence of Midgard support is a false positive.

## 3. Verdict: BLOCKED - do not port

Two independent blockers. Either one alone is fatal.

### 3.1 No Midgard r26p0 anywhere in this tree

The only Midgard drop present is `tMIx/r5p0` (mimir **r5p0**), a different
revision from the r26p0 the A30s needs, and it is behind a `SOC_EXYNOS8895`
gate that Exynos7885 does not satisfy.

r26p0 is also not an ARM-published DDK release. The public ARM Mali drop
(`android.googlesource.com/kernel/amlogic-tv-modules/mali-driver`) ships
`midgard/` = r11p0, r12p0, r13p0, r14p0, r15p0, r16p0, r21p0, r28p0. There is
no r26p0 and no 4.9-targeted Midgard drop. The 4.4 tree we already have is the
only source of this driver.

### 3.2 The ioctl ABI is a different paradigm, not a shifted number

| driver | GPU arch | `mali_kbase_ioctl.h` | `KBASE_IOCTL_*` | legacy `kbase_dispatch` | `KBASE_IOCTL_TYPE` |
| --- | --- | --- | --- | --- | --- |
| 4.4 `b_r26p0` | Midgard r26p0 (A30s silicon) | present | 46 | 0 refs - **modern only** | `0x80` |
| 4.9 `tMIx/r5p0` | Midgard r5p0 (right family) | **absent** | 0 | 10 refs - **legacy only** | n/a |
| 4.9 `b_r19p0` | Bifrost r19p0 (wrong arch) | present | 45 | 0 refs - modern only | `0x80` |

**Modern** (4.4 and `b_r19p0`) - the operation is selected by the ioctl number:

```c
/* 4.4 mali_kbase_core_linux.c:1529 */
static long __kbase_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
        switch (cmd) {
        case KBASE_IOCTL_VERSION_CHECK:
        case KBASE_IOCTL_SET_FLAGS:
        ...
```

with the numbers the 4.4-era UMD actually sends:
`VERSION_CHECK nr=0, SET_FLAGS nr=1, GET_GPUPROPS nr=3, MEM_ALLOC nr=5,
MEM_QUERY nr=6, MEM_FREE nr=7, HWCNT_READER_SETUP nr=8, HWCNT_ENABLE nr=9,
GET_DDK_VERSION nr=13, MEM_JIT_INIT nr=14, MEM_SYNC nr=15,
FIND_CPU_OFFSET nr=16, TLSTREAM_ACQUIRE nr=18, MEM_COMMIT nr=20,
MEM_ALIAS nr=21, MEM_IMPORT nr=22, MEM_FLAGS_CHANGE nr=23, ...` (46 total),
all `_IOWR(0x80, N, struct kbase_ioctl_*)`.

**Legacy** (`tMIx/r5p0`) - the ioctl number is never read at all:

```c
/* 4.9 tMIx/r5p0 mali_kbase_core_linux.c:1336 */
#define CALL_MAX_SIZE  (KBASE_FUNC_MAX - 1)

static long kbase_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
        u64 msg[(CALL_MAX_SIZE + 7) >> 3] = { 0xdeadbeefdeadbeefull };
        u32 size = _IOC_SIZE(cmd);          /* <-- cmd used ONLY for its size */
        ...
        if (kbase_dispatch(kctx, &msg, size) != 0)
```

The operation is instead taken from a `uk_header` field inside the copied
struct, numbered from `UK_FUNC_ID = 512` (`tMIx/r5p0/mali_uk.h:85`) - i.e.
ioctl "numbers" 512, 513, 514, ...

A 4.4-era UMD calling `ioctl(fd, _IOWR(0x80, 5, struct kbase_ioctl_mem_alloc), &arg)`
against `tMIx/r5p0` would have `cmd` ignored, would have a
`struct kbase_ioctl_mem_alloc` reinterpreted as a `struct kbase_uk_mem_alloc`,
and would dispatch on whatever sits where `uk_id` is expected. That is an
out-of-range function-pointer index, not a clean error.

The tail of the legacy enum also moved relative to 4.4's
`mali_kbase_uku.h:113-122`: 4.4 assigns
`SET_MIN_LOCK=41, UNSET_MIN_LOCK=42, STEP_UP_MAX_GPU_LIMIT=43,
RESTORE_MAX_GPU_LIMIT=44, SET_VK_BOOST_LOCK=45, UNSET_VK_BOOST_LOCK=46`, while
`tMIx/r5p0` renumbers `STEP_UP_MAX_GPU_LIMIT=60, RESTORE_MAX_GPU_LIMIT=61` and
drops the two `VK_BOOST_LOCK` entries entirely.

### Why the ABI does not save us

The only 4.9 drop with the modern `KBASE_IOCTL_*` ABI is `b_r19p0` - Bifrost
r19p0, a different GPU architecture with a different register map and job
descriptor format. Pointing it at Midgard r26p0 silicon would abort or corrupt
the display pipeline. A wrong GPU driver is strictly worse than no driver.

## 4. What to do instead

* Leave the A30s GPU out of this 4.9 port and keep the 4.4 kernel as the GPU
  provider. There is no in-tree path to a working Midgard r26p0 here.
* The `built-in.o` fix below keeps the tree buildable with no GPU driver, which
  is what lets the rest of the 7885 port make progress.
* If a GPU ever becomes non-negotiable, the only realistic route is the 4.4
  `b_r26p0` source forward-ported as its own subsystem together with a
  matching `libmali` UMD, on 4.4.302 - i.e. option P1, not P3.

## 5. `drivers/gpu/arm/built-in.o` link failure (fixed here)

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
defines those two when `SOC_EXYNOS8895` / `SOC_EXYNOS9810` is set.
`arch/arm64/configs/exynos7885-a30s_defconfig` sets `CONFIG_SOC_EXYNOS7885=y`
and explicitly unsets both of the others (lines 399-404), so for the A30s no
generation is selectable, this directory builds nothing, `builtin-target` is
never defined, `drivers/gpu/arm/built-in.o` is never created, and the link of
`drivers/gpu/built-in.o` fails with:

```
ld.bfd: cannot find drivers/gpu/arm/built-in.o: No such file or directory
```

Fix: when no generation is selected, add one placeholder object so the
aggregate always exists. A correctly configured build (any `MALI_TMIX` /
`MALI_THEX` generation selected) is unaffected and never compiles the stub.
