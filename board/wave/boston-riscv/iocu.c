/*
 * Copyright (C) 2016 Imagination Technologies
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <env_callback.h>
#include "boston-regs.h"

static int set_io_coherent(bool coherent)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (!coherent) {
		printf("I/O:   Non-Coherent (No IOCU)\n");
		goto noncoherent;
	}

	/* Record that I/O is coherent */
	gd->flags |= GD_FLG_COHERENT_DMA;

	printf("I/O:   Coherent\n");
	return 0;

noncoherent:
	/* Record that I/O is not coherent */
	gd->flags &= ~GD_FLG_COHERENT_DMA;
	return 0;
}

static int on_io_coherent(const char *name, const char *value,
			  enum env_op op, int flags)
{
	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		if (!strcmp(value, "0")) {
			set_io_coherent(false);
		} else if (!strcmp(value, "1")) {
			set_io_coherent(true);
		} else {
			printf("### io.coherent must equal 0 or 1\n");
			return -EINVAL;
		}
		return 0;

	case env_op_delete:
		set_io_coherent(true);
		return 0;

	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(io_coherent, on_io_coherent);

int misc_init_f(void)
{
	return set_io_coherent(env_get_yesno("io.coherent") == 1);
}
