// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <command.h>
#include <asm/arch-p8700/p8700.h>

int start_harts(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int n_cluster = 1;
	int n_core = 1;
	int run = 0;
	int cl, co;
	long cmd_reg;

	if (argc > 1 && argv[1][0] > '0' && argv[1][0] <= '9')
		n_cluster = argv[1][0] - '0';
	if (argc > 2 && argv[2][0] > '0' && argv[2][0] <= '9')
		n_core = argv[2][0] - '0';
	if (argc > 3)
		run = (argv[3][0] == 'g');

	for (cl = 1; cl < n_cluster; cl++) {
		cmd_reg = CPC_BASE + (cl << CM_BASE_CLUSTER_SHIFT) +
					CPC_OFF_LOCAL + CPC_Cx_CMD;
		printf("# Start cluster %d core 0 hart 0\n", cl);
		printf("mw.q 0x%lx %d\n", cmd_reg, CPC_Cx_CMD_PWRUP);
		if (run == 1) {
			asm volatile("sd %0, 0(%1)"::"r"(CPC_Cx_CMD_PWRUP), "r"(cmd_reg));
			asm volatile("fence");
		}

		printf("mw.q 0x%lx %d\n", cmd_reg, CPC_Cx_CMD_RESET);
		if (run == 1) {
			asm volatile("sd %0, 0(%1)"::"r"(CPC_Cx_CMD_RESET), "r"(cmd_reg));
			asm volatile("fence");
		}

		cmd_reg = CPC_BASE + (cl << CM_BASE_CLUSTER_SHIFT) + CPC_OFF_LOCAL + CPC_Cx_VP_RUN;
		printf("mw.q 0x%lx 1\n", cmd_reg);

		if (run == 1) {
			asm volatile("sd %0, 0(%1)"::"r"(1), "r"(cmd_reg));
			asm volatile("fence");
		}
	}

	for (cl = 0; cl < n_cluster; cl++) {
		for (co = 1; co < n_core; co++) {
			cmd_reg = CPC_BASE + (cl << CM_BASE_CLUSTER_SHIFT) +
						(co << CM_BASE_CORE_SHIFT) +
						CPC_OFF_LOCAL + CPC_Cx_CMD;
			printf("# Start cluster %d core %d hart 0\n", cl, co);
			printf("mw.q 0x%lx %d\n", cmd_reg, CPC_Cx_CMD_RESET);
			if (run == 1) {
				asm volatile("sd %0, 0(%1)"::"r"(CPC_Cx_CMD_RESET), "r"(cmd_reg));
				asm volatile("fence");
			}

			cmd_reg = CPC_BASE + (cl << CM_BASE_CLUSTER_SHIFT) +
						(co << CM_BASE_CORE_SHIFT) +
						CPC_OFF_LOCAL + CPC_Cx_VP_RUN;
			printf("mw.q 0x%lx 1\n", cmd_reg);
			if (run == 1) {
				asm volatile("sd %0, 0(%1)"::"r"(1), "r"(cmd_reg));
				asm volatile("fence");
			}
		}
	}

	for (cl = 0; cl < n_cluster; cl++) {
		for (co = 0; co < n_core; co++) {
			printf("# Start cluster %d core %d all harts\n", cl, co);
			cmd_reg = CPC_BASE + (cl << CM_BASE_CLUSTER_SHIFT) +
						(co << CM_BASE_CORE_SHIFT) +
						CPC_OFF_LOCAL + CPC_Cx_VP_RUN;
			printf("mw.q 0x%lx 0xff\n", cmd_reg);
			if (run == 1) {
				asm volatile("sd %0, 0(%1)"::"r"(0xff), "r"(cmd_reg));
				asm volatile("fence");
			}
		}
	}

	printf("go 0x80000000\n");
	if (run == 1) {
		void (*addr)(void) = (void (*)(void))0x80000000;
		(*addr)();
	}

	return 0;
}

/***************************************************/

U_BOOT_CMD(startharts, CONFIG_SYS_MAXARGS, 1, start_harts,
	   "print commands to start all harts and go 0x80000000",
	   "[<#clusters:1-9>] [<#cores_per_cluster:1-9>] [g]\n"
	   "    - print commands to start all harts and go 0x80000000\n"
	   "      If the 3rd parameter is 'g', execute all comamnds."
);
