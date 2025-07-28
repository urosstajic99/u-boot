/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include "boston-regs.h"

.data

msg_ddr_cal:	.ascii "DDR Cal "
msg_ddr_ok:	.ascii "DDR OK  "

.text

	.globl lowlevel_display
lowlevel_display:
	li	t0, BOSTON_LCD_BASE
	jr	ra
