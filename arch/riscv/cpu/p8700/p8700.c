// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>

#define BOSTON_IOCU_NOC_OFFSET	0x10

/*bool p8700_dma_is_coherent(void)
{
	u32 pcie0_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE0ADDR);
	u32 pcie1_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE1ADDR);
	u32 pcie2_off = readl((void __iomem *)BOSTON_PLAT_NOCPCIE2ADDR);

	return pcie0_off == BOSTON_IOCU_NOC_OFFSET &&
	       pcie1_off == BOSTON_IOCU_NOC_OFFSET &&
	       pcie2_off == BOSTON_IOCU_NOC_OFFSET;
}*/
