// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <cpu_func.h>
#include <asm/global_data.h>
#include <asm/arch-p8700/p8700.h>

/* NOTE: We force to use a0 in mcache to encode via .word. */
#define cache_loop(start, end, lsize, op) do {				\
	const __typeof__(lsize) __lsize = (lsize);			\
	const register void *addr asm("a0") = (const void *)((start) & ~(__lsize - 1));	\
	const void *aend = (const void *)(((end) - 1) & ~(__lsize - 1));	\
	for (; addr <= aend; addr += __lsize)				\
		asm volatile (".word 0xec0500f3|%0 # force to use %1" \
								::"i"((op) << 20), "r"(addr));	\
} while (0)

static unsigned long lsize;
static unsigned long l1d_total_size;
static unsigned long slsize;

static void probe_cache_config(void)
{
	lsize = 64;
	l1d_total_size = 64 * 1024;

	int l2_config = 0;
	long address = GCR_L2_CONFIG;

	asm volatile ("lw %0,0(%1)" : "=r"(l2_config) : "r"(address) : "memory");
	int l2_line_size_info = (l2_config >> L2_LINE_SIZE_SHIFT)
				& L2_LINE_SIZE_MASK;
	slsize = (l2_line_size_info == 0) ? 0 : 1 << (l2_line_size_info + 1);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->flags & GD_FLG_COHERENT_DMA)
		return;

	if (lsize == 0)
		probe_cache_config();

	/* aend will be miscalculated when size is zero, so we return here */
	if (start >= end)
		return;

	cache_loop(start, end, lsize, HIT_WRITEBACK_INV_D);

	/* flush L2 cache */
	if (slsize)
		cache_loop(start, end, slsize, HIT_WRITEBACK_INV_SD);

	/* ensure cache ops complete before any further memory access */
	asm volatile ("slli x0,x0,1 # ihb");
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->flags & GD_FLG_COHERENT_DMA)
		return;

	if (lsize == 0)
		probe_cache_config();

	/* aend will be miscalculated when size is zero, so we return here */
	if (start >= end)
		return;

	/* invalidate L2 cache */
	if (slsize)
		cache_loop(start, end, slsize, HIT_INVALIDATE_SD);

	cache_loop(start, end, lsize, HIT_INVALIDATE_D);

	/* ensure cache ops complete before any further memory access */
	asm volatile ("slli x0,x0,1 # ihb");
}
