// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <command.h>
#include <cpu_func.h>

void __weak _machine_restart(void)
{
	printf("reset not supported yet\n");
	hang();
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	reset_cpu();

	return 0;
}
