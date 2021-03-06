#include <stdio.h>

static struct
{
    char *name;
    float r,g,b;
}colors[] =
{
    "white",	1.00, 1.00, 1.00,
    "black",	0.00, 0.00, 0.00,
    "red",	1.00, 0.00, 0.00,
    "green",	0.00, 1.00, 0.00,
    "blue",	0.00, 0.00, 1.00,
    "yellow",	1.00, 1.00, 0.00,
    "magenta",	1.00, 0.00, 1.00,
    "cyan",	0.00, 1.00, 1.00,
    "aqua",     0.00, 0.75, 0.75,
    "grey",	0.75, 0.75, 0.75,
    "gray",	0.75, 0.75, 0.75,
    "orange",	1.00, 0.50, 0.00,
    "brown",	0.75, 0.50, 0.25,
    "purple",	0.50, 0.00, 1.00,
    "violet",	0.50, 0.00, 1.00,
    "indigo",   0.00, 0.50, 1.00,

    "",0.00,0.00,0.00	/* do not modify this line */
} ;

G_color_values (name, r, g, b)
    char *name;
    float *r, *g, *b;
{
    int i;

    *r = *g = *b = 0.0;
    for (i = 0; colors[i].name[0]; i++)
	if (strcmp (name, colors[i].name) == 0)
	{
	    *r = colors[i].r;
	    *g = colors[i].g;
	    *b = colors[i].b;
	    return 1;
	}
    return -1;
}

char *
G_color_name (n)
{
    int i;

    if (n >= 0)
	for (i = 0; colors[i].name[0]; i++)
	    if (i == n)
		return colors[i].name;
    return (char *) NULL;
}
