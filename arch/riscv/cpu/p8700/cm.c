// SPDX-License-Identifier: GPL-2.0+
/*
 * MIPS Coherence Manager (CM) Support
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 */

#include <asm/io.h>
#include <asm/arch-p8700/cm.h>
#include <asm/arch-p8700/p8700.h>
#include <asm/arch-p8700/p8700_platform.h>

static void mips_cpc_init(void)
{
}

__weak const struct mmio_region *get_mmio_regions(void)
{
	return NULL;
}

static void p8700_setup_mmio_limits(void)
{
	void __iomem *gcrs = (void __iomem *)(uintptr_t)p8700_cm_base();
	const struct mmio_region *rgn = get_mmio_regions();
	unsigned int num_clusters = mips_cm_num_clusters();
	unsigned int limit = MIPS_CM_MMIO_LIMIT / num_clusters;
	unsigned int i, reg_off;

	if (!rgn)
		return;

	if (num_clusters != 1) {
		// FIXME! Need to support multiple clusters.
		return;
	}

	reg_off = GCR_MMIO0_BOTTOM;

	for (i = 0; rgn[i].addr_high; i++) {
		__raw_writeq(rgn[i].addr_high & GCR_MMIO0_TOP_ADDR,
			     gcrs + reg_off + (GCR_MMIO0_TOP - GCR_MMIO0_BOTTOM));

		__raw_writeq((rgn[i].addr_low & GCR_MMIO0_BOTTOM_ADDR) |
			     (rgn[i].port << GCR_MMIO0_BOTTOM_PORT_SHIFT) |
			     (rgn[i].enable ? GCR_MMIO0_BOTTOM_ENABLE : 0),
			     gcrs + reg_off);
		reg_off += GCR_MMIO1_BOTTOM - GCR_MMIO0_BOTTOM;
	}

	__raw_writel(limit, gcrs + GCR_MMIO_REQ_LIMIT);
}

int power_up_cluster(unsigned int cluster)
{
	return 0;
}

int power_down_cluster(unsigned int cluster)
{
	return 0;
}

int init_cluster_l2(unsigned int cluster)
{
	return 0;
}

int mips_cm_init(void)
{
	int err;

	mips_cpc_init();

	err = mips_cm_init_iocus();
	if (err)
		return err;

	p8700_setup_mmio_limits();

	return 0;
}

int arch_cpu_init(void)
{
	int err;

	err = mips_cm_init();
	if (err)
		return err;

	return 0;
}
