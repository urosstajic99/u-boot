/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 */

#ifndef __CONFIG_BOSTON_RISCV_H
#define __CONFIG_BOSTON_RISCV_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0x80000000

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_2M)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_2M)

#define CONFIG_SYS_MALLOC_LEN		SZ_8M

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

#define CONFIG_STANDALONE_LOAD_ADDR	0x80200000

#define CONFIG_TFTP_FILE_NAME_MAX_LEN	256

/*
 * Flash
 */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT       1
#define CONFIG_SYS_MAX_FLASH_SECT               1024

/*
 * Ethernet
 */
#define CONFIG_PHY_REALTEK

/*
 * Miscellaneous
 */

#endif /* __CONFIG_BOSTON_RISCV_H */
