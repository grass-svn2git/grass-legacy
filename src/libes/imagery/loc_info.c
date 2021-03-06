
#include "gis.h"

/* makes a three part title with location, mapset info */
I_location_info(buf,middle)
    char *buf;
    char *middle;
{
    char left[80];
    char right[80];
    int len;

    sprintf (left, "LOCATION: %s", G_location());
    sprintf (right, "MAPSET: %s", G_mapset());
    len = 79-strlen(left)-strlen(middle)-strlen(right);
    sprintf (buf, "%s%*s%s%*s%s",
	left, len/2, "", middle,  len/2, "", right);
}
