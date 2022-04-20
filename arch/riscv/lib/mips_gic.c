// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 *
 * U-Boot syscon driver for MIPS GIC.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <linux/err.h>

#define SHOGUN_TIMER_ADDR 0x16120010
#define KINGV_TIMER_ADDR 0x16200050

static long timer_addr = 0;

DECLARE_GLOBAL_DATA_PTR;

int riscv_get_time(u64 *time)
{
	const char *model;
	if (timer_addr == 0)
	{
		timer_addr = SHOGUN_TIMER_ADDR;
#ifdef CONFIG_OF_CONTROL
		model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
		if (strcmp (model, "king-v") == 0)
			timer_addr = KINGV_TIMER_ADDR;
#endif
	}

	*time = readq((void __iomem *)timer_addr);

	return 0;
}

int riscv_set_timecmp(int hart, u64 cmp)
{
	return 0;
}

int riscv_init_ipi(void)
{
	return 0;
}

int riscv_send_ipi(int hart)
{
	return 0;
}

int riscv_clear_ipi(int hart)
{
	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	return 0;
}

static const struct udevice_id mips_gic_ids[] = {
	{ .compatible = "mips,gic0", .data = RISCV_SYSCON_PLIC },
	{ }
};

U_BOOT_DRIVER(mips_gic) = {
	.name		= "mips_gic",
	.id		= UCLASS_SYSCON,
	.of_match	= mips_gic_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
