#include "gis.h"
/*
 * Map uppercase A-Z to lower case a-z
 *
 */
static int toupper(char);
static int tolower(char);


/*!
 * \brief convert string to lower case
 *
 * Upper case
 * letters in the string <b>s</b> are converted to their lower case equivalent.
 * Returns <b>s.</b>
 *
 *  \param s
 *  \return char * 
 */

char *
G_tolcase  (char *string)

{
    register char *p;

    for (p = string; *p; p++)
	*p = tolower (*p);

    return (string);
}

static int tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
	c -= 'A' - 'a';
    return c;
}

/*
 * Map lowercase a-z to uppercase A-Z
 *
 */


/*!
 * \brief convert string to upper case
 *
 * Lower case letters in the string <b>s</b> are converted to their upper case equivalent.
 * Returns <b>s.</b>
 *
 *  \param s
 *  \return char * 
 */

char *
G_toucase  (char *string)

{
    register char *p;

    for (p = string; *p; p++)
	*p = toupper (*p);

    return (string);
}

static int toupper(char c)
{
    if (c >= 'a' && c <= 'z')
	c += 'A' - 'a';
    return c;
}
