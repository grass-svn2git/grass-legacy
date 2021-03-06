#include "gis.h"
#include "options.h"

static int line = 0;

read_line (buf, n)
    char *buf;
{
    if (!G_getl (buf, n, Infile))
	return 0;
    G_strip (buf);
    line++;
    return 1;
}

bad_line (buf)
    char *buf;
{
    if (isatty(fileno(Infile)))
	fprintf (stderr, "???\n");
    else
	fprintf (stderr, "%s: WARNING: line %d invalid: %s\n",
		G_program_name(), line, buf);
}
