/*************************************************************
* I_number_of_group_ref_files (group)
* I_number_of_subgroup_ref_files (group, subgroup)
*************************************************************/
#include <stdio.h>

I_number_of_group_ref_files (group)
    char *group;
{
    return nfiles(group,"");
}

I_number_of_subgroup_ref_files (group, subgroup)
    char *group;
    char *subgroup;
{
    return nfiles(group,subgroup);
}

static
nfiles (group, subgroup)
    char *group;
    char *subgroup;
{
    FILE *fd;
    FILE *I_fopen_group_ref_old();
    FILE *I_fopen_subgroup_ref_old();
    int n;
    char buf[1024];
    char name[30], mapset[30];

    G_suppress_warnings(1);
    n = 0;
    if (*subgroup == 0)
	fd = I_fopen_group_ref_old (group);
    else
	fd = I_fopen_subgroup_ref_old (group, subgroup);
    G_suppress_warnings(0);

    if (fd)
    {
	while (fgets(buf, sizeof buf, fd))
	    if (sscanf (buf, "%s %s", name, mapset) == 2)
		n++;
	fclose (fd);
    }

    return n;
}
