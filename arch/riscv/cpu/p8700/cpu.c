// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm/encoding.h>
#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>

static __noreturn void jump_to_addr(ulong addr)
{
	asm volatile ("jr %0" :: "r"(addr) : "memory");
	__builtin_unreachable();
}

void harts_early_init(void)
{
	if (!IS_ENABLED(CONFIG_RISCV_MMODE))
		return;

	ulong hartid = csr_read(CSR_MHARTID);

	/* Wait for DDR3 calibration */
	while (!(readl((void __iomem *)BOSTON_PLAT_DDR3STAT) &
		 BOSTON_PLAT_DDR3STAT_CALIB)) {
		/* busy-wait */
	}

	/*
	 * Only mhartid[3:0] == 0 performs CM/GCR programming.
	 * Other harts skip CM/GCR setup and go straight to PMP/PMA setup.
	 */
	if ((hartid & 0xFULL) == 0) {
		ulong cm_base = CM_BASE;
		void __iomem *gcr_win = (void __iomem *)0x1fb80000;
		ulong cluster = (hartid >> MHARTID_CLUSTER_SHIFT) &
				MHARTID_CLUSTER_MASK;

		cm_base += cluster << CM_BASE_CLUSTER_SHIFT;

		if ((hartid & 0xFFFFUL) == 0)
			writeq(cm_base, gcr_win + GCR_BASE_OFFSET);

		ulong core = (hartid >> MHARTID_CORE_SHIFT) & MHARTID_CORE_MASK;

		/* Enable coherency for the current core */
		cm_base += core << CM_BASE_CORE_SHIFT;
		writeq((u64)GCR_CL_COH_EN_EN,
		       (void __iomem *)(cm_base + P8700_GCR_C0_COH_EN));

		/*
		 * On hart 0, default PCIe DMA mapping should be the non-IOCU
		 * target.
		 */
		if (hartid == 0) {
			writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE0ADDR);
			writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE1ADDR);
			writel(0x00, (void __iomem *)BOSTON_PLAT_NOCPCIE2ADDR);
		}
	}

	/* PMP setup */
	csr_write(pmpaddr1, 0x2fffffffUL);
	csr_write(pmpaddr2, 0x07ff7fffUL);
	csr_write(pmpaddr3, 0x07f3ffffUL);
	csr_write(pmpaddr4, 0x1fffffffffffffffUL);

	unsigned long pmpcfg = ((unsigned long)(PMP_NAPOT | PMP_R | PMP_W |
						PMP_X) << 32) |
				((unsigned long)(PMP_NAPOT | PMP_R |
						PMP_X) << 24) |
				((unsigned long)(PMP_NAPOT | PMP_R | PMP_W |
						PMP_X) << 16) |
				((unsigned long)(PMP_NAPOT | PMP_R | PMP_W |
						PMP_X) << 8);

	csr_write(pmpcfg0, pmpcfg);

	/* PMA/cache attributes */
	ulong pmacfg0;

	if (hartid == 0) {
		/*
		 * Hart 0: cacheable for pmp0, pmp1, pmp3; uncacheable for
		 * pmp2, pmp4.
		 */
		pmacfg0 = ((unsigned long)CCA_CACHE_DISABLE << 32) |
			((unsigned long)CCA_CACHE_ENABLE  << 24) |
			((unsigned long)CCA_CACHE_DISABLE << 16) |
			((unsigned long)CCA_CACHE_ENABLE  << 8)  |
			((unsigned long)CCA_CACHE_ENABLE);
	} else {
		/*
		 * Hart 1 or above: cacheable for pmp0, pmp1; uncacheable for
		 * pmp2, pmp3, pmp4.
		 */
		pmacfg0 = ((unsigned long)CCA_CACHE_DISABLE << 32) |
			((unsigned long)CCA_CACHE_DISABLE << 24) |
			((unsigned long)CCA_CACHE_DISABLE << 16) |
			((unsigned long)CCA_CACHE_ENABLE  << 8)  |
			((unsigned long)CCA_CACHE_ENABLE);
	}

	asm volatile ("csrw %0, %1" :: "i"(CSR_PMACFG0), "r"(pmacfg0));
	asm volatile ("fence" ::: "memory");

	/* Secondary harts: after early setup, jump to the common entry point */
	if (hartid != 0)
		jump_to_addr(CONFIG_SYS_LOAD_ADDR);
}
