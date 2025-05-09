// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <cpu_func.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-p8700/p8700.h>

#define MCACHE_BASE_INST 0xec0500f3

/* NOTE: We force to use a0 in mcache to encode via .word.
 * 0xec0500f3 is a manually encoded custom RISC-V MCACHE instruction.
 * The bits [19:15] are set to 01010, selecting register x10 (a0)
 * as the source operand.
 * The bits [24:20] represent the 'op' field, which is currently set to 0.
 * Different cache operations are applied by OR-ing (op << 20) dynamically
 * to this base value.
 * Because of this encoding, the variable 'addr' is forced into register a0,
 * so that the MCACHE instruction uses the address in a0 as its operand.
 */
#define cache_loop(start, end, lsize, op) do {				\
	const __typeof__(lsize) __lsize = (lsize);			\
	const register void *addr asm("a0") = (const void *)((start) & ~(__lsize - 1));	\
	const void *aend = (const void *)(((end) - 1) & ~(__lsize - 1));	\
	for (; addr <= aend; addr += __lsize)				\
		asm volatile (".word %0 | %1 # force to use %2" \
					::"i"(MCACHE_BASE_INST), "i"((op) << 20), "r"(addr)); \
} while (0)

static unsigned long lsize;
static unsigned long l1d_total_size;
static unsigned long slsize;

static void probe_cache_config(void)
{
	lsize = 64;
	l1d_total_size = 64 * 1024;

	int l2_config = 0;

	l2_config = readl((void __iomem *)GCR_L2_CONFIG);
	int l2_line_size_info = (l2_config >> L2_LINE_SIZE_SHIFT)
				& L2_LINE_SIZE_MASK;
	slsize = (l2_line_size_info == 0) ? 0 : 1 << (l2_line_size_info + 1);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	if (p8700_dma_is_coherent())
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

	/* Instruction Hazard Barrier (IHB) — a hint-encoded SLLI (rd=0, rs1=0, imm=1).
	 * Ensures that all subsequent instruction fetches, including speculative ones,
	 * observe state changes from prior instructions.
	 * Required after MCACHE instructions when instruction fetch depends on cache ops.
	 */
	asm volatile ("slli x0,x0,1 # ihb");
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	if (p8700_dma_is_coherent())
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

	/* Instruction Hazard Barrier (IHB) — a hint-encoded SLLI (rd=0, rs1=0, imm=1).
	 * Ensures that all subsequent instruction fetches, including speculative ones,
	 * observe state changes from prior instructions.
	 * Required after MCACHE instructions when instruction fetch depends on cache ops.
	 */
	asm volatile ("slli x0,x0,1 # ihb");
}
