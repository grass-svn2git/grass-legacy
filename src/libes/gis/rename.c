/***********************************************************************
 *
 *  G_rename (element, oldname, newname)
 *     char *element          element in mapset containing name
 *     char *oldname          file name to be renamed
 *     char *newname          new name for file
 *
 *  Only files in current mapset can be renamed
 *
 *  Returns  -1  on fail
 *            0  if no file
 *            1  if successful
 *
 ***********************************************************************/

#include "gis.h"

G_rename (element,oldname,newname)
    char *element;
    char *oldname;
    char *newname;
{
    char mv[1024];
    char *path;
    char *mapset;
    char xname[512], xmapset[512];

/* name in mapset legal only if mapset is current mapset */
    mapset = G_mapset();
    if (G__name_is_fully_qualified (oldname, xname, xmapset)
    && strcmp (mapset, xmapset))
	    return -1;
    if (G__name_is_fully_qualified (newname, xname, xmapset)
    && strcmp (mapset, xmapset))
	    return -1;

    strcpy (mv, "mv ");
    path = mv + strlen (mv);

/* if file does not exist return 0 */
    if (access (G__file_name (path, element, oldname, mapset),0) != 0)
	    return 0;

/* now add new name to mv command */
    path = mv + strlen (mv);
    *path++ = ' ';
    G__file_name (path, element, newname, mapset);

/* return result of the mv command */
    return system (mv) == 0 ? 1 : -1;
}
