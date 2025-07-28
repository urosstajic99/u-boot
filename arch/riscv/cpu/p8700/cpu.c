// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm/encoding.h>
#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch-p8700/p8700.h>
#include <asm/arch-p8700/p8700_platform.h>

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
	p8700_wait_ddr_calib();

	/*
	 * Only mhartid[3:0] == 0 performs CM/GCR programming.
	 * Other harts skip CM/GCR setup and go straight to PMP/PMA setup.
	 */
    if ((hartid & 0xFULL) == 0) {
		ulong cm_base = (ulong)p8700_cm_base();
		void __iomem *gcr_win = p8700_gcr_win();
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
		if (hartid == 0)
			p8700_setup_pcie_dma_map();
	}

	/* PMP setup */
	int n = 0;
	const struct p8700_pmp_entry *pmp = p8700_platform_get_pmp_policy(&n);
	ulong pmpcfg = 0;

	for (int i = 0; i < n; i++) {
		u8 entry = pmp[i].entry;
		u8 cfg = pmp[i].mode | pmp[i].flags;
		ulong shift = (ulong)entry * 8;

		p8700_platform_write_pmpaddr(entry, pmp[i].napot);

		pmpcfg &= ~(0xff << shift);
		pmpcfg |=  ((ulong)cfg << shift);
	}

	csr_write(pmpcfg0, pmpcfg);

	/* PMA/cache attributes */
	const struct p8700_pma_entry *pma = p8700_platform_get_pma_policy(hartid, &n);
	ulong pmacfg0 = 0;

	for (int i = 0; i < n; i++) {
		u8 entry = pma[i].entry;
		u8 cca = pma[i].cca;
		ulong shift = (ulong)entry * 8;

		pmacfg0 &= ~(0xff << shift);
		pmacfg0 |=  ((ulong)cca << shift);
	}

	asm volatile ("csrw %0, %1" :: "i"(CSR_PMACFG0), "r"(pmacfg0));
	asm volatile ("fence" ::: "memory");

	/* Secondary harts: after early setup, jump to the common entry point */
	if (hartid != 0)
		jump_to_addr(CONFIG_SYS_LOAD_ADDR);
}
