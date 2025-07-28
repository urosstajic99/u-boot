// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

extern void p8700_platform_setup(void);
extern void set_flash_uncached(void);

void harts_early_init(void)
{
    p8700_platform_setup();
    set_flash_uncached();
}
