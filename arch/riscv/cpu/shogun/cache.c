// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 */

#include <common.h>
#include <cpu_func.h>
#include "shogun.h"

void icache_enable(void)
{
	int cca = CCA_CACHE_ENABLE;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void icache_disable(void)
{
	int cca = CCA_CACHE_DISABLE;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void dcache_enable(void)
{
	int cca = CCA_CACHE_ENABLE;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void dcache_disable(void)
{
	int cca = CCA_CACHE_DISABLE;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

#define cache_loop(start, end, lsize, op) do {				\
	const void *addr = (const void *)(start & ~(lsize - 1));	\
	const void *aend = (const void *)((end - 1) & ~(lsize - 1));	\
	for (; addr <= aend; addr += lsize)				\
		asm volatile ("mcache %0,(%1)"::"i"(op),"r"(addr));	\
} while (0)

static unsigned long lsize = 0;
static unsigned long l1d_total_size = 0;
static unsigned long slsize = 0;

static void probe_cache_config(void)
{
	lsize = 64;
	l1d_total_size = 64 * 1024;
#if 0
	// Wait for RTL to implement mipsconfig1
	int config1 = 0;
	asm volatile ("csrr %0, mipsconfig1":"=r"(config1)::);

	int l1d_line_size_info = (config1 >> L1D_LINE_SIZE_SHIFT)
				 & L1D_LINE_SIZE_MASK;
	lsize = (l1d_line_size_info == 0) ? 0 : 1 << (l1d_line_size_info + 1);
#endif

	int l2_config = 0;
	long address = GCR_L2_CONFIG;
	asm volatile ("lw %0,0(%1)":"=r"(l2_config):"r"(address):"memory");
	int l2_line_size_info = (l2_config >> L2_LINE_SIZE_SHIFT)
				& L2_LINE_SIZE_MASK;
	slsize = (l2_line_size_info == 0) ? 0 : 1 << (l2_line_size_info + 1);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	if (lsize == 0)
		probe_cache_config();

	if (start == end)
		return;

	cache_loop(start, end, lsize, HIT_WRITEBACK_INV_D);

	/* flush L2 cache */
	if (slsize)
		cache_loop(start, end, slsize, HIT_WRITEBACK_INV_SD);

	/* ensure cache ops complete before any further memory access */
	asm volatile ("ihb");
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	if (lsize == 0)
		probe_cache_config();

	if (start == end)
		return;

	cache_loop(start, end, lsize, HIT_INVALIDATE_D);

	/* invalidate L2 cache */
	if (slsize)
		cache_loop(start, end, slsize, HIT_INVALIDATE_SD);

	/* ensure cache ops complete before any further memory access */
	asm volatile ("ihb");
}

#if 0
void flush_dcache_all(void)
{
	if (lsize == 0)
		probe_cache_config();

	cache_loop(0, l1d_total_size, lsize, HIT_WRITEBACK_INV_D);

	/* ensure cache ops complete before any further memory access */
	asm volatile ("ihb");
}
#endif
