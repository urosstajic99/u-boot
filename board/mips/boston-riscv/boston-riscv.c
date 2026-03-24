// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include "boston-regs.h"
#include <asm/encoding.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>
#include <asm/arch-p8700/p8700_platform.h>

#define PMACFG0_PMP3_SHIFT	24
#define PMACFG0_PMP3_MASK	(0xffUL << PMACFG0_PMP3_SHIFT)
#define BOSTON_IOCU_NOC_OFFSET	0x10

int board_early_init_r(void)
{
	if (!IS_ENABLED(CONFIG_RISCV_MMODE))
		return 0;
	ulong pmacfg0 = csr_read(CSR_PMACFG0);

	/*
	 * Make the flash region (PMA entry corresponding to pmp3) uncached by
	 * setting the CCA field in CSR_PMACFG0[31:24].
	 */
	pmacfg0 &= ~PMACFG0_PMP3_MASK;
	pmacfg0 |= (ulong)CCA_CACHE_DISABLE << PMACFG0_PMP3_SHIFT;

	csr_write(CSR_PMACFG0, pmacfg0);
	asm volatile ("fence" ::: "memory");

	return 0;
}

u64 p8700_cm_base(void)
{
	return 0x16100000;
}

u64 p8700_cpc_base(void)
{
	return p8700_cm_base() + 0x8000;
}

u64 p8700_timer_addr(void)
{
	return 0x16108050ULL;
}

void __iomem *p8700_gcr_win(void)
{
	return (void __iomem *)0x1fb80000;
}

void p8700_wait_ddr_calib(void)
{
	while (!(readl((void __iomem *)BOSTON_PLAT_DDR3STAT) &
		 BOSTON_PLAT_DDR3STAT_CALIB)) {
		/* busy-wait */
	}
}

void p8700_setup_pcie_dma_map(void)
{
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE0ADDR);
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE1ADDR);
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE2ADDR);
}

bool p8700_dma_is_coherent(void)
{
	u32 pcie0_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE0ADDR);
	u32 pcie1_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE1ADDR);
	u32 pcie2_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE2ADDR);

	return pcie0_off == BOSTON_IOCU_NOC_OFFSET &&
	       pcie1_off == BOSTON_IOCU_NOC_OFFSET &&
	       pcie2_off == BOSTON_IOCU_NOC_OFFSET;
}
