/*
 * Samsung TUI HW Handler driver. Display functions.
 *
 * Copyright (c) 2015-2018 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "decon.h"
#include "stui_core.h"
#include "stui_hal.h"

#include <linux/dma-buf.h>
#include <linux/exynos_ion.h>
#include <linux/fb.h>
#include <linux/ion.h>
#include <linux/scatterlist.h>
#include <linux/version.h>

#include "ion_priv.h"

static struct ion_client *client;
static struct ion_handle *handle;
static struct dma_buf *dbuf;

static struct fb_info *get_fb_info_for_tui(struct device *fb_dev);

/* Framebuffer device driver identification
 * RETURN: 0 - Wrong device driver
 *         1 - Suitable device driver
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 9, 0)
static int _is_dev_ok(struct device *fb_dev, void *p)
#else
static int _is_dev_ok(struct device *fb_dev, const void *p)
#endif
{
	struct fb_info *fb_info;

	fb_info = get_fb_info_for_tui(fb_dev);
	if (!fb_info)
		return 0;

	return 1;
}

/* Find suitable framebuffer device driver */
static struct device *get_fb_dev_for_tui(void)
{
	struct device *fb_dev;
	/* get the first framebuffer device */
	fb_dev = class_find_device(fb_class, NULL, NULL, _is_dev_ok);
	if (!fb_dev)
		pr_err("[STUI] class_find_device failed\n");

	return fb_dev;
}

/* Get framebuffer's internal data */
static struct fb_info *get_fb_info_for_tui(struct device *fb_dev)
{
	struct fb_info *fb_item;

	if (!fb_dev || !fb_dev->p) {
		pr_err("[STUI] framebuffer device has no private data\n");
		return NULL;
	}
	fb_item = (struct fb_info *)dev_get_drvdata(fb_dev);
	if (!fb_item)
		pr_err("[STUI] dev_get_drvdata failed\n");

	return fb_item;
}

static int fb_protection_for_tui(bool tui_en)
{
	struct device *fb_dev;
	struct fb_info *fb_info;
	int ret;

	pr_debug("[STUI] %s : state %d start\n", __func__, tui_en);

	fb_dev = get_fb_dev_for_tui();
	if (!fb_dev)
		return -1;

	fb_info = get_fb_info_for_tui(fb_dev);
	if (!fb_info)
		return -1;

	ret = decon_tui_protection(tui_en);

	pr_debug("[STUI] %s : state %d end\n", __func__, tui_en);

	return ret;
}

void stui_free_video_space(void)
{
	dma_buf_put(dbuf);
	ion_free(client, handle);
	ion_client_destroy(client);
}

/*
 * PORT-NOTE(tui): recover the physical address of an ION handle.
 *
 * 4.4 called ion_phys() (4.4 drivers/staging/android/ion/ion.c:885), which
 * dispatched to a per-heap ->phys callback declared at 4.4 ion.h:151.  The
 * 4.9-era ION rewrite dropped both halves: struct ion_heap_ops in this tree
 * (drivers/staging/android/ion/ion_priv.h:297) is
 * allocate/free/map_kernel/unmap_kernel/map_user/shrink with no phys member,
 * and neither ion.h nor ion_priv.h offers any other way to ask a buffer for
 * a physical address.  grep -rn "ion_phys" over the ported ION returns
 * nothing.
 *
 * The heap this driver allocates from is still physically contiguous.  It
 * asks for EXYNOS_ION_HEAP_VIDEO_STREAM_MASK, which
 * drivers/staging/android/ion/exynos/exynos_ion.c:91 registers as
 * ION_EXYNOS_HEAP_ID_VIDEO_STREAM and serves from ion_hpa_heap_allocate()
 * (ion/exynos/ion_hpa_heap.c:45).  That heap takes ION_HPA_DEFAULT_ORDER
 * (order-4) high-order pages, sorts them by address and publishes them
 * through an sg_table that it installs unconditionally at ion_hpa_heap.c:103.
 * ion_share_dma_buf() has already run by the time we get here, so the
 * buffer's dmap_cnt is non-zero and buffer->sg_table is live.
 *
 * So walk the table and demand what 4.4's ion_phys() documented: the
 * entries have to form one physically contiguous run.  Anything else returns
 * -ENODEV, which makes stui_alloc_video_space() fail and
 * stui_process_cmd() report STUI_RET_ERR_INTERNAL_ERROR -- rather than
 * handing the secure world a plausible-looking but wrong address.
 */
static int stui_ion_phys(struct ion_handle *handle, unsigned long *addr,
			 size_t *len)
{
	struct ion_buffer *buffer;
	struct sg_table *table;
	struct scatterlist *sg;
	unsigned long base, cursor;
	size_t total = 0;
	int i;

	if (!handle)
		return -EINVAL;

	buffer = handle->buffer;
	if (!buffer)
		return -EINVAL;

	table = buffer->sg_table;
	if (!table || !table->nents)
		return -ENODEV;

	base = sg_phys(table->sgl);
	cursor = base;
	for (i = 0; i < table->nents; i++) {
		sg = &table->sgl[i];
		if (sg_phys(sg) != cursor)
			return -ENODEV;
		cursor += sg_dma_len(sg);
		total += sg_dma_len(sg);
	}

	*addr = base;
	*len = total;

	return 0;
}

int stui_alloc_video_space(struct tui_hw_buffer *buffer)
{
	unsigned long phys_addr = 0;
	size_t framebuf_size;
	size_t workbuf_size;
	struct decon_lcd *lcd_info = decon_drvdata[0]->lcd_info;

	framebuf_size = (lcd_info->xres * lcd_info->yres * (DEFAULT_BPP >> 3));
	workbuf_size = (lcd_info->xres * lcd_info->yres * (2 * (DEFAULT_BPP >> 3) + 1));
	framebuf_size = STUI_ALIGN_UP(framebuf_size, STUI_ALIGN_64kB_SZ);
	workbuf_size = STUI_ALIGN_UP(workbuf_size, STUI_ALIGN_64kB_SZ);
	client = exynos_ion_client_create("STUI module");
	if (IS_ERR_OR_NULL(client)) {
		pr_err("[STUI] ion_client_create() - failed: %ld\n", PTR_ERR(client));
		return 0;
	}

try_alloc:
	handle = ion_alloc(client, framebuf_size + workbuf_size, STUI_ALIGN_64kB_SZ,
					EXYNOS_ION_HEAP_VIDEO_STREAM_MASK, 0);
	if (IS_ERR_OR_NULL(handle)) {
		if (workbuf_size > STUI_ALIGN_1MB_SZ) {
			workbuf_size -= STUI_ALIGN_1MB_SZ;
			goto try_alloc;
		}
		pr_err("[STUI] ion_alloc() - failed: %ld\n", PTR_ERR(handle));
		goto clean_client;
	}

	dbuf = ion_share_dma_buf(client, handle);
	if (IS_ERR_OR_NULL(dbuf)) {
		pr_err("[STUI] ion_share_dma_buf() - failed: %ld\n", PTR_ERR(dbuf));
		goto clean_alloc;
	}

	if (stui_ion_phys(handle, &phys_addr, &dbuf->size))
		goto clean_share_dma;

	buffer->width = lcd_info->xres;
	buffer->height = lcd_info->yres;
	buffer->fb_physical = (uint64_t)phys_addr;
	buffer->wb_physical = (uint64_t)((workbuf_size) ? (phys_addr + framebuf_size) : 0);
	buffer->fb_size = framebuf_size;
	buffer->wb_size = workbuf_size;

	return 0;

clean_share_dma:
	dma_buf_put(dbuf);
clean_alloc:
	ion_free(client, handle);
clean_client:
	ion_client_destroy(client);

	return -1;
}

int stui_get_resolution(struct tui_hw_buffer *buffer)
{
	struct decon_lcd *lcd_info = decon_drvdata[0]->lcd_info;

	buffer->width = lcd_info->xres;
	buffer->height = lcd_info->yres;

	return 0;
}

int stui_prepare_tui(void)
{
	pr_debug("[STUI] %s - start\n", __func__);
	return fb_protection_for_tui(true);
}

void stui_finish_tui(void)
{
	pr_debug("[STUI] %s - start\n", __func__);
	if (fb_protection_for_tui(false))
		pr_err("[STUI] failed to unprotect tui\n");
}
