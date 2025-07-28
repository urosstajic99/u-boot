// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include "boston-regs.h"
#include <asm/encoding.h>
#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>

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

void wait_ddr_calib(void)
{
	while (!(readl((void __iomem *)BOSTON_PLAT_DDR3STAT) &
		 BOSTON_PLAT_DDR3STAT_CALIB)) {
		/* busy-wait */
	}
}

void setup_pcie_dma_map(void)
{
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE0ADDR);
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE1ADDR);
	writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE2ADDR);
}
