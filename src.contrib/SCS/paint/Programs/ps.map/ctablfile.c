/* Function: ctablfile
**
** Author: Paul W. Carlson	April 1992
*/

#include "ps_info.h"
#include "colortable.h"

#define KEY(x) (strcmp(key,x)==0)

static char *help[] =
{
    "where  x y",
    "width  table_width",
    "cols   columns",
    "font   fontname",
    "size   fontsize",
    "color  color",
    ""
};

ctablfile()
{	
    char buf[1024];
    char *key, *data;
    int color, size, cols;
    double w, x, y;

    size = 0;
    color = BLACK;
    cols = 1;
    w = x = y = 0.0;
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

        if (KEY("width"))
 	{
	    if (sscanf(data, "%lf", &w) != 1)
	    {
		w = 0.0;
		error(key, data, "illegal width request");
	    }
	    else continue;
	}

        if (KEY("cols"))
 	{
	    if (sscanf(data, "%d", &cols) != 1)
	    {
		cols= 1;
		error(key, data, "illegal columns request");
	    }
	    else continue;
	}

	if (KEY("size"))
	{
	    size = atoi(data);
	    if (size < 4 || size > 50) size = 0;
	    continue;
	}

	if (KEY("color"))
	{
	    color = get_color_number(data);
	    if (color < 0)
	    {
		color = BLACK;
		error(key, data, "illegal color request");
	    }
	    continue;
	}

	if (KEY("font"))
	{
	    get_font(data);
	    ct.font = G_store(data);
	    continue;
	}
	error(key, data, "illegal colortabe sub-request");
    }
    ct.x = x;
    ct.y = y;
    ct.width = w;
    ct.color = color;
    ct.cols = cols;
    if (size) ct.size = size;
}
