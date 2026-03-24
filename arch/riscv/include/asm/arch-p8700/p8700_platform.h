/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */
#ifndef __ASM_ARCH_P8700_PLATFORM_H__
#define __ASM_ARCH_P8700_PLATFORM_H__

#include <linux/types.h>
#include <asm/io.h>

u64 p8700_cm_base(void);
u64 p8700_cpc_base(void);

u64 p8700_timer_addr(void);

void __iomem *p8700_gcr_win(void);

void p8700_wait_ddr_calib(void);
void p8700_setup_pcie_dma_map(void);

bool p8700_dma_is_coherent(void);

#endif
