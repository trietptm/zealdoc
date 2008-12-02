
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               kernel.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
;	zealcook modify, 2008
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SELECTOR_KERNEL_CS	equ	8

; import function
extern	cstart
extern	exception_handler
extern	spurious_irq
; no param
extern  test_str

; import
extern	gdt_ptr
extern	idt_ptr
extern	disp_pos

bits 32

[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; stack top

[section .text]	; code section

; export function
global _start

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15

_start:
	; ��ʱ�ڴ濴��ȥ�������ģ�����ϸ���ڴ������ LOADER.ASM ����˵������
	;              ��                  ��
	;              �� ...              ��
	;              �ǩ�������������������������������������
	;              ��Page  Tables      ��
	;              ��(��С��LOADER����)�� PageTblBase
	;    00101000h �ǩ�������������������������������������
	;              ��PageDirectoryTable�� PageDirBase = 1M
	;    00100000h �ǩ�������������������������������������
	;              ��Hardware  Reserved�� B8000h �� gs
	;       9FC00h �ǩ�������������������������������������
	;              ��    LOADER.BIN    �� somewhere in LOADER �� esp
	;       90000h �ǩ�������������������������������������
	;              ��    KERNEL.BIN    ��
	;       80000h �ǩ�������������������������������������
	;              ��      KERNEL      �� 30400h �� KERNEL ��� (KernelEntryPointPhyAddr)
	;       30000h �ǩ�������������������������������������
	;              | ...              | 
	;              | ...              |
	;           0h ���������������������������������������� �� cs, ds, es, fs, ss
	;
	;
	; GDT �Լ���Ӧ���������������ģ�
	;
	;		              Descriptors               Selectors
	;              ����������������������������������������
	;              ��Dummy Descriptor  ��
	;              �ǩ�������������������������������������
	;              ��DESC_FLAT_C    (0��4G)     ��   8h = cs
	;              �ǩ�������������������������������������
	;              ��DESC_FLAT_RW   (0��4G)     ��  10h = ds, es, fs, ss
	;              �ǩ�������������������������������������
	;              ��DESC_VIDEO        ��  1Bh = gs
	;              ����������������������������������������
	;
	; ע��! ��ʹ�� C �����ʱ��һ��Ҫ��֤ ds, es, ss �⼸���μĴ�����ֵ��һ����
	; ��Ϊ�������п��ܱ����ʹ�����ǵĴ���, ��������Ĭ��������һ����. ���紮�����������õ� ds �� es.
	;
	;


	; �� esp �� LOADER Ų�� KERNEL
	mov	esp, StackTop	; ��ջ�� bss ����

	mov	dword [disp_pos], 0

	; sidt: store 48-bit BASE/LIMIT IDTR to m
	; sgdt: store 48-bit BASE/LIMIT GDTR to m, here is gdt_ptr
	sgdt	[gdt_ptr]	; cstart() will use gdt_ptr

	call	cstart		; this func make gdt_ptr point to the new GDT,
				; then call init_prot
	
; lgdt: load m into GDTR
; lidt: load m into IDTR
;
; http://pdos.csail.mit.edu/6.828/2005/readings/i386/LGDT.htm
;	IF instruction = LIDT
;	THEN
;		IF OperandSize = 16
;		THEN IDTR.Limit:Base := m16:24 (* 24 bits of base loaded *)
;		ELSE IDTR.Limit:Base := m16:32
;		FI;
;	ELSE (* instruction = LGDT *)
;		IF OperandSize = 16
;		THEN GDTR.Limit:Base := m16:24 (* 24 bits of base loaded *)
;		ELSE GDTR.Limit:Base := m16:32;
;		FI;
;	FI;
;
	lgdt	[gdt_ptr]	; load new GDT

	lidt	[idt_ptr]

	jmp	SELECTOR_KERNEL_CS:csinit
csinit:		; �������תָ��ǿ��ʹ�øոճ�ʼ���Ľṹ������<<OS:D&I 2nd>> P90.

	;jmp 0x40:0
	;ud2

	sti
HALT:
	call	test_str
	hlt
	jmp	HALT	
; interrupt and exception -- hw interrupt
; ---------------------------------
%macro	hwint_master	1
	push	%1
	call	spurious_irq
	add	esp, 4
;	hlt	; do not die
	ret
%endmacro

ALIGN	16
hwint00:		; Interrupt routine for irq 0 (the clock).
	hwint_master	0

ALIGN	16
hwint01:		; Interrupt routine for irq 1 (keyboard)
	hwint_master	1

ALIGN	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
	hwint_master	2

ALIGN	16
hwint03:		; Interrupt routine for irq 3 (second serial)
	hwint_master	3

ALIGN	16
hwint04:		; Interrupt routine for irq 4 (first serial)
	hwint_master	4

ALIGN	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwint_master	5

ALIGN	16
hwint06:		; Interrupt routine for irq 6 (floppy)
	hwint_master	6

ALIGN	16
hwint07:		; Interrupt routine for irq 7 (printer)
	hwint_master	7

; ---------------------------------
%macro	hwint_slave	1
	push	%1
	call	spurious_irq
	add	esp, 4
	hlt
%endmacro
; ---------------------------------

ALIGN	16
hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwint_slave	8

ALIGN	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
	hwint_slave	9

ALIGN	16
hwint10:		; Interrupt routine for irq 10
	hwint_slave	10

ALIGN	16
hwint11:		; Interrupt routine for irq 11
	hwint_slave	11

ALIGN	16
hwint12:		; Interrupt routine for irq 12
	hwint_slave	12

ALIGN	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwint_slave	13

ALIGN	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave	14

ALIGN	16
hwint15:		; Interrupt routine for irq 15
	hwint_slave	15



; �жϺ��쳣 -- �쳣
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2	; ��ջ��ָ�� EIP����ջ�дӶ����������ǣ�EIP��CS��EFLAGS
	hlt
