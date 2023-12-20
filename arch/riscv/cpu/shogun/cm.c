/*
 * MIPS Coherence Manager (CM) Support
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch-shogun/cm.h>
#include <asm/arch-shogun/shogun.h>

void setup_redirect(unsigned int cluster, unsigned int core,
		    unsigned int vp, unsigned int block)
{
	u32 redir = 0;

	if (__raw_readl(mips_cm_base() + GCR_REV) >= GCR_REV_CM3_5) {
		redir |= GCR_Cx_REDIRECT_CLUSTER_EN;
		redir |= GCR_Cx_REDIRECT_GIC_EN;
		redir |= cluster << GCR_Cx_REDIRECT_CLUSTER_SHIFT;
		redir |= block << GCR_Cx_REDIRECT_BLOCK_SHIFT;
	} else {
		/* CM < 3.5 doesn't support cluster or block redirects */
		//assert(cluster == 0);
		//assert(block == 0);
	}

	redir |= core << GCR_Cx_REDIRECT_CORE_SHIFT;
	redir |= vp << GCR_Cx_REDIRECT_VP_SHIFT;

	/*
	 * Set the redirect register & sync to ensure later memory accesses to
	 * GCRs cannot be reordered and handled before the new redirect value
	 * takes effect.
	 */
	__raw_writel(redir, mips_cm_base() + GCR_Cx_REDIRECT);
	sync();
}

static void mips_cpc_init(void)
{
}

__weak const struct mmio_region *get_mmio_regions(void)
{
	return NULL;
}

static void kingv_setup_mmio_limits(void)
{
	void __iomem *gcrs = mips_cm_base();
	const struct mmio_region *rgn = get_mmio_regions();
	unsigned int num_clusters = mips_cm_num_clusters();
	unsigned int limit = CONFIG_MIPS_CM_MMIO_LIMIT / num_clusters;
	unsigned int i, reg_off;

	if (!rgn)
		return;

	if (num_clusters != 1)
	{
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

static void setup_mmio_limits(void)
{
	void __iomem *gcrs = mips_cm_base();
	const struct mmio_region *rgn = get_mmio_regions();
	unsigned int num_clusters = mips_cm_num_clusters();
	unsigned int limit = CONFIG_MIPS_CM_MMIO_LIMIT / num_clusters;
	unsigned int cm_rev = __raw_readl(gcrs + GCR_REV);
	unsigned int cl, i, reg_off;

	if (!rgn)
		return;

	if (cm_rev < GCR_REV_CM3_5)
		return;

	gcrs += GCR_OFF_REDIRECT;

	for (cl = 0;  cl < num_clusters; cl++) {
		if (cl > 0) {
			setup_redirect(cl, 0, 0, GCR_Cx_REDIRECT_BLOCK_CPC_GLOBAL);

			/* Skip clusters we didn't power up */
			if (!__raw_readl(mips_cpc_base() + CPC_OFF_REDIRECT + CPC_PWRUP_CTL))
				continue;
		}

		setup_redirect(cl, 0, 0, GCR_Cx_REDIRECT_BLOCK_GCR_GLOBAL);

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
	long marchid;

	mips_cpc_init();

	err = mips_cm_init_iocus();
	if (err)
		return err;

	/* Check if marchid is King-V */
	asm volatile("csrr %0, marchid":"=r"(marchid));
	if (marchid != KINGV_MARCHID)
		setup_mmio_limits();
	else
		kingv_setup_mmio_limits();

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
