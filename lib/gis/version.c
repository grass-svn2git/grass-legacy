/**
 * \file version.c
 *
 * \brief GRASS version functions.
 *
 * This program is free software under the GNU General Public License
 * (>=v2). Read the file COPYING that comes with GRASS for details.
 *
 * \author GRASS GIS Development Team
 *
 * \date 1999-2006
 */

#include <grass/gis.h>
#include <grass/version.h>
#include <stdlib.h>
#include <string.h>


/**
 * \fn int G_version (char **major, char **minor, char **release)
 *
 * \brief Reports GRASS version.
 *
 * <b>Note:</b> <b>major</b>, <b>minor</b>, and <b>release</b> are 
 * dynamically allocated in this function.  They must be G_free()'d by 
 * the programmer to reclaim memory.
 *
 * \param[in,out] major 
 * \param[in,out] minor
 * \param[in,out] release
 * \return always returns 0
 */

int
G_version (char **major, char **minor, char **release)
{
    int len;

    len = strlen(GRASS_VERSION_MAJOR);
    *major = G_malloc (len);
    sprintf(*major, "%s", GRASS_VERSION_MAJOR);
    
    len = strlen(GRASS_VERSION_MINOR);
    *minor = G_malloc (len);
    sprintf(*minor, "%s", GRASS_VERSION_MINOR);
    
    len = strlen(GRASS_VERSION_RELEASE);
    *release = G_malloc (len);
    sprintf(*release, "%s", GRASS_VERSION_RELEASE);

    return 0;    
}
