#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>

#include "gis.h"
#include "graph.h"
#include "driverlib.h"
#include "driver.h"
#include "pad.h"

const struct driver *driver;

int NCOLORS;

int screen_left;
int screen_right;
int screen_bottom;
int screen_top;

int cur_x;
int cur_y;

double text_size_x;
double text_size_y;
double text_rotation;

static jmp_buf save;

static RETSIGTYPE handle_sigpipe(int sig)
{
	longjmp(save, 1);
}

static RETSIGTYPE handle_sigterm(int sig)
{
	COM_Graph_close();
}

int LIB_main(const struct driver *drv, int argc, char **argv)
{
	const char *p;
	char *me;
	char *connpath;
	int _wfd;
	int _rfd;
	int eof;

	char c;
	pid_t pid;
	int foreground;
#ifdef USE_G_SOCKS
	int listenfd;
#endif
	struct sigaction sigact;

	driver = drv;

	/* The calling syntax is as follows:
	   monitor_name [-] "input_fifo output_fifo"

	   The "-", if present, causes the monitor to run in foreground.
	   Otherwise, once it has been determined that the monitor is not
	   already running, we will fork and the parent will exit, so that
	   the monitor is left running in background.  For more information,
	   see the comments in Dstart_mon.c.
	*/

	if (argc != 4)
	{
		fprintf(stderr,"Usage:  %s <name> [-] \"input_fifo output_fifo\"\n", argv[0]);
		return 1;
	}

	/* whoami */
	me = argv[1];

	foreground = (argv[2][0] == '-');

#ifdef USE_G_SOCKS
	connpath = G_sock_get_fname(me);
	if (connpath == NULL)
		G_fatal_error("In %s: Couldn't get socket path.\n", __FILE__);
#else
	connpath = argv[3];
#endif

	/* Syntax is all right.  Now we must check and see whether or not someone */
	/* (possibly another invocation of ourself) is using our socket/FIFOs.    */

	if (check_connection(me, connpath) != 0)
	{
		/* Our socket/FIFOs are in use (or other error) */
		G_fatal_error("Unable to start monitor <%s>", me);
	}

	/* initialize graphics */

	p = getenv ("GRASS_WIDTH");
	screen_left = 0;
	screen_right = (p && atoi(p)) ? atoi(p) : DEF_WIDTH;

	p = getenv ("GRASS_HEIGHT");
	screen_top = 0;
	screen_bottom = (p && atoi(p)) ? atoi(p) : DEF_HEIGHT;

	if (COM_Graph_set(argc, argv) < 0)
		exit(1);

	/* Initialize color stuff */
	COM_Color_table_fixed();

	/* We are free to run now.  No one is using our socket/FIFOs.             */
	/* If we are to run in background, we will have foreground == 0 from      */
	/* the syntax check above. */

#ifdef SIGPIPE
	sigact.sa_handler = handle_sigpipe;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGPIPE, &sigact, NULL);
#endif
	sigact.sa_handler = handle_sigterm;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGTERM, &sigact, NULL);

#ifdef USE_G_SOCKS
	listenfd = prepare_connection_sock(me, connpath);
#endif

	/* initialize the pads */
	create_pad("");    /* scratch pad */

	fprintf(stderr,"Graphics driver [%s] started\n", me);

	if (!foreground)
	{
		pid = fork();
		if (pid)
		{
			if (pid > 0)        /* parent exits */
			{
				exit(0);
			}
			else    /* weren't able to fork */
			{
				fprintf(stderr,"Error - Could not fork to start [%s]\n",me);
				exit(1);
			}
		}
		else
		{
			/* change process groups to be shielded from keyboard signals */
			/* note: use BSD form of call, which will also work for ATT   */
#ifdef SETPGRP_VOID
			setpgrp();
#else
			setpgrp(0, getpid());
#endif
		}
	}               /* monitor runs */

	while (1)   /* re-open upon EOF */
	{
		for (;;)
		{
#ifdef USE_G_SOCKS
			if (get_connection_sock(listenfd, &_rfd, &_wfd, COM_Work_stream()) >= 0)
				break;
#else
			if (get_connection_fifo(connpath, &_rfd, &_wfd, COM_Has_work()) >= 0)
				break;
#endif

			COM_Do_work(0);
		}

		command_init(_rfd, _wfd);

		COM_Client_Open();

		eof = 0;

		while (eof <= 0)
		{
			COM_Do_work(1);

			if (setjmp(save))
			{
				fprintf(stderr, "Monitor <%s>: Caught SIGPIPE\n", me);
				break;
			}

			if (get_command(&c) != 0)
				break;

			if (process_command(c))
			{
				fprintf(stderr, "Monitor <%s>: Premature EOF\n", me);
				break;
			}
		}

		/* read encountered EOF. close socket/FIFOs now */
		close(_wfd);
		close(_rfd);
		_rfd = _wfd = -1;  /* Set to invalid file descriptor number */

		COM_Client_Close();
	}

	return 0;
}

