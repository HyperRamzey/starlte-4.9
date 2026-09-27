/*
 * DERP PORT NOTE (4.4 exynos7885 -> 4.9.219 starlte port)
 *
 * Ported from the 4.4 Exynos7885 (Galaxy A30s) tree. The 4.9 base is the
 * Samsung Exynos9810 (Galaxy S9) tree, which forked before the s2m_ pdic
 * notifier existed, so there is no 4.9 donor for this file.
 *
 * WHY THIS FILE IS NEEDED EVEN THOUGH USB-PD IS NOT PORTED
 * ---------------------------------------------------------
 * This is NOT part of the USB-PD stack. It is the attach/detach notifier
 * chain that the S2MU106 *MUIC* driver broadcasts on. Ten call sites in the
 * already-ported MUIC layer reach it:
 *
 *   drivers/muic/s2mu106-muic.c  : 1369, 1889, 1914, 1981, 2122, 2242
 *   drivers/muic/muic_manager.c  :  778,  788
 *
 * all through the macros in include/linux/muic/s2mu106-muic.h:1186-1193
 *
 *   MUIC_SEND_NOTI_TO_CCIC_ATTACH(dev) -> s2m_pdic_notifier_attach_attached_dev()
 *   MUIC_SEND_NOTI_TO_CCIC_DETACH(dev) -> s2m_pdic_notifier_detach_attached_dev()
 *
 * Without this .c the MUIC/AFC layer does not link. The 4.4 call sites are
 * byte-identical to the ported ones (same line numbers), so the port is a
 * verbatim move.
 *
 * THE "pdic_notifier.h COLLISION" - RECONCILED
 * --------------------------------------------
 * There is no collision. 4.9 already has a *different* pdic notifier, in
 * include/linux/battery/battery_notifier.h, which serves the max77705 /
 * s2mm003 / s2mm005 CCIC path:
 *
 *   struct pdic_notifier_struct          (no s2m_ prefix)
 *   pdic_notifier_call() / _register() / _unregister()
 *   implemented in drivers/battery/battery_notifier.c:171,63,89
 *
 * The two APIs are disjoint - different struct names, different function
 * prefixes, different .c files. Both are live in this tree at once:
 *
 *   struct pdic_notifier_struct pd_noti;    s2mu106-usbpd.c:59      (4.9 API)
 *   s2m_pdic_notifier_notify()              this file               (4.4 API)
 *
 * s2mu106-usbpd.c therefore uses BOTH notifier families, exactly as 4.4 does.
 * Nothing was renamed and no file outside drivers/ccic/ was touched.
 *
 * ONE ADAPTATION
 * --------------
 * s2m_pdic_notifier_attach_attached_jig_dev() assigned
 * PDIC_MUIC_NOTIFY_CMD_JIG_ATTACH, one of the four
 * muic_notifier_cmd_t enumerators that 4.4 has and 4.9 does not. That enum
 * lives in include/linux/muic/muic_notifier.h, which this port does not own.
 * The function is compiled out below rather than given a fabricated enum
 * value, because:
 *   - it has zero callers in the 4.4 tree and zero in this one (measured),
 *   - 4.4 already compiles out its own sibling,
 *     s2m_pdic_notifier_detach_attached_jig_dev(), the same way
 *     (4.4 lines 88-106), and
 *   - inventing an out-of-range enumerator would silently change the
 *     notifier command that subscribers match on.
 * The missing enumerators are reported as a shared-hunk.
 */

#include <linux/device.h>

#include <linux/notifier.h>
#include <linux/ccic/pdic_notifier.h>

#define SET_PDIC_NOTIFIER_BLOCK(nb, fn, dev) do {	\
		(nb)->notifier_call = (fn);		\
		(nb)->priority = (dev);			\
	} while (0)

#define DESTROY_PDIC_NOTIFIER_BLOCK(nb)			\
		SET_PDIC_NOTIFIER_BLOCK(nb, NULL, -1)

static struct s2m_pdic_notifier_struct pdic_notifier;

int s2m_pdic_notifier_register(struct notifier_block *nb, notifier_fn_t notifier,
			muic_notifier_device_t listener)
{
	int ret = 0;

	pr_info("%s: listener=%d register\n", __func__, listener);

	SET_PDIC_NOTIFIER_BLOCK(nb, notifier, listener);
	ret = blocking_notifier_chain_register(&(pdic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		pr_err("%s: blocking_notifier_chain_register error(%d)\n",
				__func__, ret);

	/* current pdic's attached_device status notify */
	nb->notifier_call(nb, pdic_notifier.cmd,
			&(pdic_notifier.attached_dev));

	return ret;
}

int s2m_pdic_notifier_unregister(struct notifier_block *nb)
{
	int ret = 0;

	pr_info("%s: listener=%d unregister\n", __func__, nb->priority);

	ret = blocking_notifier_chain_unregister(&(pdic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		pr_err("%s: blocking_notifier_chain_unregister error(%d)\n",
				__func__, ret);
	DESTROY_PDIC_NOTIFIER_BLOCK(nb);

	return ret;
}

static int s2m_pdic_notifier_notify(void)
{
	int ret = 0;

	pr_info("%s: CMD=%d, DATA=%d\n", __func__, pdic_notifier.cmd,
			pdic_notifier.attached_dev);

	ret = blocking_notifier_call_chain(&(pdic_notifier.notifier_call_chain),
			pdic_notifier.cmd, &(pdic_notifier.attached_dev));

	switch (ret) {
	case NOTIFY_STOP_MASK:
	case NOTIFY_BAD:
		pr_err("%s: notify error occur(0x%x)\n", __func__, ret);
		break;
	case NOTIFY_DONE:
	case NOTIFY_OK:
		pr_info("%s: notify done(0x%x)\n", __func__, ret);
		break;
	default:
		pr_info("%s: notify status unknown(0x%x)\n", __func__, ret);
		break;
	}

	return ret;
}

#if 0
/* DERP PORT: disabled - see the port note at the top of this file.
 * 4.4 used PDIC_MUIC_NOTIFY_CMD_JIG_ATTACH here, a muic_notifier_cmd_t
 * enumerator that does not exist in the 4.9 Exynos9810 header. This function
 * has no caller in either tree, and 4.4 already compiles out its sibling
 * s2m_pdic_notifier_detach_attached_jig_dev() the same way, directly below.
 */
void s2m_pdic_notifier_attach_attached_jig_dev(muic_attached_dev_t new_dev)
{
	pr_info("%s: (%d)\n", __func__, new_dev);

	pdic_notifier.cmd = PDIC_MUIC_NOTIFY_CMD_JIG_ATTACH;
	pdic_notifier.attached_dev = new_dev;

	/* pdic's attached_device attach broadcast */
	s2m_pdic_notifier_notify();
}
#endif
#if 0
void s2m_pdic_notifier_detach_attached_jig_dev(muic_attached_dev_t cur_dev)
{
	pr_info("%s: (%d)\n", __func__, cur_dev);

	pdic_notifier.cmd = PDIC_MUIC_NOTIFY_CMD_JIG_DETACH;

	if (pdic_notifier.attached_dev != cur_dev)
		pr_warn("%s: attached_dev of pdic_notifier(%d) != pdic_data(%d)\n",
				__func__, pdic_notifier.attached_dev, cur_dev);

	if (pdic_notifier.attached_dev != ATTACHED_DEV_NONE_MUIC) {
		/* pdic's attached_device detach broadcast */
		s2m_pdic_notifier_notify();
	}

	pdic_notifier.attached_dev = ATTACHED_DEV_NONE_MUIC;
}
#endif
void s2m_pdic_notifier_attach_attached_dev(muic_attached_dev_t new_dev)
{
	pr_info("%s: (%d)\n", __func__, new_dev);

	pdic_notifier.cmd = MUIC_NOTIFY_CMD_ATTACH;
	pdic_notifier.attached_dev = new_dev;

	/* pdic's attached_device attach broadcast */
	s2m_pdic_notifier_notify();
}

void s2m_pdic_notifier_detach_attached_dev(muic_attached_dev_t cur_dev)
{
	pr_info("%s: (%d)\n", __func__, cur_dev);

	pdic_notifier.cmd = MUIC_NOTIFY_CMD_DETACH;

	if (pdic_notifier.attached_dev != cur_dev)
		pr_warn("%s: attached_dev of pdic_notifier(%d) != pdic_data(%d)\n",
				__func__, pdic_notifier.attached_dev, cur_dev);

	if (pdic_notifier.attached_dev != ATTACHED_DEV_NONE_MUIC) {
		/* pdic's attached_device detach broadcast */
		s2m_pdic_notifier_notify();
	}

	pdic_notifier.attached_dev = ATTACHED_DEV_NONE_MUIC;
}

void s2m_pdic_notifier_logically_attach_attached_dev(muic_attached_dev_t new_dev)
{
	pr_info("%s: (%d)\n", __func__, new_dev);

	pdic_notifier.cmd = MUIC_NOTIFY_CMD_LOGICALLY_ATTACH;
	pdic_notifier.attached_dev = new_dev;

	/* pdic's attached_device attach broadcast */
	s2m_pdic_notifier_notify();
}

void s2m_pdic_notifier_logically_detach_attached_dev(muic_attached_dev_t cur_dev)
{
	pr_info("%s: (%d)\n", __func__, cur_dev);

	pdic_notifier.cmd = MUIC_NOTIFY_CMD_LOGICALLY_DETACH;
	pdic_notifier.attached_dev = cur_dev;

	/* pdic's attached_device detach broadcast */
	s2m_pdic_notifier_notify();

	pdic_notifier.attached_dev = ATTACHED_DEV_NONE_MUIC;
}

static int __init s2m_pdic_notifier_init(void)
{
	int ret = 0;

	pr_info("%s\n", __func__);

	BLOCKING_INIT_NOTIFIER_HEAD(&(pdic_notifier.notifier_call_chain));
	pdic_notifier.cmd = MUIC_NOTIFY_CMD_DETACH;
	pdic_notifier.attached_dev = ATTACHED_DEV_UNKNOWN_MUIC;

	return ret;
}
subsys_initcall(s2m_pdic_notifier_init);

