#include "kernel.h"

PUBLIC void cstart()
{
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n-----\"cstart\" begins-----\n");

	/* copy GDT in LOADER to the new GDT */
	memcpy(&gdt,				   /* new GDT */
	       (void*)(*((t_32*)(&gdt_ptr[2]))),   /* Base  of Old GDT */
	       *((t_16*)(&gdt_ptr[0])) + 1	   /* Limit of Old GDT */
	);
	/* gdt_ptr[6] has 6 bytes: 0~15:Limit  16~47:Base, as parameter of sgdt and lgdt */
	t_16 *p_gdt_limit = (t_16*)(&gdt_ptr[0]);
	t_32 *p_gdt_base  = (t_32*)(&gdt_ptr[2]);

	*p_gdt_limit = GDT_SIZE * sizeof( DESCRIPTOR ) - 1;
	*p_gdt_base  = (t_32)&gdt;

	/* idt_ptr[6] has 6 bytes: 0~15:Limit  16~47:Base, as parameter of sidt and lidt */
	t_16 *p_idt_limit = (t_16*)(&idt_ptr[0]);
	t_32 *p_idt_base  = (t_32*)(&idt_ptr[2]);

	*p_idt_limit = IDT_SIZE * sizeof( GATE ) - 1;
	*p_idt_base  = (t_32)&idt;

	init_prot();

	disp_str("-----\"cstart\" finished-----\n");
}
