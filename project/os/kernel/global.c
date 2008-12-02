
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
						    rubbish global.c
						    rubbish style
						    rubbish macro
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "kernel.h"

int		disp_pos;
t_8		gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
t_8		idt_ptr[6];	/* 0~15:Limit  16~47:Base */
DESCRIPTOR	gdt[GDT_SIZE];
GATE		idt[IDT_SIZE];


