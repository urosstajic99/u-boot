// SPDX-License-Identifier: GPL-2.0+
/*
 * MIPS Coherence Manager (CM) Support
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 */
#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/arch-p8700/cm.h>
#include <asm/arch-p8700/p8700_platform.h>

__weak bool plat_iocu_usable(unsigned int cluster, unsigned int iocu)
{
	return true;
}

static int init_cluster_iocus(unsigned int cluster)
{
	DECLARE_GLOBAL_DATA_PTR;
	void __iomem *global_gcrs;
	u32 cfg, num_iocus, num_iocus_usable, local_cluster;
	int i;

	local_cluster = mips_cluster_id();

	global_gcrs = (void __iomem *)(uintptr_t)p8700_cm_base();
	if (cluster != local_cluster) {
		// FIXME
		return 1;
	}

	cfg = __raw_readl(global_gcrs + GCR_CONFIG);
	num_iocus = cfg >> GCR_CONFIG_NUMIOCU_SHIFT;
	num_iocus &= GCR_CONFIG_NUMIOCU_MASK;
	gd->arch.num_iocus += num_iocus;

	/* Discover how many IOCUs are usable */
	num_iocus_usable = num_iocus;
	for (i = num_iocus - 1; i >= 0; i--) {
		if (!plat_iocu_usable(cluster, i))
			num_iocus_usable--;
	}
	gd->arch.num_iocus_usable += num_iocus_usable;

	/* If the cluster has no usable IOCUs there's nothing to do */
	if (num_iocus_usable == 0) {
		if (cluster != local_cluster)
			power_down_cluster(cluster);

		return 0;
	}

	/* If the IOCUs are in the local cluster we're good to go already */
	if (cluster == local_cluster)
		return 0;

	/* Ensure that the cluster's L2 cache is initialised */
	return init_cluster_l2(cluster);
}

int mips_cm_init_iocus(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int cluster;
	int err;

	for (cluster = 0; cluster < mips_cm_num_clusters(); cluster++) {
		err = init_cluster_iocus(cluster);
		if (err) {
			gd->arch.num_iocus_usable = err;
			return 0;
		}
	}

	return 0;
}
