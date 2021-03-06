#include "globals.h"
prepare_group_list ()
{
    FILE *fd;
    int *idx;
    int n;
    int cmp();
    int len,len1,len2;

/* open file to store group file names */
    fd = fopen (group_list, "w");
    if (fd == NULL)
	G_fatal_error ("Can't open any tempfiles");

/*
 * build sorted index into group files
 * so that all cell files for a mapset to appear together
 */
    idx = (int *) G_calloc (group.group_ref.nfiles, sizeof (int));
    for (n = 0; n < group.group_ref.nfiles; n++)
	idx[n] = n;
    qsort (idx, group.group_ref.nfiles, sizeof(int), cmp);

/* determine length of longest mapset name, and longest cell file name */
    len1 = len2 = 0;
    for (n = 0; n < group.group_ref.nfiles; n++)
    {
	len = strlen (group.group_ref.file[n].name);
	if (len > len1)
	    len1 = len;
	len = strlen (group.group_ref.file[n].mapset);
	if (len > len2)
	    len2 = len;
    }

/* write lengths, names to file */
    fwrite (&len1, sizeof (len1), 1, fd);
    fwrite (&len2, sizeof (len2), 1, fd);
    for (n = 0; n < group.group_ref.nfiles; n++)
	fprintf (fd, "%s %s\n", group.group_ref.file[idx[n]].name, 
			group.group_ref.file[idx[n]].mapset);
    fclose (fd);

    free (idx);
}

static
cmp(a,b)
    int *a, *b;
{
    int n;

    if(n = strcmp (group.group_ref.file[*a].mapset, group.group_ref.file[*b].
			mapset))
	return n;
    return strcmp (group.group_ref.file[*a].name, group.group_ref.file[*b].name);
}

/* ask the user to pick a file */
choose_groupfile (name, mapset)
    char *name, *mapset;
{
    return ask_gis_files ("cell", group_list, name, mapset, -1);
}
