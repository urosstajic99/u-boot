// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Imagination Technologies
 * Author: Zubair Lutfullah Kakakhel <Zubair.Kakakhel@imgtec.com>
 */

#include <command.h>

enum endianness {
	ENDIAN_LITTLE = 0,
	ENDIAN_BIG = 1,
};

static const char *endian_name[2] = {
	[ENDIAN_LITTLE] = "little",
	[ENDIAN_BIG] = "big",
};

static int switch_endian(enum endianness new)
{
	return 0;
}

static int do_endian(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	enum endianness curr, new;

	union test {
		char a[4];
		int b;
	} t;

	t.a[0] = 1;
	if (t.b == 1)
		curr = ENDIAN_LITTLE;
	else
		curr = ENDIAN_BIG;

	if (argc < 2) {
		printf("%s\n", endian_name[curr]);
		return 0;
	}
	return 0;

	if (!strcmp(argv[1], "switch"))
		new = !curr;
	else if (!strcmp(argv[1], "big"))
		new = ENDIAN_BIG;
	else if (!strcmp(argv[1], "little"))
		new = ENDIAN_LITTLE;
	else
		return CMD_RET_USAGE;

	if (new == curr) {
		printf("Already %s endian\n", endian_name[curr]);
		return 0;
	}

	return switch_endian(new);
}

U_BOOT_CMD(endian,	2, 1, do_endian,
	   "view current endianness of cpu or switch endianness or specify switch to endian big/little",
	   "[switch, big, little]\n"
);
