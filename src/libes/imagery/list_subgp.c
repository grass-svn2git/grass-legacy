#include "imagery.h"

I_list_subgroup (group, subgroup, ref, fd)
    char *group;
    char *subgroup;
    struct Ref *ref;
    FILE *fd;
{
    char buf[80];
    int i;
    int len, tot_len;
    int max;

    if (ref->nfiles <= 0)
    {
	fprintf (fd, "subgroup [%s] of group [%s] is empty\n",
	    subgroup, group);
	return;
    }
    max = 0;
    for (i=0; i < ref->nfiles; i++)
    {
	sprintf (buf, "%s in %s", ref->file[i].name, ref->file[i].mapset);
	len = strlen(buf)+4;
	if (len > max) max = len;
    }
    fprintf (fd, "subgroup [%s] of group [%s] references the following cellfiles\n", subgroup, group);
    fprintf (fd, "-------------\n");
    tot_len = 0;
    for (i=0; i < ref->nfiles; i++)
    {
	sprintf (buf, "%s in %s", ref->file[i].name, ref->file[i].mapset);
	tot_len += max;
	if (tot_len > 78)
	{
	    fprintf (fd, "\n");
	    tot_len = max;
	}
	fprintf (fd, "%-*s", max, buf);
    }
    if (tot_len)
	fprintf (fd, "\n");
    fprintf (fd, "-------------\n");
}
