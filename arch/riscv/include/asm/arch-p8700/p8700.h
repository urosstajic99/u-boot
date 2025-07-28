/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#ifndef __P8700_H__
#define __P8700_H__

#define CSR_MIPSCONFIG7		0x7d7
#define CSR_PMACFG0			0x7e0

#define MHARTID_HART_SHIFT	0
#define MHARTID_HART_MASK	0xf
#define MHARTID_CORE_SHIFT	4
#define MHARTID_CORE_MASK	0xff
#define MHARTID_CLUSTER_SHIFT	16
#define MHARTID_CLUSTER_MASK	0xf

#define MARCHID_UARCH_SHIFT	0
#define MARCHID_UARCH_MASK	0xff
#define MARCHID_CLASS_SHIFT	8
#define MARCHID_CLASS_MASK	0xff
#define MARCHID_CLASS_M		0
#define MARCHID_CLASS_I		1
#define MARCHID_CLASS_P		2

#define CM_BASE_CORE_SHIFT	8
#define CM_BASE_CLUSTER_SHIFT	19

#define CCA_CACHE_ENABLE	0
#define CCA_BUFFER_CACHE	1
#define CCA_CACHE_DISABLE	2
#define CCA_UNCACHE_ACC		3

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

#define GCR_L2_CONFIG_OFF	0x0130
#define L2_LINE_SIZE_SHIFT	8
#define L2_LINE_SIZE_MASK	0xf

#define PMP_R			0x01
#define PMP_W			0x02
#define PMP_X			0x04
#define PMP_TOR			0x8
#define PMP_NA4			0x10
#define PMP_NAPOT		0x18

/* CPC Block offsets */
#define CPC_OFF_LOCAL		0x2000

#define CPC_PWRUP_CTL		0x0030

#define CPC_SYS_CONFIG		0x0140

#define CPC_Cx_CMD		0x0000
#define CPC_Cx_CMD_RESET	0x4

#define P8700_GCR_C0_COH_EN	0x20f8
#define P8700_GCR_C1_COH_EN	0x21f8
#define P8700_GCR_C2_COH_EN	0x22f8
#define P8700_GCR_C3_COH_EN	0x23f8
#define P8700_GCR_C4_COH_EN	0x24f8
#define P8700_GCR_C5_COH_EN	0x25f8

#define GCR_CL_COH_EN		0x2008
#define GCR_CL_COH_EN_EN	(0x1 << 0)
#define GCR_BASE_OFFSET		0x0008
#define CPC_BASE_OFFSET		0x0088

#endif /* __P8700_H__ */
