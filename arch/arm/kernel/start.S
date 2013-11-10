#include <asm.h>

;.text
.globl _start
_start:
	b	reset
	b	arm_undefined
	b	arm_syscall
	b	arm_prefetch_abort
	b	arm_data_abort
	b	arm_reserved
	b	arm_irq
	b	arm_fiq

reset:
#if defined(CONFIG_CPU_EARLY_INIT)
	bl	cpu_early_init
#endif

#if !defined(CONFIG_ENABLE_NANDWRITE) && defined(CONFIG_CPU_WARM_BOOT)
	ldr 	r0, warm_boot_tag
	cmp 	r0, #1

	/* if set, warm boot */
	ldreq 	pc, =CONFIG_BASE_ADDR

	mov 	r0, #1
	str	r0, warm_boot_tag
#endif

	/* see if we need to relocate */
	mov	r0, pc
	sub	r0, r0, #(.Laddr - _start)
.Laddr:
	ldr	r1, =_start
	cmp	r0, r1
	beq	.Lstack_setup

	/* we need to relocate ourselves to the proper spot */
	ldr	r2, =__data_end	

.Lrelocate_loop:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	cmp	r1, r2
	bne	.Lrelocate_loop

	/* we're relocated, jump to the right address */
	ldr	r0, =.Lstack_setup
	bx	r0

.ltorg
#if defined(CONFIG_CPU_WARM_BOOT)
warm_boot_tag:
	.word 0
#endif

.Lstack_setup:
	/* set up the stack for irq, fiq, abort, undefined,
	 * system/user, and lastly supervisor mode
	 */
	mrs     r0, cpsr
	bic     r0, r0, #0x1f

	ldr	r2, =abort_stack_top

	orr     r1, r0, #0x12	/* irq */
	msr     cpsr_c, r1
	/* save a pointer to a temporary dumping spot used during irq delivery */
//	ldr	r13, =irq_save_spot
	mov	sp, r2
	    
	orr     r1, r0, #0x11	/* fiq */
	msr     cpsr_c, r1
	mov	sp, r2
	            
	orr     r1, r0, #0x17	/* abort */
	msr     cpsr_c, r1
	mov	sp, r2
	    
	orr     r1, r0, #0x1b	/* undefined */
	msr     cpsr_c, r1
	mov		sp, r2
	    
	orr     r1, r0, #0x1f	/* system */
	msr     cpsr_c, r1
	mov	sp, r2

	orr	r1, r0, #0x13	/* supervisor */
	msr	cpsr_c, r1
	mov	sp, r2

	/* copy the initialized data segment out of rom if necessary */
	ldr	r0, =__data_start_rom
	ldr	r1, =__data_start
	ldr	r2, =__data_end

	cmp	r0, r1
	beq	.L__do_bss

.L__copy_loop:
	cmp	r1, r2
	ldrlt	r3, [r0], #4
	strlt	r3, [r1], #4
	blt	.L__copy_loop

.L__do_bss:
	/* clear out the bss */
	ldr	r0, =__bss_start
	ldr	r1, =_end
	mov	r2, #0
.L__bss_loop:
	cmp	r0, r1
	strlt	r2, [r0], #4
	blt	.L__bss_loop

	bl	kmain
	b	.

arm_undefined:
	b	arm_undefined
arm_syscall:
	b	arm_syscall
arm_prefetch_abort:
	b	arm_prefetch_abort
arm_data_abort:
	b	arm_data_abort
arm_reserved:
	b	arm_reserved
arm_irq:
	b	arm_irq
arm_fiq:
	b	arm_fiq

.ltorg

.bss
.align 2
	/* the abort stack is for unrecoverable errors.
	 * also note the initial working stack is set to here.
	 * when the threading system starts up it'll switch to a new 
	 * dynamically allocated stack, so we don't need it for very long
	 */
abort_stack:
	.skip 1024
abort_stack_top:
