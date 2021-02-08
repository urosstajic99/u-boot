// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <init.h>

#include <asm/io.h>
//#include <asm/mipsregs.h>

#include "boston-lcd.h"
#include "boston-regs.h"

int checkboard(void)
{
	u32 changelist;

	lowlevel_display("U-boot  ");

	printf("Board: Wave Boston RISC-V\n");

	changelist = __raw_readl((uint32_t *)BOSTON_PLAT_CORE_CL);
	if (changelist > 1)
		printf("CPU cl: %x\n", changelist);

	return 0;
}
