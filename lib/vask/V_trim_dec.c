#include <grass/vask.h>
/* this routine remove trailing zeros from decimal number
 * for example: 23.45000 would come back as 23.45
 */
int V__trim_decimal ( char *buf)
{
    char *mark;

/* find the . */
    while (*buf != '.')
	if (*buf++ == '\0')
	    return 0;
    mark = buf;
    while (*++buf)
	if (*buf != '0')
	    mark = buf+1;
    while(*mark)
	*mark++ = 0;

	return 0;
}
