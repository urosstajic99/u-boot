// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-17 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * Copyright (C) 2019 Sean Anderson <seanga2@gmail.com>
 */

#include <linux/compat.h>
#include <efi_loader.h>
#include <hang.h>
#include <interrupt.h>
#include <irq_func.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>
#include <semihosting.h>

#define ILLEGAL_INSTRUCTION 2
#define AMO_MASK 0xf800707f
#define AQRL_MASK 0x06000000
#define AQRL_SHIFT 25
#define RS2_MASK 0x01f00000
#define RS2_SHIFT 20
#define RS1_MASK 0x000f8000
#define RS1_SHIFT 15
#define RD_MASK 0x00000f80
#define RD_SHIFT 7

#define AMOADD_D_MATCH 0x0000302f
#define AMOADD_W_MATCH 0x0000202f
#define AMOAND_D_MATCH 0x6000302f
#define AMOAND_W_MATCH 0x6000202f
#define AMOMAX_D_MATCH 0xa000302f
#define AMOMAX_W_MATCH 0xa000202f
#define AMOMAXU_D_MATCH 0xe000302f
#define AMOMAXU_W_MATCH 0xe000202f
#define AMOMIN_D_MATCH 0x8000302f
#define AMOMIN_W_MATCH 0x8000202f
#define AMOMINU_D_MATCH 0xc000302f
#define AMOMINU_W_MATCH 0xc000202f
#define AMOOR_D_MATCH 0x4000302f
#define AMOOR_W_MATCH 0x4000202f
#define AMOSWAP_D_MATCH 0x0800302f
#define AMOSWAP_W_MATCH 0x0800202f
#define AMOXOR_D_MATCH 0x2000302f
#define AMOXOR_W_MATCH 0x2000202f

DECLARE_GLOBAL_DATA_PTR;

void set_resume(struct resume_data *data)
{
	gd->arch.resume = data;
}

static void show_efi_loaded_images(uintptr_t epc)
{
	efi_print_image_infos((void *)epc);
}

static void __maybe_unused show_regs(struct pt_regs *regs)
{
	printf("\nSP:  " REG_FMT " GP:  " REG_FMT " TP:  " REG_FMT "\n",
	       regs->sp, regs->gp, regs->tp);
	printf("T0:  " REG_FMT " T1:  " REG_FMT " T2:  " REG_FMT "\n",
	       regs->t0, regs->t1, regs->t2);
	printf("S0:  " REG_FMT " S1:  " REG_FMT " A0:  " REG_FMT "\n",
	       regs->s0, regs->s1, regs->a0);
	printf("A1:  " REG_FMT " A2:  " REG_FMT " A3:  " REG_FMT "\n",
	       regs->a1, regs->a2, regs->a3);
	printf("A4:  " REG_FMT " A5:  " REG_FMT " A6:  " REG_FMT "\n",
	       regs->a4, regs->a5, regs->a6);
	printf("A7:  " REG_FMT " S2:  " REG_FMT " S3:  " REG_FMT "\n",
	       regs->a7, regs->s2, regs->s3);
	printf("S4:  " REG_FMT " S5:  " REG_FMT " S6:  " REG_FMT "\n",
	       regs->s4, regs->s5, regs->s6);
	printf("S7:  " REG_FMT " S8:  " REG_FMT " S9:  " REG_FMT "\n",
	       regs->s7, regs->s8, regs->s9);
	printf("S10: " REG_FMT " S11: " REG_FMT " T3:  " REG_FMT "\n",
	       regs->s10, regs->s11, regs->t3);
	printf("T4:  " REG_FMT " T5:  " REG_FMT " T6:  " REG_FMT "\n",
	       regs->t4, regs->t5, regs->t6);
}

static void __maybe_unused show_backtrace(struct pt_regs *regs)
{
	uintptr_t *fp = (uintptr_t *)regs->s0;
	unsigned count = 0;
	ulong ra;

	printf("\nbacktrace:\n");

	/* there are a few entry points where the s0 register is
	 * set to gd, so to avoid changing those, just abort if
	 * the value is the same */
	while (fp != NULL && fp != (uintptr_t *)gd) {
		ra = fp[-1];
		printf("%3d: FP: " REG_FMT " RA: " REG_FMT,
		       count, (ulong)fp, ra);

		if (gd && gd->flags & GD_FLG_RELOC)
			printf(" - RA: " REG_FMT " reloc adjusted\n",
			ra - gd->reloc_off);
		else
			printf("\n");

		fp = (uintptr_t *)fp[-2];
		count++;
	}
}

/**
 * instr_len() - get instruction length
 *
 * @i:		low 16 bits of the instruction
 * Return:	number of u16 in instruction
 */
static int instr_len(u16 i)
{
	if ((i & 0x03) != 0x03)
		return 1;
	/* Instructions with more than 32 bits are not yet specified */
	return 2;
}

/**
 * show_code() - display code leading to exception
 *
 * @epc:	program counter
 */
static void show_code(ulong epc)
{
	u16 *pos = (u16 *)(epc & ~1UL);
	int i, len = instr_len(*pos);

	printf("\nCode: ");
	for (i = -8; i; ++i)
		printf("%04x ", pos[i]);
	printf("(");
	for (i = 0; i < len; ++i)
		printf("%04x%s", pos[i], i + 1 == len ? ")\n" : " ");
}

static char *reg_names[32] =
{
	"zero",
	"ra",
	"sp",
	"gp",
	"tp",
	"t0",
	"t1",
	"t2",
	"s0",
	"s1",
	"a0",
	"a1",
	"a2",
	"a3",
	"a4",
	"a5",
	"a6",
	"a7",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"s8",
	"s9",
	"s10",
	"s11",
	"t3",
	"t4",
	"t5",
	"t6"
};

static long get_reg(struct pt_regs *regs, int reg_num)
{
	switch (reg_num) {
		case 0:
			return 0;
		case 1:
			return regs->ra;
		case 2:
			return regs->sp;
		case 3:
			return regs->gp;
		case 4:
			return regs->tp;
		case 5:
			return regs->t0;
		case 6:
			return regs->t1;
		case 7:
			return regs->t2;
		case 8:
			return regs->s0;
		case 9:
			return regs->s1;
		case 10:
			return regs->a0;
		case 11:
			return regs->a1;
		case 12:
			return regs->a2;
		case 13:
			return regs->a3;
		case 14:
			return regs->a4;
		case 15:
			return regs->a5;
		case 16:
			return regs->a6;
		case 17:
			return regs->a7;
		case 18:
			return regs->s2;
		case 19:
			return regs->s3;
		case 20:
			return regs->s4;
		case 21:
			return regs->s5;
		case 22:
			return regs->s6;
		case 23:
			return regs->s7;
		case 24:
			return regs->s8;
		case 25:
			return regs->s9;
		case 26:
			return regs->s10;
		case 27:
			return regs->s11;
		case 28:
			return regs->t3;
		case 29:
			return regs->t4;
		case 30:
			return regs->t5;
		case 31:
			return regs->t6;
		default:
			printf("Error reg_num=%d for get_reg\n", reg_num);
			break;
	}
	return 0;
}

static void set_reg(struct pt_regs *regs, int reg_num, long reg_value)
{
	switch (reg_num) {
		case 0:
			break;
		case 1:
			regs->ra = reg_value;
			break;
		case 2:
			regs->sp = reg_value;
			break;
		case 3:
			regs->gp = reg_value;
			break;
		case 4:
			regs->tp = reg_value;
			break;
		case 5:
			regs->t0 = reg_value;
			break;
		case 6:
			regs->t1 = reg_value;
			break;
		case 7:
			regs->t2 = reg_value;
			break;
		case 8:
			regs->s0 = reg_value;
			break;
		case 9:
			regs->s1 = reg_value;
			break;
		case 10:
			regs->a0 = reg_value;
			break;
		case 11:
			regs->a1 = reg_value;
			break;
		case 12:
			regs->a2 = reg_value;
			break;
		case 13:
			regs->a3 = reg_value;
			break;
		case 14:
			regs->a4 = reg_value;
			break;
		case 15:
			regs->a5 = reg_value;
			break;
		case 16:
			regs->a6 = reg_value;
			break;
		case 17:
			regs->a7 = reg_value;
			break;
		case 18:
			regs->s2 = reg_value;
			break;
		case 19:
			regs->s3 = reg_value;
			break;
		case 20:
			regs->s4 = reg_value;
			break;
		case 21:
			regs->s5 = reg_value;
			break;
		case 22:
			regs->s6 = reg_value;
			break;
		case 23:
			regs->s7 = reg_value;
			break;
		case 24:
			regs->s8 = reg_value;
			break;
		case 25:
			regs->s9 = reg_value;
			break;
		case 26:
			regs->s10 = reg_value;
			break;
		case 27:
			regs->s11 = reg_value;
			break;
		case 28:
			regs->t3 = reg_value;
			break;
		case 29:
			regs->t4 = reg_value;
			break;
		case 30:
			regs->t5 = reg_value;
			break;
		case 31:
			regs->t6 =reg_value;
			break;
		default:
			printf("Error reg_num=%d for set_reg\n", reg_num);
			break;
	}
}

static long _exit_trap(ulong code, ulong epc, ulong tval, struct pt_regs *regs)
{
	static const char * const exception_code[] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode",
		"Environment call from S-mode",
		"Reserved",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault",
	};

	if (gd->arch.resume) {
		gd->arch.resume->code = code;
		longjmp(gd->arch.resume->jump, 1);
	}
	if (code == ILLEGAL_INSTRUCTION) {
		int opcode = *(int *)epc;
		int aqrl = (opcode & AQRL_MASK) >> AQRL_SHIFT;
		int rs2 = (opcode & RS2_MASK) >> RS2_SHIFT;
		int rs1 = (opcode & RS1_MASK) >> RS1_SHIFT;
		int rd = (opcode & RD_MASK) >> RD_SHIFT;
		long rs2_value = get_reg(regs, rs2);
		long rs1_value = get_reg(regs, rs1);
		long rd_value = 0;
		switch (opcode & AMO_MASK) {
		case AMOADD_D_MATCH:
			rd_value = atomic_add_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOADD_W_MATCH:
			rd_value = atomic_add_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOAND_D_MATCH:
			rd_value = atomic_and_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOAND_W_MATCH:
			rd_value = atomic_and_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMAX_D_MATCH:
			rd_value = atomic_max_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMAX_W_MATCH:
			rd_value = atomic_max_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMAXU_D_MATCH:
			rd_value = atomic_maxu_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMAXU_W_MATCH:
			rd_value = atomic_maxu_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMIN_D_MATCH:
			rd_value = atomic_min_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMIN_W_MATCH:
			rd_value = atomic_min_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMINU_D_MATCH:
			rd_value = atomic_minu_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOMINU_W_MATCH:
			rd_value = atomic_minu_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOOR_D_MATCH:
			rd_value = atomic_or_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOOR_W_MATCH:
			rd_value = atomic_or_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOSWAP_D_MATCH:
			rd_value = atomic_swap_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOSWAP_W_MATCH:
			rd_value = atomic_swap_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOXOR_D_MATCH:
			rd_value = atomic_xor_d(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		case AMOXOR_W_MATCH:
			rd_value = atomic_xor_w(rs1_value, rs2_value);
			set_reg(regs, rd, rd_value);
			return epc + 4;

		default:
			break;
		}
	}

	if (code < ARRAY_SIZE(exception_code))
		printf("Unhandled exception: %s\n", exception_code[code]);
	else
		printf("Unhandled exception code: %ld\n", code);

	printf("EPC: " REG_FMT " RA: " REG_FMT " TVAL: " REG_FMT "\n",
	       epc, regs->ra, tval);
	/* Print relocation adjustments, but only if gd is initialized */
	if (gd && gd->flags & GD_FLG_RELOC)
		printf("EPC: " REG_FMT " RA: " REG_FMT " reloc adjusted\n",
		       epc - gd->reloc_off, regs->ra - gd->reloc_off);

	if (CONFIG_IS_ENABLED(SHOW_REGS))
		show_regs(regs);
	if (CONFIG_IS_ENABLED(FRAMEPOINTER))
		show_backtrace(regs);
	show_code(epc);
	show_efi_loaded_images(epc);
	panic("\n");
}

int interrupt_init(void)
{
	return 0;
}

/*
 * enable interrupts
 */
void enable_interrupts(void)
{
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
	return 0;
}

ulong handle_trap(ulong cause, ulong epc, ulong tval, struct pt_regs *regs)
{
	ulong is_irq, irq;

	/* An UEFI application may have changed gd. Restore U-Boot's gd. */
	efi_restore_gd();

	if (cause == CAUSE_BREAKPOINT &&
	    CONFIG_IS_ENABLED(SEMIHOSTING_FALLBACK)) {
		ulong pre_addr = epc - 4, post_addr = epc + 4;

		/* Check for prior and post addresses to be in same page. */
		if ((pre_addr & ~(PAGE_SIZE - 1)) ==
			(post_addr & ~(PAGE_SIZE - 1))) {
			u32 pre = *(u32 *)pre_addr;
			u32 post = *(u32 *)post_addr;

			/* Check for semihosting, i.e.:
			 * slli    zero,zero,0x1f
			 * ebreak
			 * srai    zero,zero,0x7
			 */
			if (pre == 0x01f01013 && post == 0x40705013) {
				disable_semihosting();
				epc += 4;
				return epc;
			}
		}
	}

	is_irq = (cause & MCAUSE_INT);
	irq = (cause & ~MCAUSE_INT);

	if (is_irq) {
		switch (irq) {
		case IRQ_M_EXT:
		case IRQ_S_EXT:
			external_interrupt(0);	/* handle external interrupt */
			break;
		case IRQ_M_TIMER:
		case IRQ_S_TIMER:
			timer_interrupt(0);	/* handle timer interrupt */
			break;
		default:
			_exit_trap(cause, epc, tval, regs);
			break;
		};
	} else {
		_exit_trap(cause, epc, tval, regs);
	}

	return epc;
}

/*
 *Entry Point for PLIC Interrupt Handler
 */
__attribute__((weak)) void external_interrupt(struct pt_regs *regs)
{
}

__attribute__((weak)) void timer_interrupt(struct pt_regs *regs)
{
}
