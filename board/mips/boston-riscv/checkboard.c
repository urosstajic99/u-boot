// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <init.h>
#include <asm/io.h>
#include "boston-lcd.h"
#include "boston-regs.h"
#include <asm/arch-p8700/p8700.h>

int checkboard(void)
{
	u32 changelist, cfg, core, uarch;
	u64 marchid;

	lowlevel_display("U-boot  ");

	printf("Board: Mips Boston RISC-V\n");

	changelist = in_le32((uint32_t *)BOSTON_PLAT_CORE_CL);
	if (changelist > 1) {
		asm volatile ("csrr %0, marchid" : "=r"(marchid)::);
		core = (marchid >> MARCHID_CLASS_SHIFT) & MARCHID_CLASS_MASK;
		uarch = (marchid >> MARCHID_UARCH_SHIFT) & MARCHID_UARCH_MASK;

		printf("Core:  class%x uarch%x cl%x", core, uarch, changelist);

		cfg = in_le32((uint32_t *)BOSTON_PLAT_BUILDCFG0);
		if (cfg & BOSTON_PLAT_BUILDCFG0_CFG_NUM)
			printf(" config %u",
			       (cfg & BOSTON_PLAT_BUILDCFG0_CFG_NUM) >> 8);
		if (cfg & BOSTON_PLAT_BUILDCFG0_CFG_LTR)
			printf("%c",
			       'a' + ((cfg & BOSTON_PLAT_BUILDCFG0_CFG_LTR) >> 4) - 1);
		if (cfg & BOSTON_PLAT_BUILDCFG0_DP)
			printf(", x%u debug port",
			       (cfg & BOSTON_PLAT_BUILDCFG0_DP_MULT) >> 28);
		printf("\n");
	}

	return 0;
}
