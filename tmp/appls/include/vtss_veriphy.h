#ifndef __VTSS_VERIPHY_INCLUDE__
#define __VTSS_VERIPHY_INCLUDE__

#define ABS(x) (((x) < 0) ? -(x) : (x))

#define MAX_ABS_COEFF_ANOM_INVALID_NOISE  70
#define MAX_ABS_COEFF_LEN_INVALID_NOISE  180

#define DISCARD 0x80
#define FFEinit4_7anomSearch   0
#define FFEinit4_7lengthSearch 1

#define VTSS_VERIPHY_STATE_DONEBIT 0x80
#define VTSS_VERIPHY_STATE_IDLE 0x00

typedef enum {
	VERIPHY_STATE_IDLE              = VTSS_VERIPHY_STATE_IDLE,
	VERIPHY_STATE_INIT_0            = VTSS_VERIPHY_STATE_IDLE + 0x01,
	VERIPHY_STATE_INIT_1            = VTSS_VERIPHY_STATE_IDLE + 0x02,
	VERIPHY_STATE_INIT_LINKDOWN     = VTSS_VERIPHY_STATE_IDLE + 0x03,
	VERIPHY_STATE_INIT_ANOMSEARCH_0 = VTSS_VERIPHY_STATE_IDLE + 0x04,
	VERIPHY_STATE_INIT_ANOMSEARCH_1 = VTSS_VERIPHY_STATE_IDLE + 0x05,
	VERIPHY_STATE_ANOMSEARCH_0      = VTSS_VERIPHY_STATE_IDLE + 0x06,
	VERIPHY_STATE_ANOMSEARCH_1      = VTSS_VERIPHY_STATE_IDLE + 0x07,
	VERIPHY_STATE_ANOMSEARCH_2      = VTSS_VERIPHY_STATE_IDLE + 0x08,
	VERIPHY_STATE_ANOMSEARCH_3      = VTSS_VERIPHY_STATE_IDLE + 0x09,
	VERIPHY_STATE_INIT_CABLELEN     = VTSS_VERIPHY_STATE_IDLE + 0x0a,
	VERIPHY_STATE_GETCABLELEN_0     = VTSS_VERIPHY_STATE_IDLE + 0x0b,
	VERIPHY_STATE_GETCABLELEN_1     = VTSS_VERIPHY_STATE_IDLE + 0x0c,
	VERIPHY_STATE_PAIRSWAP          = VTSS_VERIPHY_STATE_IDLE + 0x0d,
	VERIPHY_STATE_ABORT             = (VTSS_VERIPHY_STATE_DONEBIT | 0),
	VERIPHY_STATE_FINISH            = (VTSS_VERIPHY_STATE_DONEBIT | 0x0e)
} vtss_veriphy_task_state_t;

/* VeriPHY status */
typedef enum {
	VTSS_VERIPHY_STATUS_OK      = 0,  /* Correctly terminated pair */
	VTSS_VERIPHY_STATUS_OPEN    = 1,  /* Open pair */
	VTSS_VERIPHY_STATUS_SHORT   = 2,  /* Short pair */
	VTSS_VERIPHY_STATUS_ABNORM  = 4,  /* Abnormal termination */
	VTSS_VERIPHY_STATUS_SHORT_A = 8,  /* Cross-pair short to pair A */
	VTSS_VERIPHY_STATUS_SHORT_B = 9,  /* Cross-pair short to pair B */
	VTSS_VERIPHY_STATUS_SHORT_C = 10, /* Cross-pair short to pair C */
	VTSS_VERIPHY_STATUS_SHORT_D = 11, /* Cross-pair short to pair D */
	VTSS_VERIPHY_STATUS_COUPL_A = 12, /* Abnormal cross-pair coupling, pair A */
	VTSS_VERIPHY_STATUS_COUPL_B = 13, /* Abnormal cross-pair coupling, pair B */
	VTSS_VERIPHY_STATUS_COUPL_C = 14, /* Abnormal cross-pair coupling, pair C */
	VTSS_VERIPHY_STATUS_COUPL_D = 15  /* Abnormal cross-pair coupling, pair D */
} vtss_phy_veriphy_status_t;

typedef struct _vtss_veriphy_task_t {
	/* Variables common to all tasks */
	vtss_mtimer_t timeout;	/* Absolute timeout */
	unsigned char task_state;/* 0x00 ==> idle (but still serviced) */
	/* 0x01 - 0x7f ==> task-defined, */
	/* bit [7]: 1 ==> abort requested */
	
	/* VeriPHY public variables */
	unsigned char port;	/* PHY on which VeriPHY is running/last run on */
	unsigned char flags;	/* bit [7:6] = VeriPHY operating mode */
				/*        00 ==> full VeriPHY algorithm */
				/*        01 ==> anomaly-search only */
				/*        10 ==> anomaly-search w/o x-pair only */
				/*        11 ==> reserved */
				/* bit [5:4] = unreliablePtr (if bit [3] == 1) */
				/* bit [3]   = unreliablePtr presence flag */
				/* bit [2]   = getCableLength done flag */
				/* bit [1]   = valid */
				/* bit [0]   = linkup-mode */
	unsigned char flags2;	/* bits [7:2] - reserved */
	/* bits [1:0] - ams_force_cu, ams_force_fi on entry (Spyder) */
	/* bits [1:0] - reserved, ActiPHY-enabled on entry (Luton) */
	unsigned long saveReg;	/*- TP[5.4:3], MII [28.2], MII[9.12:11], MII[0] - 21 bits */
	unsigned long tokenReg;	/*-  Token Ring Registers 29 bits */
	unsigned char stat[4];	/* status for pairs A-D (0-3), 4-bit unsigned number */
	/*        most signiciant 4-bits represents prev. status */
	unsigned char loc[4];	/* length/fault location for pairs A-D (0-3), 8-bit unsgn */
	
	/* VeriPHY private variables */
	signed char subchan;
	signed char nc;
	unsigned char numCoeffs;
	unsigned char firstCoeff;
	short strength[4];		/* fault strength for pairs A-D (0-3), 14-bit signed int. */
	short thresh[4];	/* threshold (used in different places), 13-bit unsgn */
	short log2VGAx256;	/* log2(VGA gain scalefactor)*256 (0 for link-down case) */
	signed char  signFlip;	/* count used to improve location accuracy */
	long tr_raw0188;	/* wastes one byte */
} vtss_veriphy_task_t;

/* VeriPHY result */
typedef struct {
	BOOL                      link;      /* Link status */
	vtss_phy_veriphy_status_t status[4]; /* Status, pair A-D (0-3) */
	uchar                     length[4]; /* Length (meters), pair A-D (0-3) */
} vtss_phy_veriphy_result_t;

/******************************************************************************
 * Description: Start VeriPHY process on port.
 *
 * \param port_no (input): Port number of PHY.
 * \param mode (input)   : VeriPHY operating mode
 *                0 ==> full VeriPHY algorithm
 *                1 ==> anomaly-search and x-pair (no cable length search)
 *                2 ==> anomaly-search  (no cable length search and no x-pair search)
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_veriphy_start(const vtss_port_no_t port_no, uchar mode);

/******************************************************************************
 * Description: Get VeriPHY process result on port.
 *
 * \param port_no (input): Port number of PHY.
 * \param result (output): VeriPHY result, valid if VTSS_OK is returned.
 *
 * \return : Return code, VTSS_INCOMPLETE until result is valid.
 ******************************************************************************/
vtss_rc vtss_phy_veriphy_get(const vtss_port_no_t port_no, 
                             vtss_phy_veriphy_result_t * const result);

/******************************************************************************
 * Purpose     : Prepare for VeriPHY
 * Remarks     : mode:
 *                0 ==> full VeriPHY algorithm
 *                1 ==> anomaly-search and x-pair (no cable length search)
 *                2 ==> anomaly-search  (no cable length search and no x-pair search)
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
vtss_rc vtss_phy_veriphy_task_start(vtss_port_no_t port_no, uchar mode);

/******************************************************************************
 * Purpose     : Do VeriPHY operation on specified ports.
 * Remarks     : 
 *               Results of operation is returned in structure pointed to
 *               by *result.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
vtss_rc vtss_phy_veriphy(vtss_veriphy_task_t *tsk);

#endif /* __VTSS_VERIPHY_INCLUDE__ */
