/*
 * Copyright (C) 2017 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <asm/io.h>
#include <linux/delay.h>
#include "boston-regs.h"

void _machine_restart(void)
{
	writel(BOSTON_PLAT_SOFT_RST_SYSTEM, (void __iomem *)BOSTON_PLAT_SOFT_RST);

	udelay(1000);
}
