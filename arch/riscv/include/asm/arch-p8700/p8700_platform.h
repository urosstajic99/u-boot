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

/* PMP entry configuration */
struct p8700_pmp_entry {
	u8 entry;        /* PMP entry number */
	ulong napot;   /* NAPOT encoding for pmpaddrN */
	u8 flags;      /* PMP_R | PMP_W | PMP_X */
    u8 mode;       /* PMP mode: PMP_TOR, PMP_NA4, PMP_NAPOT */
};

/* PMA cacheability */
struct p8700_pma_entry {
	u8 entry;        /*PMA entry number (pmacfg0 byte index) */
	u8 cca;        /* CCA_* (cacheability attribute) */
};

/*
 * Platform-provided PMP/PMA tables.
 * Return a pointer to a static table and store the number of entries in @count.
 */
const struct p8700_pmp_entry *p8700_platform_get_pmp_policy(int *count);
const struct p8700_pma_entry *p8700_platform_get_pma_policy(ulong hartid, int *count);

void p8700_platform_write_pmpaddr(u8 entry, ulong pmpaddr_val);

#endif
