#include "gis.h"
#include "parms.h"
parse(argc,argv,parms)
    int argc;
    char *argv[];
    struct parms *parms;
{
    struct Option *group, *subgroup, *sigfile, *trainingmap;

    trainingmap = G_define_option();
    trainingmap->key = "trainingmap";
    trainingmap->description = "ground truth training map";
    trainingmap->required = YES;
    trainingmap->type = TYPE_STRING;
    trainingmap->gisprompt = "old,cell,raster";

    group = G_define_option();
    group->key = "group";
    group->description = "imagery group";
    group->required = YES;
    group->type = TYPE_STRING;
    group->gisprompt = "old,group,group";

    subgroup = G_define_option();
    subgroup->key = "subgroup";
    subgroup->description = "subgroup containing image files";
    subgroup->required = YES;
    subgroup->type = TYPE_STRING;

    sigfile = G_define_option();
    sigfile->key = "signaturefile";
    sigfile->description = "resultant signature file";
    sigfile->required = YES;
    sigfile->type = TYPE_STRING;

    if (G_parser(argc,argv)) exit(1);

    parms->training_map = trainingmap->answer;
    parms->group = group->answer;
    parms->subgroup = subgroup->answer;
    parms->sigfile = sigfile->answer;

/* check all the inputs */
    if(G_find_cell(parms->training_map, "") == NULL)
    {
	fprintf (stderr, "ERROR: training map [%s] not found\n", parms->training_map);
	exit(1);
    }
    if (!I_find_group(parms->group))
    {
	fprintf (stderr, "ERROR: group [%s] not found\n", parms->group);
	exit(1);
    }
    if (!I_find_subgroup(parms->group, parms->subgroup))
    {
	fprintf (stderr, "ERROR: subgroup [%s] not found\n", parms->subgroup);
	exit(1);
    }
}
