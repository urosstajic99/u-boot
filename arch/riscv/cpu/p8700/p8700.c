// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>

__weak void wait_ddr_calib(void) { }

__weak void setup_pcie_dma_map(void) { }

__weak bool dma_is_coherent(void) { return false; }
