/*!
   \file list_subgp.c
   
   \brief Imagery Library - List subgroup
 
   (C) 2001-2008 by the GRASS Development Team
   
   This program is free software under the GNU General Public License
   (>=v2). Read the file COPYING that comes with GRASS for details.
   
   \author USA CERL
*/

#include <string.h>
#include <grass/imagery.h>
#include <grass/glocale.h>

/*!
 * \brief Prints maps in a subgroup (fancy version)
 *
 * \param group group name
 * \param subgroup subgroup name
 * \param ref group reference (set with I_get_subgroup_ref())
 * \param fd where to print (typically stdout)
 * \return 0
 */
int I_list_subgroup(const char *group,
		    const char *subgroup, const struct Ref *ref, FILE * fd)
{
    char buf[80];
    int i;
    int len, tot_len;
    int max;

    if (ref->nfiles <= 0) {
	fprintf(fd, _("subgroup <%s> of group <%s> is empty\n"),
		subgroup, group);
	return 0;
    }
    max = 0;
    for (i = 0; i < ref->nfiles; i++) {
	sprintf(buf, "<%s@%s>", ref->file[i].name, ref->file[i].mapset);
	len = strlen(buf) + 4;
	if (len > max)
	    max = len;
    }
    fprintf(fd,
	    _("subgroup <%s> of group <%s> references the following raster maps\n"),
	    subgroup, group);
    fprintf(fd, "-------------\n");
    tot_len = 0;
    for (i = 0; i < ref->nfiles; i++) {
	sprintf(buf, "<%s@%s>", ref->file[i].name, ref->file[i].mapset);
	tot_len += max;
	if (tot_len > 78) {
	    fprintf(fd, "\n");
	    tot_len = max;
	}
	fprintf(fd, "%-*s", max, buf);
    }
    if (tot_len)
	fprintf(fd, "\n");
    fprintf(fd, "-------------\n");

    return 0;
}

/*!
 * \brief Prints maps in a subgroup (simple version)
 *
 * Same as I_list_subgroup(), but without all the fancy stuff.
 * Prints one map per line in map@mapset form.
 *
 * \param ref group reference (set with I_get_subgroup_ref())
 * \param fd where to print (typically stdout)
 * \return 0
 */
/* same as above, but one map per line in map@mapset form */
int I_list_subgroup_simple(const struct Ref *ref, FILE * fd)
{
    return I_list_group_simple(ref, fd);
}
