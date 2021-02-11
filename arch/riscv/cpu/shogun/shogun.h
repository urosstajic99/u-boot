/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@wavecomp.com>
 */

#ifndef __SHOGUN_H__
#define __SHOGUN_H__

#define CCA_CACHE_ENABLE	16
#define CCA_CACHE_DISABLE	2

#define L1_I_CACHE      0
#define L1_D_CACHE      1
#define L3_CACHE        2
#define L2_CACHE        3

#define HIT_INVALIDATE          4
#define HIT_WRITEBACK_INV       5

#define HIT_INVALIDATE_D        ((HIT_INVALIDATE << 2) | L1_D_CACHE)
#define HIT_INVALIDATE_SD       ((HIT_INVALIDATE << 2) | L2_CACHE)
#define HIT_WRITEBACK_INV_D     ((HIT_WRITEBACK_INV << 2) | L1_D_CACHE)
#define HIT_WRITEBACK_INV_SD    ((HIT_WRITEBACK_INV << 2) | L2_CACHE)

#define L1D_LINE_SIZE_SHIFT	10
#define L1D_LINE_SIZE_MASK	0x7

#define GCR_L2_CONFIG	0x16100130
#define L2_LINE_SIZE_SHIFT	8
#define L2_LINE_SIZE_MASK	0xf

#endif /* __SHOGUN_H__ */
