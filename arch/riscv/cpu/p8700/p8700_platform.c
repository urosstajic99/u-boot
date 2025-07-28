/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */
#include <linux/types.h>
#include <asm/io.h>

__weak u64 p8700_cm_base(void) { return 0; }
__weak u64 p8700_cpc_base(void) { return 0; }

__weak u64 p8700_timer_addr(void) { return 0; }

__weak void __iomem *p8700_gcr_win(void) { return NULL; }

__weak void p8700_board_wait_ddr_calib(void) { }
__weak void p8700_board_setup_pcie_dma_map(void) { }

__weak bool p8700_dma_is_coherent(void)
{
    return false;
}
