/*  index ()
 *  rindex ()
 */
#include "gis.h"

char *
index (str, delim)
    register char *str, delim;
{
    while (*str && *str != delim)
	str++;
    if (delim == 0)
	return str;
    return (*str ? str : NULL);
}

char *
rindex (str, delim)
    register char *str, delim;
{
    register char *p;

    p = NULL;
    while (*str)
    {
	if (*str == delim)
	    p = str;
	str ++;
    }
    if (delim == 0)
	return str;
    return (p);
}
