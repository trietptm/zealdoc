#ifndef __GLOBAL_INCLUDE_H__
#define __GLOBAL_INCLUDE_H__

extern int		disp_pos;
extern t_8		gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
extern t_8		idt_ptr[6];	/* 0~15:Limit  16~47:Base */
extern DESCRIPTOR	gdt[GDT_SIZE];
extern GATE		idt[IDT_SIZE];

/* kliba.asm */
t_8	in_byte(t_port port);
void	out_byte(t_port port, t_8 value);
void	disp_str(char * info);
void	disp_color_str(char * info, int color);
void	init_prot();

#endif /* __GLOBAL_INCLUDE_H__ */
