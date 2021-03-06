#include "defs.h"

void
parse (argc, argv, parms)
    int argc;
    char *argv[];
    struct Parms *parms;
{
    struct Option *maps, *fs;
    struct Flag *quiet, *labels;
    char *name, *mapset;

    maps = G_define_option();

    maps->key = "maps";
    maps->key_desc = "map1,map2";
    maps->required = YES;
    maps->multiple = NO;
    maps->type = TYPE_STRING;
    maps->description = "maps for computing inter-class distances";
    maps->gisprompt = "old,cell,raster";

    fs  = G_define_option();
    fs->key = "fs";
    fs->required = NO;
    fs->multiple = NO;
    fs->answer   = ":";       /* colon is default output fs */
    fs->type = TYPE_STRING;
    fs->description = "output field separator";

    labels = G_define_flag();
    labels -> key = 'l';
    labels->description = "include category labels in the output";

    quiet = G_define_flag();
    quiet -> key = 'q';
    quiet->description = "run quietly";

    if (G_parser(argc, argv))
	exit(0);

    name = parms->map1.name = maps->answers[0];
    mapset = parms->map1.mapset = G_find_cell (name, "");
    if (mapset == NULL)
    {
	fprintf (stderr, "%s not found\n", name);
	exit(1);
    }
    parms->map1.fullname = G_fully_qualified_name (name,mapset);

    name = parms->map2.name = maps->answers[1];
    mapset = parms->map2.mapset = G_find_cell (name, "");
    if (mapset == NULL)
    {
	fprintf (stderr, "%s not found\n", name);
	exit(1);
    }
    parms->map2.fullname = G_fully_qualified_name (name,mapset);

    parms->verbose = quiet->answer ? 0 : 1;
    parms->labels = labels->answer ? 1 : 0;
    parms->fs = fs->answer;
}
