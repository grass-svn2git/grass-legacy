/* Function: infofile
**
** Author: Paul W. Carlson	April 1992
*/

#include <stdlib.h>
#include <string.h>
#include "map_info.h"
#include "ps_info.h"
#include "local_proto.h"

#define KEY(x) (strcmp(key,x)==0)

static char *help[] =
{
    "where      x y",
    "font       fontname",
    "fontsize   fontsize",
    "color      color",
    ""
};

int 
read_info (void)
{	
    char buf[1024];
    char *key, *data;
    int color, fontsize;
    double x, y;

    fontsize = 0;
    color = BLACK;
    x = y = 0.0;
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
	    m_info.font = G_store(data);
	    continue;
	}
	error(key, data, "illegal mapinfo sub-request");
    }
    m_info.x = x;
    m_info.y = y;
    m_info.color = color;
    if (fontsize) m_info.fontsize = fontsize;

    return 0;
}
