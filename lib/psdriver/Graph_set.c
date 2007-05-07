/*
 * Start up graphics processing.  Anything that needs to be assigned, set up,
 * started-up, or otherwise initialized happens here.  This is called only at
 * the startup of the graphics driver.
 *
 * The external variables define the pixle limits of the graphics surface.  The
 * coordinate system used by the applications programs has the (0,0) origin
 * in the upper left-hand corner.  Hence,
 *    screen_left < screen_right
 *    screen_top  < screen_bottom 
 */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <grass/gis.h>
#include "psdriver.h"

FILE *outfp;
int true_color;
int width, height;

struct paper
{
	const char *name;
	double width, height;
	double left, right, top, bot;
};

static const struct paper papers[] =
{
	/* name		width	height	left	right	top	bottom */
	{ "a4",		8.268,	11.693,	0.5,	0.5,	1.0,	1.0 },
	{ "a3",		11.693,	16.535,	0.5,	0.5,	1.0,	1.0 },
	{ "a2",		16.54,	23.39,	1.0,	1.0,	1.0,	1.0 },
	{ "a1",		23.39,	33.07,	1.0,	1.0,	1.0,	1.0 },
	{ "a0",		33.07,	46.77,	1.0,	1.0,	1.0,	1.0 },
	{ "us-legal",	8.5,	14.0,	1.0,	1.0,	1.0,	1.0 },
	{ "us-letter",	8.5,	11.0,	1.0,	1.0,	1.0,	1.0 },
	{ "us-tabloid",	11.0,	17.0,	1.0,	1.0,	1.0,	1.0 },
	{ NULL,		0,	0,	0,	0,	0,	0 }
};

static void write_prolog(void)
{
	char prolog_file[GPATH_MAX];
	FILE *prolog_fp;

	sprintf(prolog_file, "%s/etc/psdriver.ps", G_gisbase());

	prolog_fp = fopen(prolog_file, "r");
	if (!prolog_fp)
		G_fatal_error("Unable to open prolog file");

	while (!feof(prolog_fp))
	{
		char buf[256];

		if (!fgets(buf, sizeof(buf), prolog_fp))
			break;

		fputs(buf, outfp);
	}

	fclose(prolog_fp);
}

static int in2pt(double x)
{
	return (int) (x * 72);
}

static void set_paper(void)
{
	const char *name = getenv("GRASS_PAPER");
	const struct paper *paper;
	int i;

	if (!name)
		return;

	for (i = 0; ; i++)
	{
		paper = &papers[i];

		if (!paper->name)
			return;

		if (G_strcasecmp(name, paper->name) == 0)
			break;
	}

	width  = in2pt(paper->width ) - in2pt(paper->left) - in2pt(paper->right);
	height = in2pt(paper->height) - in2pt(paper->top ) - in2pt(paper->bot  );

	output("%d %d translate\n", in2pt(paper->left), in2pt(paper->bot));
}

int PS_Graph_set(int argc, char **argv)
{
	const char *file_name;
	int landscape;
	char *p;

	G_gisinit("PS driver") ;

	p = getenv("GRASS_PSFILE");
	if (!p || strlen(p) == 0)
		p = FILE_NAME;

	file_name = p;

	p = getenv("GRASS_TRUECOLOR");
	true_color = p && strcmp(p, "TRUE") == 0;

	G_message("PS: GRASS_TRUECOLOR status: %s",
		true_color ? "TRUE" : "FALSE");

	width = screen_right - screen_left;
	height = screen_bottom - screen_top;

	init_color_table();

	outfp = fopen(file_name, "w");

	if (!outfp)
		G_fatal_error("Unable to open output file: %s", file_name);

	write_prolog();

	set_paper();

	p = getenv("GRASS_LANDSCAPE");
	landscape = p && strcmp(p, "TRUE") == 0;

	if (landscape)
	{
		int tmp = width;
		width = height;
		height = tmp;
		output("90 rotate 0 1 -1 scale\n");
	}
	else
		output("0 %d translate 1 -1 scale\n", height);

	screen_right  = screen_left + width;
	screen_bottom = screen_top + height;

	PS_Erase();

	G_message("PS: collecting to file: %s,\n     GRASS_WIDTH=%d, GRASS_HEIGHT=%d",
		file_name, width, height);

	fflush(outfp);

	return 0;
}

void output(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(outfp, fmt, va);
	va_end(va);
}

