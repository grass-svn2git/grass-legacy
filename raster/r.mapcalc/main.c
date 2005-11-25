
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapcalc.h"

/****************************************************************************/

int overflow_occurred;

volatile int floating_point_exception;
volatile int floating_point_exception_occurred;

/****************************************************************************/

static const char help_text[] =
"r.mapcalc - Raster map layer data calculator\n"
"\n"
"usage: r.mapcalc '<map>=<expression>'\n"
"\n"
"r.mapcalc performs arithmetic on raster map layers.\n"
"\n"
"New raster map layers can be created which are arithmetic expressions\n"
"involving existing raster map layers, integer or floating point constants,\n"
"and functions.\n"
"\n"
"For more information use 'g.manual r.mapcalc'\n";

/****************************************************************************/

static expr_list *result;

/****************************************************************************/

static RETSIGTYPE
handle_fpe(int n)
{
	floating_point_exception = 1;
	floating_point_exception_occurred = 1;
}

static void
pre_exec(void)
{
#ifndef __MINGW32__
#ifdef SIGFPE
	struct sigaction act;

	act.sa_handler = &handle_fpe;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	sigaction(SIGFPE, &act, NULL);
#endif
#endif 

	floating_point_exception_occurred = 0;
	overflow_occurred = 0;
}

static void
post_exec(void)
{
#ifndef __MINGW32__
#ifdef SIGFPE
	struct sigaction act;

	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	sigaction(SIGFPE, &act, NULL);
#endif
#endif
}

/****************************************************************************/

static const char *
join(int argc, char **argv)
{
	int size = 0;
	char *buf;
	int i;

	for (i = 0; i < argc; i++)
		size += strlen(argv[i]) + 1;

	buf = G_malloc(size);
	*buf = '\0';
	for (i = 0; i < argc; i++)
	{
		if (i)
			strcat(buf, " ");
		strcat(buf, argv[i]);
	}

	return buf;
}

/****************************************************************************/

int 
main(int argc, char **argv)
{
	int all_ok;

	G_gisinit(argv[0]);

	if (argc > 1 && strcmp(argv[1], "help") == 0)
	{
		fputs(help_text, stderr);
		return 0;
	}

	result = (argc >= 2)
		? parse_string(join(argc - 1, argv + 1))
		: parse_stream(stdin);

	if (!result)
		return 1;

	pre_exec();
	execute(result);
	post_exec();

	all_ok = 1;

	if (floating_point_exception_occurred)
	{
		fprintf(stderr, "NOTE: floating point error(s) occured in the calculation\n");
		all_ok = 0;
	}

	if (overflow_occurred)
	{
		fprintf(stderr, "NOTE: overflow occured in the calculation\n");
		all_ok = 0;
	}

	return all_ok ? 0 : 1;
}

/****************************************************************************/

