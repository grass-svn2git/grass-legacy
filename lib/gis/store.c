#include "gis.h"
/*
 *******************************************************************
 * G_store (string)
 *
 * allocates permanent memory to hold 'string'
 * copies 'string' to this memory, and returns the address
 * of the allocated memory
 *******************************************************************/
#include <string.h>


/*!
 * \brief copy string to allocated memory
 *
 * This routine
 * allocates enough memory to hold the string <b>s</b>, copies <b>s</b> to
 * the allocated memory, and returns a pointer to the allocated memory.
 *
 *  \param s
 *  \return char * 
 */

char *G_store (s) char *s;
{
    char *buf;

    buf = G_malloc (strlen(s) + 1);
    strcpy (buf, s);
    return buf;
}
