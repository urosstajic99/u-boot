/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021, Chao-ying Fu <cfu@mips.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <elf.h>
#include <system-constants.h>
#include <asm/encoding.h>
#include <generated/asm-offsets.h>
#include <asm/arch-p8700/p8700.h>

.global p8700_platform_setup
.global set_flash_uncached

p8700_platform_setup:
	move s6, ra
	li	x1,0
	li	x2,0
	li	x3,0
	li	x4,0
	li	x5,0
	li	x6,0
	li	x7,0
	li	x8,0
	li	x9,0
	li	x10,0
	li	x11,0
	li	x12,0
	li	x13,0
	li	x14,0
	li	x15,0
	li	x16,0
	li	x17,0
	li	x18,0
	li	x19,0
	li	x20,0
	li	x21,0
	li	x23,0
	li	x24,0
	li	x25,0
	li	x26,0
	li	x27,0
	li	x28,0
	li	x29,0
	li	x30,0
	li	x31,0

	/* a0 has mhartid */
	csrr	a0, CSR_MHARTID

	/* Test mhartid */
	bne	a0, zero, ENDIAN_OK
1:

	/* Check mstatus bit 37 as the endianness */
	csrr	s2, mstatus
	srli	s2, s2, 37
	andi	s2, s2, 1
	bne	s2, zero, DDR_INIT_DONE

	li	t0, BOSTON_PLAT_DDR3STAT
1:	lw	t1, 0(t0)
	andi	t1, t1, BOSTON_PLAT_DDR3STAT_CALIB
	beqz	t1, 1b

DDR_INIT_DONE:
#ifdef __riscv_big_endian
	bne	s2, zero, ENDIAN_OK
#else
	beq	s2, zero, ENDIAN_OK

#endif
SWITCH_ENDIAN:
	li	s0, CM_BASE
	li	t0, 0x1fb80000
	li	t1, CM_BASE
	sd	t1, GCR_BASE_OFFSET(t0)

	/* Write the new endianness */
	li	s1, CPC_BASE
	xori	t0, s2, 1
	sd	t0, CPC_SYS_CONFIG(s1)
	fence

	/* Reset */
	li	t0, CPC_Cx_CMD_RESET
	INDEXED(sd, t0, t1, CPC_OFF_LOCAL + CPC_Cx_CMD, s1)
	fence
1:
	j	1b
ENDIAN_OK:

	/* Test mhartid lowest 4 bits */
	andi	t0, a0, 0xf
	bne	t0, zero, setup_pmp

	li	s0, CM_BASE
	li	t0, 0x1fb80000
	/* Get cluster number to update CM_BASE */
	srl	t1, a0, MHARTID_CLUSTER_SHIFT
	andi	t1, t1, MHARTID_CLUSTER_MASK
	sll	t1, t1, CM_BASE_CLUSTER_SHIFT
	add	s0, s0, t1
	move	t1, s0

	/* Test mhartid lowest 16 bits */
	li	t2, 0xffff
	and	t2, a0, t2
	bne	t2, zero, cm_relocate_done
	sd	t1, GCR_BASE_OFFSET(t0)
cm_relocate_done:

	li	t0, GCR_CL_COH_EN_EN

	/* Get core number to update CM_BASE */
	srl	t1, a0, MHARTID_CORE_SHIFT
	andi	t1, t1, MHARTID_CORE_MASK
	sll	t1, t1, CM_BASE_CORE_SHIFT
	add	s0, s0, t1
	INDEXED(sd, t0, t1, P8700_GCR_C0_COH_EN, s0)
3:

	/* Test mhartid */
	bne	a0, zero, setup_pmp

	/* Map all PCIe DMA access to its default, non-IOCU, target */
	li	t0,BOSTON_PLAT_NOCPCIE0ADDR
	sw	zero, 0(t0)
	li	t0,BOSTON_PLAT_NOCPCIE1ADDR
	sw	zero, 0(t0)
	li	t0,BOSTON_PLAT_NOCPCIE2ADDR
	sw	zero, 0(t0)

setup_pmp:
	/* Setup PMP */
	li	t0, 0x2fffffff # 2G from 0x80000000
	csrw	pmpaddr1, t0
	li	t0, 0x07ff7fff # 0x40000 from 0x1ffc0000
	csrw	pmpaddr2, t0
	li	t0, 0x07f3ffff # 2M from 0x1fc00000
	csrw	pmpaddr3, t0
	li	t0, 0x1fffffffffffffff # All from 0x0
	csrw	pmpaddr4, t0
	li	t0, ((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 32) | \
			((PMP_NAPOT | PMP_R | PMP_X) << 24) | \
			((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 16) | \
			((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 8)
	csrw	pmpcfg0, t0

	/* Test mhartid */
	bne	a0, zero, 2f
	/* HART 0: Set cacheable for pmp0, pmp1, pmp3, uncacheable for pmp2, pmp4 */
	li	t0, (CCA_CACHE_DISABLE << 32) | (CCA_CACHE_ENABLE << 24) | \
			(CCA_CACHE_DISABLE << 16) | (CCA_CACHE_ENABLE << 8) | CCA_CACHE_ENABLE
	j	3f
2:	/* HART 1 or above: Set cacheable for pmp0, pmp1, uncacheable for pmp2, pmp3, pmp4 */
	li	t0, (CCA_CACHE_DISABLE << 32) | (CCA_CACHE_DISABLE << 24) | \
			(CCA_CACHE_DISABLE << 16) | (CCA_CACHE_ENABLE << 8) | CCA_CACHE_ENABLE
3:
	csrw	CSR_PMACFG0, t0
	fence

	/* Test mhartid */
	beq	a0, zero, 1f
	/* Jump to 0x80000000 */
	li	t0, 0x80000000
	jr	t0
1:
	move ra, s6
	li s6, 0
	ret

set_flash_uncached:
	addi	sp, sp, -16
	sd	 s6, 0(sp)
	move s6, ra

	/* Set flash uncached at pmp3 */
	csrr	t0, CSR_PMACFG0
	li	t1, 0xffffffffffffffff - (0xff << 24)
	and	t0, t0, t1
	li	t1, (CCA_CACHE_DISABLE << 24)
	or	t0, t0, t1
	csrw	CSR_PMACFG0, t0
	fence

	move ra, s6
	ld	s6, 0(sp)
	addi	sp, sp, 16
	ret
