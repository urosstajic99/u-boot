        .text

        .align 2
	.globl	atomic_swap_w
atomic_swap_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a1,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_swap_d
atomic_swap_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a1,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_add_w
atomic_add_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	addw	a3,a5,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_add_d
atomic_add_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	add	a3,a5,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_and_w
atomic_and_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	and	a3,a5,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_and_d
atomic_and_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	and	a3,a5,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_or_w
atomic_or_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	or	a3,a5,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_or_d
atomic_or_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	or	a3,a5,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_xor_w
atomic_xor_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	xor	a3,a5,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_xor_d
atomic_xor_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	xor	a3,a5,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_max_w
atomic_max_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bge	a5,a1,1f
	mv	a3,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_max_d
atomic_max_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bge	a5,a1,1f
	mv	a3,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_maxu_w
atomic_maxu_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bgeu	a5,a1,1f
	mv	a3,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_maxu_d
atomic_maxu_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bgeu	a5,a1,1f
	mv	a3,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_min_w
atomic_min_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bge	a1,a5,1f
	mv	a3,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_min_d
atomic_min_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bge	a1,a5,1f
	mv	a3,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_minu_w
atomic_minu_w:
	lw	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bgeu	a1,a5,1f
	mv	a3,a1
1:	lr.w	a5,(a4)
	bne	a5,a0,2b
	sc.w	a6,a3,(a4)
	bnez	a6,1b
	ret

        .align 2
	.globl	atomic_minu_d
atomic_minu_d:
	ld	a5,0(a0)
	mv	a4,a0
2:	mv	a0,a5
	mv	a3,a5
	bgeu	a1,a5,1f
	mv	a3,a1
1:	lr.d	a5,(a4)
	bne	a5,a0,2b
	sc.d	a6,a3,(a4)
	bnez	a6,1b
	ret
