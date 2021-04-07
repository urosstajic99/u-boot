/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Startup Code for RISC-V Core
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Copyright (c) 2017 Padmarao Begari <Padmarao.Begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <elf.h>
#include <system-constants.h>
#include <asm/encoding.h>
#include <generated/asm-offsets.h>
#include <asm/arch-shogun/shogun.h>

#ifdef CONFIG_32BIT
#define LREG			lw
#define SREG			sw
#define REGBYTES		4
#define RELOC_TYPE		R_RISCV_32
#define SYM_INDEX		0x8
#define SYM_SIZE		0x10
#else
#define LREG			ld
#define SREG			sd
#define REGBYTES		8
#define RELOC_TYPE		R_RISCV_64
#define SYM_INDEX		0x20
#define SYM_SIZE		0x18
#endif

#define PMP_R			0x01
#define PMP_W			0x02
#define PMP_X			0x04
#define PMP_TOR			0x8
#define PMP_NA4			0x10
#define PMP_NAPOT		0x18

#define CM_BASE			0x16100000
#define GIC_BASE		0x16120000
#define CPC_BASE		0x16200000
#define GCR_CL_COH_EN		0x2008
#define GCR_CL_COH_EN_EN	(0x1 << 0)
#define GCR_BASE_OFFSET		0x0008
#define GIC_BASE_OFFSET		0x0080
#define CPC_BASE_OFFSET		0x0088
#define ENABLE			0x1
#define COUNT_STOP		(0x1 << 28)
#define INDEXED(op, reg, idx, offset, base) \
	li	idx, offset	;\
	add	idx, idx, base	;\
	op	reg, (idx)

#define BOSTON_PLAT_BASE	0x17ffd000
#define BOSTON_PLAT_DDR3STAT	(BOSTON_PLAT_BASE + 0x14)
#define BOSTON_PLAT_DDR3STAT_CALIB	(1 << 2)
#define BOSTON_PLAT_NOCPCIE0ADDR        (BOSTON_PLAT_BASE + 0x3c)
#define BOSTON_PLAT_NOCPCIE1ADDR        (BOSTON_PLAT_BASE + 0x40)
#define BOSTON_PLAT_NOCPCIE2ADDR        (BOSTON_PLAT_BASE + 0x44)

.section .data
secondary_harts_relocation_error:
	.ascii "Relocation of secondary harts has failed, error %d\n"

.section .text
.globl _start
_start:
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
	li	x22,0
	li	x23,0
	li	x24,0
	li	x25,0
	li	x26,0
	li	x27,0
	li	x28,0
	li	x29,0
	li	x30,0
	li	x31,0

	li	t0, BOSTON_PLAT_DDR3STAT
1:	lw	t1, 0(t0)
	andi	t1, t1, BOSTON_PLAT_DDR3STAT_CALIB
	beqz	t1, 1b

	li	s0, CM_BASE
	li	t0, 0x1fbf8000

	li	t1, CM_BASE
	sd	t1, GCR_BASE_OFFSET(t0)

	li	t0, GCR_CL_COH_EN_EN
	INDEXED(sd, t0, t1, GCR_CL_COH_EN, s0)

	li	t0, GIC_BASE + ENABLE
	INDEXED(sd, t0, t1, GIC_BASE_OFFSET, s0)

	li	t0, CPC_BASE + ENABLE
	INDEXED(sd, t0, t1, CPC_BASE_OFFSET, s0)

	/* Clear bit 28 at GIC_Base to enable counter */
	li	t0, GIC_BASE
	ld	t1, 0(t0)
	li	t2, COUNT_STOP
	not	t2, t2
	and	t1, t1, t2
	sd	t1, 0(t0)

	/* Map all PCIe DMA access to its default, non-IOCU, target */
	li	t0,BOSTON_PLAT_NOCPCIE0ADDR
	sw	zero, 0(t0)
	li	t0,BOSTON_PLAT_NOCPCIE1ADDR
	sw	zero, 0(t0)
	li	t0,BOSTON_PLAT_NOCPCIE2ADDR
	sw	zero, 0(t0)

	/* Setup PMP */
	li	t0, 0x2fffffff # 2G from 0x80000000
	csrw	pmpaddr1, t0
	li	t0, 0x1fffffffffffffff # All from 0x0
	csrw	pmpaddr2, t0
	li	t0, ((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 16) | ((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 8)
	csrw	pmpcfg0, t0
	/* Set up PMP Uncacheable for pmp0, pmp1, pmp2 */
	li	t0, (CCA_CACHE_DISABLE << 16) | (CCA_CACHE_DISABLE << 8) | CCA_CACHE_DISABLE
	csrw	pmacfg0, t0
	fence

#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csrr	a0, CSR_MHARTID
#endif

	/*
	 * Save hart id and dtb pointer. The thread pointer register is not
	 * modified by C code. It is used by secondary_hart_loop.
	 */
	mv	tp, a0
	mv	s1, a1

	/*
	 * Set the global data pointer to a known value in case we get a very
	 * early trap. The global data pointer will be set its actual value only
	 * after it has been initialized.
	 */
	mv	gp, zero

	/*
	 * Set the trap handler. This must happen after initializing gp because
	 * the handler may use it.
	 */
	la	t0, trap_entry
	csrw	MODE_PREFIX(tvec), t0

	/*
	 * Mask all interrupts. Interrupts are disabled globally (in m/sstatus)
	 * for U-Boot, but we will need to read m/sip to determine if we get an
	 * IPI
	 */
	csrw	MODE_PREFIX(ie), zero

#if CONFIG_IS_ENABLED(SMP)
	/* check if hart is within range */
	/* tp: hart id */
	li	t0, CONFIG_NR_CPUS
	bge	tp, t0, hart_out_of_bounds_loop

	/* set xSIE bit to receive IPIs */
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	li	t0, MIE_MSIE
#else
	li	t0, SIE_SSIE
#endif
	csrs	MODE_PREFIX(ie), t0
#endif

/*
 * Set stackpointer in internal/ex RAM to call board_init_f
 */
call_board_init_f:
#if CONFIG_IS_ENABLED(HAVE_INIT_STACK)
	li	t0, CONFIG_VAL(STACK)
#else
	li	t0, SYS_INIT_SP_ADDR
#endif
	and	t0, t0, -16		/* force 16 byte alignment */

	/* setup stack */
#if CONFIG_IS_ENABLED(SMP)
	/* tp: hart id */
	slli	t1, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, t0, t1
#else
	mv	sp, t0
#endif
/*
 * Now sp points to the right stack belonging to current CPU.
 * It's essential before any function call, otherwise, we get data-race.
 */

/* clear stack if necessary */
#if CONFIG_IS_ENABLED(ZERO_MEM_BEFORE_USE)
clear_stack:
	li	t1, 1
	slli	t1, t1, CONFIG_STACK_SIZE_SHIFT
	sub	t1, sp, t1
clear_stack_loop:
	SREG	zero, 0(t1)		/* t1 is always 16 byte aligned */
	addi	t1, t1, REGBYTES
	blt	t1, sp, clear_stack_loop
#endif

call_board_init_f_0:
	/* find top of reserve space */
#if CONFIG_IS_ENABLED(SMP)
	li	t1, CONFIG_NR_CPUS
#else
	li	t1, 1
#endif
	slli	t1, t1, CONFIG_STACK_SIZE_SHIFT
	sub	a0, t0, t1		/* t1 -> size of all CPU stacks */
	jal	board_init_f_alloc_reserve

	/*
	 * Save global data pointer for later. We don't set it here because it
	 * is not initialized yet.
	 */
	mv	s0, a0


	/* Configure proprietary settings and customized CSRs of harts */
call_harts_early_init:
	jal	harts_early_init

#if !CONFIG_IS_ENABLED(XIP)
	/*
	 * Pick hart to initialize global data and run U-Boot. The other harts
	 * wait for initialization to complete.
	 */
	la	t0, hart_lottery
	li	t1, 1
	amoswap.w s2, t1, 0(t0)
	bnez	s2, wait_for_gd_init
#else
	/*
	 * FIXME: gp is set before it is initialized. If an XIP U-Boot ever
	 * encounters a pending IPI on boot it is liable to jump to whatever
	 * memory happens to be in ipi_data.addr on boot. It may also run into
	 * problems if it encounters an exception too early (because printf/puts
	 * accesses gd).
	 */
	mv	gp, s0
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	bnez	tp, secondary_hart_loop
#endif
#endif
	
	mv      a0, s0
	jal	board_init_f_init_reserve

	SREG	s1, GD_FIRMWARE_FDT_ADDR(gp)
	/* save the boot hart id to global_data */
	SREG	tp, GD_BOOT_HART(gp)

#if !CONFIG_IS_ENABLED(XIP)
#ifdef CONFIG_AVAILABLE_HARTS
	la	t0, available_harts_lock
	amoswap.w.rl zero, zero, 0(t0)
#endif

wait_for_gd_init:
	/*
	 * Set the global data pointer only when gd_t has been initialized.
	 * This was already set by arch_setup_gd on the boot hart, but all other
	 * harts' global data pointers gets set here.
	 */
	mv	gp, s0
#ifdef CONFIG_AVAILABLE_HARTS
	la	t0, available_harts_lock
	li	t1, 1
1:	amoswap.w.aq t1, t1, 0(t0)
	bnez	t1, 1b

	/* register available harts in the available_harts mask */
	li	t1, 1
	sll	t1, t1, tp
	LREG	t2, GD_AVAILABLE_HARTS(gp)
	or	t2, t2, t1
	SREG	t2, GD_AVAILABLE_HARTS(gp)

	amoswap.w.rl zero, zero, 0(t0)
#endif

	/*
	 * Continue on hart lottery winner, others branch to
	 * secondary_hart_loop.
	 */
	bnez	s2, secondary_hart_loop
#endif

#ifdef CONFIG_DEBUG_UART
	jal	debug_uart_init
#endif

	mv	a0, zero		/* a0 <-- boot_flags = 0 */
	la	t5, board_init_f
	jalr	t5			/* jump to board_init_f() */

#ifdef CONFIG_XPL_BUILD
spl_clear_bss:
	la	t0, __bss_start
	la	t1, __bss_end
	beq	t0, t1, spl_stack_gd_setup

spl_clear_bss_loop:
	SREG	zero, 0(t0)
	addi	t0, t0, REGBYTES
	blt	t0, t1, spl_clear_bss_loop

spl_stack_gd_setup:
	jal	spl_relocate_stack_gd

	/* skip setup if we did not relocate */
	beqz	a0, spl_call_board_init_r
	mv	s0, a0

	/* setup stack on main hart */
#if CONFIG_IS_ENABLED(SMP)
	/* tp: hart id */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, s0, t0
#else
	mv	sp, s0
#endif

#if CONFIG_IS_ENABLED(SMP)
	/* set new stack and global data pointer on secondary harts */
spl_secondary_hart_stack_gd_setup:
	la	a0, secondary_hart_relocate
	mv	a1, s0
	mv	a2, s0
	mv	a3, zero
	jal	smp_call_function

	/* hang if relocation of secondary harts has failed */
	beqz	a0, 1f
	mv	a1, a0
	la	a0, secondary_harts_relocation_error
	jal	printf
	jal	hang
#endif

	/* set new global data pointer on main hart */
1:	mv	gp, s0

spl_call_board_init_r:
	mv	a0, zero
	mv	a1, zero
	j	board_init_r
#endif

#if !defined(CONFIG_XPL_BUILD)
/*
 * void relocate_code(addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl relocate_code
relocate_code:
	mv	s2, a0			/* save addr_sp */
	mv	s3, a1			/* save addr of gd */
	mv	s4, a2			/* save addr of destination */

/*
 *Set up the stack
 */
stack_setup:
#if CONFIG_IS_ENABLED(SMP)
	/* tp: hart id */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, s2, t0
#else
	mv	sp, s2
#endif

	la	t0, _start
	sub	t6, s4, t0		/* t6 <- relocation offset */
	beq	t0, s4, clear_bss	/* skip relocation */

	mv	t1, s4			/* t1 <- scratch for copy_loop */
	la	t2, __bss_start		/* t2 <- source end address */

copy_loop:
	LREG	t5, 0(t0)
	addi	t0, t0, REGBYTES
	SREG	t5, 0(t1)
	addi	t1, t1, REGBYTES
	blt	t0, t2, copy_loop

/*
 * Update dynamic relocations after board_init_f
 */
fix_rela_dyn:
	la	t1, __rel_dyn_start
	la	t2, __rel_dyn_end
	beq	t1, t2, clear_bss
	add	t1, t1, t6		/* t1 <- rela_dyn_start in RAM */
	add	t2, t2, t6		/* t2 <- rela_dyn_end in RAM */

6:
	LREG	t5, REGBYTES(t1)	/* t5 <-- relocation info:type */
	li	t3, R_RISCV_RELATIVE	/* reloc type R_RISCV_RELATIVE */
	bne	t5, t3, 8f		/* skip non-RISCV_RELOC entries */
	LREG	t3, 0(t1)
	LREG	t5, (REGBYTES * 2)(t1)	/* t5 <-- addend */
	add	t5, t5, t6		/* t5 <-- location to fix up in RAM */
	add	t3, t3, t6		/* t3 <-- location to fix up in RAM */
	SREG	t5, 0(t3)
	j	10f

8:
	la	t4, __dyn_sym_start
	add	t4, t4, t6

9:
	srli	t0, t5, SYM_INDEX	/* t0 <--- sym table index */
	andi	t5, t5, 0xFF		/* t5 <--- relocation type */
	li	t3, RELOC_TYPE
	bne	t5, t3, 10f		/* skip non-addned entries */

	LREG	t3, 0(t1)
	li	t5, SYM_SIZE
	mul	t0, t0, t5
	add	s5, t4, t0
	LREG	t0, (REGBYTES * 2)(t1)	/* t0 <-- addend */
	LREG	t5, REGBYTES(s5)
	add	t5, t5, t0
	add	t5, t5, t6		/* t5 <-- location to fix up in RAM */
	add	t3, t3, t6		/* t3 <-- location to fix up in RAM */
	SREG	t5, 0(t3)
10:
	addi	t1, t1, (REGBYTES * 3)
	blt	t1, t2, 6b

/*
 * trap update
*/
	la	t0, trap_entry
	add	t0, t0, t6
	csrw	MODE_PREFIX(tvec), t0

clear_bss:
	la	t0, __bss_start		/* t0 <- rel __bss_start in FLASH */
	add	t0, t0, t6		/* t0 <- rel __bss_start in RAM */
	la	t1, __bss_end		/* t1 <- rel __bss_end in FLASH */
	add	t1, t1, t6		/* t1 <- rel __bss_end in RAM */
	beq	t0, t1, relocate_secondary_harts

clbss_l:
	SREG	zero, 0(t0)		/* clear loop... */
	addi	t0, t0, REGBYTES
	blt	t0, t1, clbss_l

relocate_secondary_harts:
#if CONFIG_IS_ENABLED(SMP)
	/* send relocation IPI */
	la	t0, secondary_hart_relocate
	add	a0, t0, t6

	/* store relocation offset */
	mv	s5, t6

	mv	a1, s2
	mv	a2, s3
	mv	a3, zero
	jal	smp_call_function

	/* hang if relocation of secondary harts has failed */
	beqz	a0, 1f
	mv	a1, a0
	la	a0, secondary_harts_relocation_error
	jal	printf
	jal	hang

	/* restore relocation offset */
1:	mv	t6, s5
#endif

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
call_board_init_r:
	jal	invalidate_icache_all
	jal	flush_dcache_all
	la	t0, board_init_r        /* offset of board_init_r() */
	add	t4, t0, t6		/* real address of board_init_r() */
/*
 * setup parameters for board_init_r
 */
	mv	a0, s3			/* gd_t */
	mv	a1, s4			/* dest_addr */
	mv	s0, zero		/* fp == NULL */

/*
 * jump to it ...
 */
	jr	t4			/* jump to board_init_r() */
#endif /* !defined(CONFIG_XPL_BUILD) */

#if CONFIG_IS_ENABLED(SMP)
hart_out_of_bounds_loop:
	/* Harts in this loop are out of bounds, increase CONFIG_NR_CPUS. */
	wfi
	j	hart_out_of_bounds_loop

/* SMP relocation entry */
secondary_hart_relocate:
	/* a1: new sp */
	/* a2: new gd */
	/* tp: hart id */

	/* setup stack */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, a1, t0

	/* update global data pointer */
	mv	gp, a2
#endif

/*
 * Interrupts are disabled globally, but they can still be read from m/sip. The
 * wfi function will wake us up if we get an IPI, even if we do not trap.
 */
secondary_hart_loop:
	wfi

#if CONFIG_IS_ENABLED(SMP)
	csrr	t0, MODE_PREFIX(ip)
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	andi	t0, t0, MIE_MSIE
#else
	andi	t0, t0, SIE_SSIE
#endif
	beqz	t0, secondary_hart_loop

	mv	a0, tp
	jal	handle_ipi
#endif

	j	secondary_hart_loop

	.globl My_Delay_Loop
My_Delay_Loop:
	li	t0, 0x1fc00000
	sd	zero, 0(t0)
	li	t0, 20000000
1:	addi	t0, t0, -1
	bne	t0, zero, 1b
	li	a0, 0
	jr	ra
	nop
	nop
