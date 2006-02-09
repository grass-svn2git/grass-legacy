#include <grass/gis.h>
#include <grass/glocale.h>
/**********************************************************************
 *
 *   char *
 *   G_legal_filename (name)
 *      char *name           filename to be checked
 *
 *   returns:    1  if name is OK
 *              -1  if name begins with ".", if name contains a "/",
 *                  if name contains a quote character,
 *                  or if name contains a non-printing character.
 **********************************************************************/


/*!
 * \brief check for legal database file names
 *
 * Returns 1 if <b>name</b> is ok, -1 otherwise.
 *
 *  \param name
 *  \return int
 */

int G_legal_filename (char *s)
{
    if (*s == '.' || *s == 0) {
	fprintf(stderr, _("Illegal filename.  Cannot be '.' or 'NULL'\n"));
	return -1;
    }

    for ( ; *s; s++)
	if (*s == '/' || *s == '"' || *s == '\'' || *s <= ' ' || *s > 0176) {
		fprintf(stderr, _("Illegal filename. character <%c> not allowed."), *s);
	    return -1;
	}

    return 1;
}
