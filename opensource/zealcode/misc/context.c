/*******************************************************************************
 *  The BYTE UNIX Benchmarks (NEC Version) - Release 1
 *          Module: context3.c   SID: 1.0 4/14/05 10:30:09
 *          
 *******************************************************************************
 * Author:
 *      Zhengyh "Spiegel",     zhengyh@sh.necas.nec.com.cn
 * Bug reports, patches, comments, suggestions are welcome:
 *	NEC-CAS ELF,           amr@sh.necas.nec.com.cn
 *
 *******************************************************************************
 *  Modification Log:
 *  $Header: /home/cvsroot/osi-unixbench/src/context.c,v 1.6 2005/09/16 12:57:41 zhengl Exp $
 *
 *
 ******************************************************************************/
/*
 *  Context switching via synchronized unbuffered pipe i/o
 *
 *  NOTE: This file is token from Benchmarks's context1.c and make the following
 *        changes:
 *        1. Context switching between huge of created processes, no longer
 *           limited to only two processes.
 *        2. Ensure the number of the "Active" processeses remains unchanged
 *           and also configurable via command lines.
 *
 ******************************************************************************/
/*
 *  We use the following conversions to identify errors:
 *     no error            0
 *     command line error  1
 *     pipe  error         2
 *     fork  error         3
 *     write error         4
 *     read  error         5
 *     fopen error         6
 *     dir   error         7
 *     timeout error       8
 *     sync error          9
 *     signal error        10
 *     clock error         11
 *
 ******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/*
 * Error code definitions
 */
#define ERR_SUC  0 /* no error */
#define ERR_CMD  1 /* command lines error */
#define ERR_PIP  2 /* pipe created error */
#define ERR_FRK  3 /* fork error */
#define ERR_WRI  4 /* write error */
#define ERR_RED  5 /* read error */
#define ERR_FOP  6 /* fopen error */
#define ERR_DIR  7 /* dir error */
#define ERR_TIO  8 /* timeout error */
#define ERR_SYN  9 /* sync error */
#define ERR_SIG  10 /* kill error */
#define ERR_CLK  11 /* clock error */

unsigned long iter = 0;
clock_t cost = 0;
int p1[2], p2[2];

int duration = 0;
int nproc = 0;

struct option LongOptions[] = {
    {"duration", 1, NULL, 'd'},
    {"process",  1, NULL, 'p'},
    {"help",     0, NULL, 'h'},
    {"version",  0, NULL, 'v'},
    {0,          0, NULL, 0}
};

int usage () {
    fprintf (stdout, "name\n");
    fprintf (stdout, "context2 -- multi processes switching based on pipe\n");
    fprintf (stdout, "    usage\n");
    fprintf (stdout, "context2 [OPTION]\n");
    fprintf (stdout, "    options\n");
    fprintf (stdout, "    -d, --duration=DURATION   record duration\n");
    fprintf (stdout, "    -p, --process=PROCESS     number of processes to be created\n");
    fprintf (stdout, "    -h, --help                usage\n");
    fprintf (stdout, "    -v, --version             print version information\n");
    exit (ERR_SUC);
}

int version () {
    fprintf (stdout, "context2 version 1.0\n\n");
    fprintf (stdout, "Copyright (C) 2005 Free Software Foundation, Inc.\n");
    fprintf (stdout, "This is free software; see the source for copying conditions.  There is NO\n");
    fprintf (stdout, "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
    exit (ERR_SUC);
}

#if 0
void report () {
    fprintf (stderr, "%lu loops\n", iter_local);
    exit (0);
}
#endif

/********************************************************
 * Print the test results to the log files
 * in this format: [number of switching] [earse time]
 *******************************************************/
void report (int signum) {
#ifndef MAXPATH
#define MAXPATH 256
#endif
    char filename[MAXPATH];
    FILE *f;
    struct tms tms_clock;
    pid_t pid;
    int status;

    /* close the pipe to inform the child process to exit */
    close (p1[1]); close (p2[0]);
    while (1) {
        pid = wait (&status);
        /* close the pipe will casue the child process to exit
         * abnormally, the exit code will be ingored in this case */
        if (pid <= 0)
           break;
    }

    /* calculate the cost time */
    // cost = time(NULL) - cost;
    if (times (&tms_clock) == -1) {
#ifdef DEBUG
        perror ("times error");
#endif
        exit (ERR_CLK);
    }

    cost = tms_clock.tms_utime + tms_clock.tms_stime
            + tms_clock.tms_cutime + tms_clock.tms_cstime;

    sprintf (filename, "context3%d.pidlog", getpid());
    if ((f = fopen (filename, "w")) == NULL) {
        perror ("fopen failed\n");
        exit (ERR_FOP);
    }
    fprintf (f, "%u %u\n", iter, cost);
    fclose (f);
    exit (ERR_SUC);
}

/********************************************************
 * Timing routine
 *******************************************************/
int wake_me (int seconds, void (*func)(int signum)) {
    /* record the start time */
    //cost = time(NULL);
     /* set up the signal */
    signal (SIGALRM, func);
    /* set up the timeout clock */
    alarm (seconds);
}

#if 0
/********************************************************
 * resume from block status
 *******************************************************/
void resume (int signum) {
    /* shut up gcc */
    (void)signum;

    /* shutdown the timer, if any */
    alarm (0);

    /* Return the signum to default handler */
    signal (signum, SIG_DFL);

}


/*******************************************************
 * Timeout resume from block status
 * to prevent some kind of unexpected errors
 ******************************************************/
void resume_timeout(int signum) {
#ifdef NDEBUG
    perror ("Timeout when waiting for SIGUSR1");
#endif
    exit (ERR_TIO);
}


/*******************************************************
 * Signal routing
 ******************************************************/
int wait_sig (int signum, void (*func)(int signum)) {
   /* shutup gcc */
    (void)signum;
    /* set up the signal */
    signal (signum, func);
    /* set up the time out handler, 
       in case of some kind of unexpected errors */
    wake_me (10, resume_timeout);
}
#endif

/*******************************************************
 * Calculate routine, only used by the top most process
 ******************************************************/
void calculate () {
    DIR *dir = opendir (".");
    struct dirent *dent= NULL;
    FILE *f;
    unsigned long iter_local;
    clock_t cost_local;

    if (dir == NULL) {
        perror ("opendir failed");
        exit (ERR_DIR);
    }

    while (dent = readdir(dir)) {
        if ((strcmp (dent->d_name, ".") == 0) ||
            (strcmp (dent->d_name, "..") == 0))
            continue;

        if (strstr (dent->d_name, ".pidlog")) {
            if ((f = fopen (dent->d_name, "r")) == NULL) {
#ifdef DEBUG
                perror ("fopen failed");
                fprintf (stderr, "%s can't not be opened\n", dent->d_name);
#endif
                continue;
            }

            if (fscanf (f, "%u %u", &iter_local, &cost_local) < 2)
                continue;

            iter += iter_local;
            cost += cost_local;
            fclose (f);
            /* delete the temp file created by spawning process */
#ifndef DEBUG
            /* In debug mode files will be remain untouched*/
            unlink (dent->d_name);
#endif
        }
    }

    closedir(dir);

    /* Print the result to the stdout & exit with 0 */
    fprintf (stdout, "iteration %u time %u\n", iter, cost);
    exit (ERR_SUC);
}

int pipe_loop() {
    unsigned long check;
    pid_t pid;

    /* Hope the sleep will help the top process
     * finish the forkes, at least most of them*/
    sleep (1);

    if (pipe (p1) || pipe (p2)) {
        perror ("pipe created failed");
        exit (ERR_PIP);
    }

    if ((pid = fork()) < 0) {
#ifdef DEBUG
        perror ("fork failed\n");
#endif
        exit (ERR_FRK);
    }

    if (pid > 0) {
        /* parent process */

#if 0
        /***************************************************
        * Set up the signal
        * only after receiving the SIGUSR1 signal
        * transfer is started
        **************************************************/
        wait_sig (SIGUSR1, resume);
        pause ();
#endif

        /* set up the timer */
        wake_me (duration, report);

        /* master, write p1 & read p2 */
        close(p1[0]); close(p2[1]);
        while (1) {
            if (write(p1[1], (char *)&iter, sizeof(iter)) != sizeof(iter)) {
#ifdef NDEBUG
                if ((errno != 0) && (errno != EINTR))
                    perror("master write failed");
#endif
                exit(ERR_WRI);
            }

            if (read(p2[0], (char *)&check, sizeof(check)) != sizeof(check)) {
#ifdef NDDEBUG
                if ((errno != 0) && (errno != EINTR))
                    perror("master read failed");
#endif
                exit(ERR_RED);
            }

            if (check != iter) {
#ifdef DEBUG
                printf("Master sync error: expect %lu, got %lu\n",
                       iter, check);
#endif
                exit(ERR_SYN);
            }
            iter++;
        }
    }
    /* child process */

#if 0
    /**************************************************
     * Child processes must ignore the signal SIGUSR1
     * otherwise will be corrupted. Also this would
     * prevent overhead casued by signal handling
     **************************************************/
    signal (SIGUSR1, SIG_IGN);
#endif


    unsigned long iter1;
    iter1 = 0;
    /* slave, read p1 & write p2 */
    close(p1[1]); close(p2[0]);
    while (1) {
        if (read(p1[0], (char *)&check, sizeof(check)) != sizeof(check)) {
#ifdef NDEBUG
            if ((errno != 0) && (errno != EINTR))
                perror("slave read failed");
#endif
            exit(ERR_RED);
        }

        if (check != iter1) {
#ifdef DEBUG
            printf("Slave sync error: expect %lu, got %lu\n",
                   iter1, check);
#endif
            exit(ERR_SYN);
        }
        if (write(p2[1], (char *)&iter1, sizeof(iter1)) != sizeof(iter1)) {
#ifdef NDEBUG
            if ((errno != 0) && (errno != EINTR))
                perror("slave write failed");
#endif
            exit(ERR_WRI);
        }
        iter1++;
    }
    return 0;
}

void wait_child() {
    pid_t pid;
    int status;
#ifdef DEBUG
    int err_proc_num = 0;
    int err_code = 0;
#endif

    while (1) {
        pid = wait (&status);
        if (pid <= 0)
            break;
#ifdef DEBUG
        err_code = WEXITSTATUS (status);
        if (err_code != 0) {
            fprintf (stdout, "PID %d process encounted errors, error code %d", pid, err_code);
            err_proc_num++;
        }
#endif
    }
#ifdef DEBUG
    fprintf (stdout, "Total created processes    : %d\n", nproc);
    fprintf (stdout, "Execution success processes: %d\n", nproc - err_proc_num);
    fprintf (stdout, "Execution failure processes: %d\n", err_proc_num);
#endif

}

int main(int argc, char *argv[]) {
    int procnum = 0;
    int i = 0;
    pid_t pid;

    while (1) {
        int option_index = 0;
        int c;

        c = getopt_long(argc, argv, "d:p:hv",
                        LongOptions, &option_index);

        if (c == EOF)
            break;

        switch(c) {
        case 'v':
            version ();
            break;
        case 'd':
            duration = atoi (optarg);
            break;
        case 'p':
            nproc = atoi (optarg);
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

    procnum = nproc / 2;
    while (i < procnum) {
        if ((pid = fork ()) < 0) {
#ifdef DEBUG
            perror ("fork error");
            fprintf (stderr, "%d processes has been created\n");
#endif
            break;
        } 

        if (pid == 0) {
            /* Child process will never return from pipe_loop() */
            return pipe_loop ();
        }

        i++;
    }
#if 1
    /* Should we wait a little more until all the child processes 
     * have been initialized? FIXME*/
    sleep (3);
#endif

#if 0
    /* This would be buggy, especially when huge of child processes
     * are created. It's better to disable this feature
     */

    /* Ok, until now, we can send SIGUSR1 to all the child processes
     * to inform them to begin executed.
     */
    /* Ignore the SIGUSR1 signal */
    signal (SIGUSR1, SIG_IGN);

    if (kill (0, SIGUSR1) == -1) {
#ifdef DEBUG
        perror ("kill error");
#endif
        exit (ERR_SIG);
    }
#endif
    /* wait for termination of all the child process */
    wait_child ();

    /* Calculate the final results and print to the stdout */
    calculate();
    return 0;
} // MAIN
