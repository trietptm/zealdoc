#include "kernel.h"

PUBLIC void init_8259A()
{
	/* ^^^80x86 mode standard init_8259A procedure^^^ */

	out_byte(INT_M_CTL,	0x11);		/* Master 8259 - 0x20, ICW1. */
	out_byte(INT_S_CTL,	0x11);		/* Slave  8259 - 0xA0, ICW1. */

	/* master vector at 0x20 */
	out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	/* Master 8259, ICW2. */
	/* slave vector at 0x28 */
	out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	/* Slave  8259, ICW2. */

	/* 0x4 means master's IRQ2(00000100) */
	out_byte(INT_M_CTLMASK,	0x4);		/* Master 8259, ICW3. */
	out_byte(INT_S_CTLMASK,	0x2);		/* Slave  8259, ICW3. */

	/* 0x1 means Full Nested Mode */
	out_byte(INT_M_CTLMASK,	0x1);		/* Master 8259, ICW4. */
	out_byte(INT_S_CTLMASK,	0x1);		/* Slave  8259, ICW4. */

	/* ===80x86 mode standard init_8259A procedure=== */

	/* mask interrupt */
	out_byte(INT_M_CTLMASK,	0xFD);		/* Master 8259, OCW1. */
	out_byte(INT_S_CTLMASK,	0xFF);		/* Slave  8259, OCW1. */
}

PUBLIC void spurious_irq(int irq)
{
	disp_str("spurious_irq: ");
	disp_int(irq);
	disp_str("\n");
}

PUBLIC void test_str(void)
{
	disp_str("TEST_STR: haha");
	disp_str("\n");
}
