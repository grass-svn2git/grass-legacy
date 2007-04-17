#include <string.h>
#include <grass/imagery.h>

FILE *I_fopen_signature_file_new (
    char *group,
    char *subgroup,
    char *name)
{
    char element[200];
    FILE *fd;

    sprintf (element, "group/%s/subgroup/%s/sig", group, subgroup);

    fd = G_fopen_new (element, name);
    if (fd == NULL)
	G_warning ("unable to create signature file %s for subgroup %s of group %s",
		   name, subgroup, group);
    return fd;
}

FILE *I_fopen_signature_file_old(
    char *group,
    char *subgroup,
    char *name)
{
    char element[200];
    FILE *fd;

    sprintf (element, "group/%s/subgroup/%s/sig", group, subgroup);

    fd = G_fopen_old (element, name, G_mapset());
    if (fd == NULL)
	G_warning ("unable to open signature file %s for subgroup %s of group [%s in %s]",
		   name, subgroup, group, G_mapset());
    return fd;
}

