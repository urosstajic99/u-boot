// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 *
 * U-Boot syscon driver for MIPS GIC.
 */

#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

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
	{ .compatible = "mips,gic0", .data = RISCV_SYSCON_PLICSW },
	{ }
};

U_BOOT_DRIVER(mips_gic) = {
	.name		= "mips_gic",
	.id		= UCLASS_SYSCON,
	.of_match	= mips_gic_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
