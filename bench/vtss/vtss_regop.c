#ifdef ARCH_ARM
#define VTSS_TRACE_LAYER 2
#define VTSS_TRACE_FILE "ll"
#include "vtss.h"

int vitfd = -1;
/*
 * l-cache 32kb
 * d-cache 32kb
 * */

static int vtss_io_pi_rd_fast(uint block, uint subblock, const uint reg, ulong *value)
{
	vtss_rc rc = VTSS_OK;
	struct vitgenio_32bit_readwrite pibuf;

	pibuf.offset=(block<<12)|(subblock<<8)|(reg<<0);
	if (ioctl(vitfd, VITGENIO_32BIT_READ, &pibuf) < 0) {
		printf("ioctl(fd,VITGENIO_32BIT_READ) failed");
		*value = 0;
		rc = -1;
	} else {
		*value=pibuf.value;
	}
	return rc;
}

static int vtss_io_pi_rd(uint block, uint subblock, const uint reg, ulong * const value)
{
    int rc;

    if (block == B_CAPTURE || block == B_SYSTEM) {
        rc = vtss_io_pi_rd_fast(block,subblock,reg,value);
    }
#if 0
    else {
        int   i;
        ulong val;

        if ((rc = vtss_io_pi_rd_fast(block, subblock, reg, &val)) < 0)
            return rc;
        for (i = 0; ; i++) {
            if ((rc = vtss_io_pi_rd_fast(B_SYSTEM, 0, R_SYSTEM_CPUCTRL, &val)) < 0)
                break;
            if (val & (1 << 4)) {
                rc = vtss_io_pi_rd_fast(B_SYSTEM, 0, R_SYSTEM_SLOWDATA, value);
                break;
            }
            if (i == 25) {
                printf("block: 0x%02x, subblock: 0x%02x, reg: 0x%02x, DONE failed after %d retries",
                         block,subblock,reg,i);
                rc = -1;
                break;
            }
        }
    }

    printf("block: 0x%02x, subblock: 0x%02x, reg: 0x%02x, value: 0x%08lx",
            block,subblock,reg,*value);
#endif
    return rc;
}

int vtss_vit_init(void)
{
	vitfd = open("/dev/vitgenio", 0);
	if (vitfd < 0) {
		printf("cannot open /dev/vitgenio\n");
		return -1;
	}
	return 0;
}

void vtss_vit_exit(void)
{
	if (vitfd > 0)
		close(vitfd);
}
/* Read target register using current CPU interface */
int ht_rd_wr(uint blk, uint sub, uint reg, ulong *value, int write)
{
	int rc;
	int error;

	switch (blk) {
		case B_PORT:
			/* By default, it is an error if the sub block is not included in chip port mask */
			error = ((sub > VTSS_CHIP_PORTS) ||
					(VTSS_CHIP_PORTMASK & (1<<sub)) == 0);
			if (sub == VTSS_CHIP_PORT_CPU)
				error = 0;
			if (error) {
				break;
			}
			if (sub >= 16) {
				blk = B_PORT_HI;
				sub -= 16;
			}
			break;
		case B_MIIM: /* B_MEMINIT/B_ACL/B_VAUI */
			error = (sub > S_MEMINIT);
			break;
		case B_CAPTURE:
			error = 0;
			break;
		case B_ANALYZER:
		case B_ARBITER:
		case B_SYSTEM:
			error = (sub != 0);
			break;
		case B_PORT_HI: /* vtss_ll_register_read/write may access this directly */
			error = 0;
			break;
		default:
			error = 1;
			break;
	}

	if (error) {
		printf("illegal access, blk: 0x%02x, sub: 0x%02x, reg: 0x%02x", blk, sub, reg);
		return -1;
	}

	rc = vtss_io_pi_rd(blk, sub, reg, value);

	printf("%s blk: %d, sub: %2d, reg: 0x%02x, value: 0x%08lx", 
			write ? "write" : "read ", blk, sub, reg, *value);

	return rc;
}

/* Read target register using current CPU interface */
int ht_rd(uint blk, uint sub, uint reg, ulong *value)
{
    return ht_rd_wr(blk, sub, reg, value, 0);
}

#define VTSS_CPU_RX_QUEUE_START		1
int ht_cpurx_readbuffer(int queue_no, const uint addr, ulong *value)
{
    uint sub, offset;

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
    sub = (S_CAPTURE_DATA+(addr>>8));
    offset = (addr & 0xFF);
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

    sub = (S_CAPTURE_DATA + (queue_no - VTSS_CPU_RX_QUEUE_START)*2);
    offset = 0; /* SparX-28 uses FIFO mode */
    HT_RD(CAPTURE, sub, FRAME_DATA + offset, value);
    return 0;
}
#endif /* ARCH_ARM */
