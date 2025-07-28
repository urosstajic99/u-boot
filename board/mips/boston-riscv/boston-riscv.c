// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <asm/encoding.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>

#define PMACFG0_PMP3_SHIFT	24
#define PMACFG0_PMP3_MASK	(0xffUL << PMACFG0_PMP3_SHIFT)

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
