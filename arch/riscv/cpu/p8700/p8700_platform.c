// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm/io.h>
#include <linux/types.h>

__weak u64 p8700_cm_base(void) { return 0; }
__weak u64 p8700_cpc_base(void) { return 0; }

__weak u64 p8700_timer_addr(void) { return 0; }

__weak void __iomem *p8700_gcr_win(void) { return NULL; }

__weak void p8700_wait_ddr_calib(void) { }
__weak void p8700_setup_pcie_dma_map(void) { }

__weak bool p8700_dma_is_coherent(void) { return false; }

__weak const struct p8700_pmp_entry *p8700_platform_get_pmp_policy(int *count)
{
	*count = 0;
	return NULL;
}

__weak const struct p8700_pma_entry *p8700_platform_get_pma_policy(int *count)
{
	*count = 0;
	return NULL;
}

__weak void p8700_platform_write_pmpaddr(u8 entry, ulong pmpaddr_val) { }
