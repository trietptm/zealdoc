#ifndef __GLOBAL_INCLUDE_H__
#define __GLOBAL_INCLUDE_H__

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		disp_pos;
EXTERN	t_8		gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	t_8		idt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	GATE		idt[IDT_SIZE];

/* kliba.asm */
PUBLIC t_8	in_byte(t_port port);
PUBLIC void	out_byte(t_port port, t_8 value);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void	init_prot();

#endif /* __GLOBAL_INCLUDE_H__ */
