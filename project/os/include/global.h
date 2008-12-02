#ifndef __GLOBAL_INCLUDE_H__
#define __GLOBAL_INCLUDE_H__

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		disp_pos;
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	t_8		gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	GATE		idt[IDT_SIZE];
EXTERN	t_8		idt_ptr[6];	/* 0~15:Limit  16~47:Base */

/* kliba.asm */
PUBLIC void	out_byte(t_port port, t_8 value);
PUBLIC t_8	in_byte(t_port port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void	init_prot();

#endif /* __GLOBAL_INCLUDE_H__ */
