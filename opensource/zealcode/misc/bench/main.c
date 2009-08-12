#include "main.h"

#define DEF_DURATION		3	/* secs */
#define DEF_TIMES		5
#define DEF_SIZE		1024	/* kb */
#define DEF_STRIDE		1

#define CACHE_LINE_SIZE		64	/* bytes */

#define LOOP_TYPE_DURATION	0x1
#define LOOP_TYPE_TIMES		0x2

#define BENCH_ALIGN_SHIFT	5
#define BENCH_ALIGN_MASK	(1 << 5)
#define BEACH_ALIGN(ptr)	(((unsigned long)(ptr) + BENCH_ALIGN_MASK - 1)&~(BENCH_ALIGN_MASK - 1))

clock_t g_cost = 0;
int g_duration = 0;
long long g_times = 0;		/* store `iter' when op_type is DURATION */
int g_buffer_size = 0;
int g_do_loop = 0;
int g_op_type = 0;
int g_stride = 0;

unsigned char *work_set_ptr = NULL;	/* aligned ptr */
unsigned char *work_set = NULL;

struct option LongOptions[] = {
    {"duration", 1, NULL, 'd'},
    {"class",    1, NULL, 'c'},
    {"times",    1, NULL, 't'},
    {"stride",   1, NULL, 's'},
    {"buffer",   1, NULL, 'b'},
    {"help",     0, NULL, 'h'},
    {0,          0, NULL, 0}
};

int usage () {
	fprintf (stdout, "    usage\n");
	fprintf (stdout, "bench[OPTION]\n");
	fprintf (stdout, "    options\n");
	fprintf (stdout, "    -c, --class=CLASS(d/t)    duration or times\n");
	fprintf (stdout, "    -d, --duration=DURATION   record duration\n");
	fprintf (stdout, "    -t, --times=TIMES         loop times\n");
	fprintf (stdout, "    -b, --bucket=BUCKET       bucket size\n");
	fprintf (stdout, "    -s, --stride              for loop\n");
	fprintf (stdout, "    -h, --help                usage\n");
	exit (ERR_SUC);
}


/********************************************************
 * Timing routine
 *******************************************************/
int wake_me(int seconds, void (*func)(int signum))
{
	/* record the start time */
	/* set up the signal */
	signal (SIGALRM, func);
	/* set up the timeout clock */
	alarm (seconds);
}

/* test type */
typedef int T_type;

void report(int signum)
{
	struct tms tms_clock;

	g_do_loop = 0;

#if 0
	/* XXX: times() is get process time param */
	/* calculate the cost time */
	if (times (&tms_clock) == -1) {
		exit (ERR_CLK);
	}

	g_cost = tms_clock.tms_utime + tms_clock.tms_stime 
		+ tms_clock.tms_cutime + tms_clock.tms_cstime;
#endif
}

#define BENCH_KB2BYTE(kb)		(kb * 1024)
#define BENCH_GET_BYTE(kb)		(BENCH_KB2BYTE(kb) + BENCH_ALIGN_MASK)
static void warm_up(unsigned char *ptr, int size_kb)
{
	int result = 0;
	volatile int sink;
	long i, size = BENCH_KB2BYTE(size_kb);

	for (i = 0; i < size; i++)
		result += ptr[i];
	sink = result;
}

static void do_summ(void)
{
	printf("SUMM: duration=%d, bucket=%d(kb)\n", g_duration, g_buffer_size);
	if (g_op_type == LOOP_TYPE_DURATION)
		printf("SUMM: loop_times=%lld\n", g_times);
}

static int do_cache_read(void)
{
	volatile int sink;
	long i, size = BENCH_KB2BYTE(g_buffer_size);
	unsigned char *ptr = work_set_ptr;
	int type = g_op_type;
	register long result;
	long iter = 0;


	i = 0;

	if (type == LOOP_TYPE_DURATION) {
		g_do_loop = 1;
		/* loop forever */
		while (g_do_loop) {
			if (i >= size) {
				i = 0;
				result = 0;
				iter++;
			}
			result = ptr[i];
			i += g_stride;
		}
		g_times = iter;
	} else {
		do {
			for (i = 0; i < size; i += g_stride)
				result += ptr[i];
		} while (g_times--);
	}
}

/* work_set_size - kb */
static int do_cache_bench(void)
{
	if (!work_set) {
		return -1;
	}

	warm_up(work_set_ptr, g_buffer_size);

	if (g_op_type == LOOP_TYPE_DURATION) {
		wake_me(g_duration, report);
		do_cache_read();
		do_summ();
	} else {
		time_main(do_cache_read);	
	}
	return 0;
}

int do_init(void)
{
	if (!vtss_vit_init())
		return 0;
	return -1;
}

void do_exit(void)
{
	vtss_vit_exit();
}

int beach_alloc(void)
{
	work_set = malloc(sizeof (unsigned char) * BENCH_GET_BYTE(g_buffer_size));
	if (!work_set) {
		printf("out of memory.\n");
		return 1;
	}
	work_set_ptr = (unsigned char *)BEACH_ALIGN(work_set);
	return 0;
}

void beach_free(void)
{
	if (work_set)
		free(work_set);
	work_set_ptr = NULL;
}
void do_first_summ(void)
{
	printf("BENCH: Alloc bucket %d (kb)\n", g_buffer_size);
	printf("BENCH: %s benchmark test", 
			g_op_type == LOOP_TYPE_DURATION ? "DURATION" : "TIMES");
	if (g_op_type == LOOP_TYPE_DURATION)
		printf(", duration=%d(secs)", g_duration);
	else
		printf(", times=%d", g_times);
	printf(", stride=%d\n", g_stride);
}

int main(int argc, char *argv[])
{
	int i = 0;

	if (argc == 1)
		usage();

	while (1) {
		int option_index = 0;
		int c;

		c = getopt_long(argc, argv, "d:s:b:c:t:h",
				LongOptions, &option_index);

		if (c == EOF)
			break;

		switch(c) {
			case 'd':
				g_duration = atoi (optarg);
				break;
			case 's':
				g_stride = atoi(optarg);
				break;
			case 'b':
				g_buffer_size = atoi(optarg);
				break;
			case 't':
				g_times = atoll(optarg);
				break;
			case 'c':
				if (optarg[0] == 'd')
					g_op_type = LOOP_TYPE_DURATION;
				else
					g_op_type = LOOP_TYPE_TIMES;
				break;
			case '?':
				fprintf (stdout, "invalid usgae\n");
			case 'h':
				usage ();
				break;
			default:
				fprintf (stdout, "Unknown usage %c.\n", c);
				exit (ERR_CMD);
		}
	}

	if (do_init()) {
		fprintf(stderr, "BENCH: do_init failure\n");
		return -1;
	}

	if (g_duration == 0) g_duration = DEF_DURATION;
	if (g_buffer_size == 0) g_buffer_size = DEF_SIZE;
	if (g_times == 0) g_times= DEF_TIMES;
	if (g_stride == 0) g_stride = DEF_STRIDE;

	if (beach_alloc())
		goto err;

	do_first_summ();

	do_cache_bench();

	printf("BENCH: test finish and exit\n");

	beach_free();
err:

	do_exit();
	return 0;
}
