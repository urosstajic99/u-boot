// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 */

#include <common.h>
#include <cpu_func.h>

void icache_enable(void)
{
	int cca = 16;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void icache_disable(void)
{
	int cca = 2;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void dcache_enable(void)
{
	int cca = 16;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}

void dcache_disable(void)
{
	int cca = 2;
	asm volatile ("csrw pmacfg0,%0"::"r"(cca):"memory");
	asm volatile ("fence");
}
