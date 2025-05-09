/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 */

#ifndef __P8700_CM_H__
#define __P8700_CM_H__

#include <asm/arch-p8700/p8700.h>

struct mmio_region {
	phys_addr_t addr_low;
	phys_addr_t addr_high;
	unsigned int port : 4;
	unsigned int enable : 1;
};

extern const struct mmio_region *get_mmio_regions(void);

extern void setup_redirect(unsigned int cluster, unsigned int core,
			   unsigned int vp, unsigned int block);

extern int mips_cm_init_iocus(void);

extern int power_up_cluster(unsigned int cluster);
extern int power_down_cluster(unsigned int cluster);
extern int init_cluster_l2(unsigned int cluster);

static inline void *mips_cm_base(void)
{
	return (void *)CM_BASE;
}

static inline void *mips_cpc_base(void)
{
	return (void *)CPC_BASE;
}

static inline unsigned int mips_cm_num_clusters(void)
{
	u32 cfg;

	cfg = __raw_readl(mips_cm_base() + GCR_CONFIG);
	cfg >>= GCR_CONFIG_NUMCLUSTERS_SHIFT;
	cfg &= GCR_CONFIG_NUMCLUSTERS_MASK;

	return cfg;
}

static inline unsigned int mips_cluster_id(void)
{
	u32 temp;

	asm volatile("csrr %0, mhartid" : "=r"(temp));
	temp >>= MHARTID_CLUSTER_SHIFT;
	temp &= MHARTID_CLUSTER_MASK;

	return temp;
}

#endif /* __P8700_CM_H__ */
