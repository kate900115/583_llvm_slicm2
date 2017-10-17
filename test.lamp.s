	.file	"<stdin>"
	.text
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp3:
	.cfi_def_cfa_offset 16
.Ltmp4:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp5:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%rbx
	subq	$408, %rsp              # imm = 0x198
.Ltmp6:
	.cfi_offset %rbx, -40
.Ltmp7:
	.cfi_offset %r14, -32
.Ltmp8:
	.cfi_offset %r15, -24
	movl	$9, %edi
	movl	$1, %esi
	movl	$1, %edx
	xorl	%ecx, %ecx
	callq	LAMP_init
	leaq	-28(%rbp), %rsi
	xorl	%edi, %edi
	xorl	%edx, %edx
	callq	LAMP_store4
	movl	$0, -28(%rbp)
	leaq	-32(%rbp), %r14
	movl	$1, %edi
	movq	%r14, %rsi
	xorl	%edx, %edx
	callq	LAMP_store4
	movl	$0, -32(%rbp)
	movl	$9, %edi
	callq	LAMP_loop_invocation
	jmp	.LBB0_1
	.align	16, 0x90
.LBB0_2:                                # %for.body
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$3, %edi
	movq	%r14, %rsi
	callq	LAMP_load4
	movl	-32(%rbp), %r15d
	movl	$4, %edi
	movq	%r14, %rsi
	callq	LAMP_load4
	movl	-32(%rbp), %ebx
	movl	$5, %edi
	movq	%r14, %rsi
	callq	LAMP_load4
	imull	%r15d, %ebx
	movslq	%ebx, %r15
	movslq	-32(%rbp), %rbx
	leaq	-432(%rbp,%rbx,4), %rsi
	movl	$6, %edi
	movq	%r15, %rdx
	callq	LAMP_store4
	movl	%r15d, -432(%rbp,%rbx,4)
	movl	$7, %edi
	movq	%r14, %rsi
	callq	LAMP_load4
	movl	-32(%rbp), %eax
	incl	%eax
	movslq	%eax, %rbx
	movl	$8, %edi
	movq	%r14, %rsi
	movq	%rbx, %rdx
	callq	LAMP_store4
	movl	%ebx, -32(%rbp)
	callq	LAMP_loop_iteration_end
.LBB0_1:                                # %for.cond
                                        # =>This Inner Loop Header: Depth=1
	callq	LAMP_loop_iteration_begin
	movl	$2, %edi
	movq	%r14, %rsi
	callq	LAMP_load4
	cmpl	$99, -32(%rbp)
	jle	.LBB0_2
# BB#3:                                 # %for.end
	callq	LAMP_loop_iteration_end
	callq	LAMP_loop_exit
	xorl	%eax, %eax
	addq	$408, %rsp              # imm = 0x198
	popq	%rbx
	popq	%r14
	popq	%r15
	popq	%rbp
	ret
.Ltmp9:
	.size	main, .Ltmp9-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
