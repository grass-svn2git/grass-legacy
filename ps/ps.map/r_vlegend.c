/* Function: vlegfile
**
** Author: Paul W. Carlson	April 1992
*/

#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "ps_info.h"
#include "local_proto.h"

#define KEY(x) (strcmp(key,x)==0)

static char *help[] =
{
    "where      x y",
    "font       fontname",
    "fontsize   fontsize",
    "width	width",
    "cols	cols",
    ""
};

int 
read_vlegend (void)
{	
    char buf[1024];
    char *key, *data;
    int fontsize, cols;
    double x, y, width;

    fontsize = 0;
    x = y = 0.0;
    width = -1;
    cols = 1;
    while (input(2, buf, help))
    {
	if (!key_data(buf, &key, &data)) continue;

        if (KEY("where"))
 	{
	    if (sscanf(data, "%lf %lf", &x, &y) != 2)
	    {
		x = y = 0.0;
		error(key, data, "illegal where request");
	    }
	    else continue;
	}

	if (KEY("fontsize"))
	{
	    fontsize = atoi(data);
	    if (fontsize < 4 || fontsize > 50) fontsize = 0;
	    continue;
	}

	if (KEY("font"))
	{
	    get_font(data);
	    vector.font = G_store(data);
	    continue;
	}

	if (KEY("width"))
	{
	    G_strip(data);
	    width = atof (data);
	    continue;
	}

	if (KEY("cols"))
	{
	    cols = atoi (data);
	    if (cols < 1 || cols > 10) cols = 1;
	    continue;
	}

	error(key, data, "illegal vlegend sub-request");
    }
    vector.x = x;
    vector.y = y;
    if (fontsize) vector.fontsize = fontsize;

    if (width > 0) vector.width = width;
    else vector.width = 3 * fontsize / 72.0;

    vector.cols = cols;

    return 0;
}
