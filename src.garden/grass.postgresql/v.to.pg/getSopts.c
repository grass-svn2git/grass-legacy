#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "dbvect.h"
#include "display.h"
#include "glocale.h"

int getSelectOpts(argc, argv)
     int argc;
     char **argv;


{

    char *mapset;
    int colr, fillcolr, i, retval;
    FILE *fp;
    char SQL_stmt[QRY_LENGTH];

    struct Option *map, *vtype, *color, *sql, *rmap;
    struct Flag *select, *flag1, *flag3, *flag4;

    memset(SQL_stmt, '\0', sizeof(SQL_stmt));

    retval = 0;

    select = G_define_flag();
    select->key = 's';
    select->description =
	_("Use [s] flag to select db records using an input file.");

    map = G_define_option();
    map->key = "map";
    map->type = TYPE_STRING;
    map->gisprompt = "old,dig,vector";
    map->required = YES;
    map->multiple = NO;
    map->description = _("Name of existing vector file.");

    vtype = G_define_option();
    vtype->key = "type";
    vtype->type = TYPE_STRING;
    vtype->required = YES;
    vtype->multiple = NO;
    vtype->options = "area,line";
    vtype->description = _("Select area or line.");

    sql = G_define_option();
    sql->key = "sql";
    sql->key_desc = "file";
    sql->type = TYPE_STRING;
    sql->required = YES;
    sql->multiple = NO;
    sql->description = _("SQL statements specifying selection criteria. ");

    color = G_define_option();
    color->key = "color";
    color->type = TYPE_STRING;
    color->required = NO;
    color->multiple = NO;
    color->description = _("Color for vector draw.");

    rmap = G_define_option();
    rmap->key = "rmap";
    rmap->gisprompt = "old,cell,raster";
    rmap->type = TYPE_STRING;
    rmap->required = NO;
    rmap->multiple = NO;
    rmap->description = _("Raster map for Z-coord (may be used with PostGIS).");

    flag1 = G_define_flag();
    flag1->key = 'f';
    flag1->description = _("Fill polygons");

    flag3 = G_define_flag();
    flag3->key = 'v';
    flag3->description = _("Verbose mode");

    flag4 = G_define_flag();
    flag4->key = 'p';
    flag4->description = _("Create and populate PostGIS table instead");



    /* Check for help flag */
    for (i = 0; i < argc; i++)
	if (strcmp(argv[i], "help") == 0)
	    argv[1] = "help";

    if ((argc == 2) && (strcmp(argv[1], "-s") == 0)) {	/* Run interactive parser */
	/*argv[1] == NULL ; */
	argc = 1;
    }


    /* Invoke parser */
    if (G_parser(argc, argv))
	exit(-1);
    if (color->answer == NULL)
	colr = D_translate_color("white");
    else
	colr = D_translate_color(color->answer);

    map_string = map->answer;
    vtype_string = vtype->answer;
    fillcolr = flag1->answer;
    verbose = flag3->answer;
    to_postgis = flag4->answer;
    rmap_string = rmap->answer;



    if ((mapset = G_find_file2("dig", map->answer, "")) == NULL) {
	fprintf(stderr, _("Vector file %s not found.\n"), map->answer);
	exit(-1);
    }

    if ((fp = fopen(sql->answer, "r")) == NULL) {
	fprintf(stderr, _("File read error on %s\n"), sql->answer);
	exit(-1);
    }

    fread(SQL_stmt, QRY_LENGTH, 1, fp);
    /* read all lines of sql stmt into a var  */
    fclose(fp);

    runPg(SQL_stmt, map->answer, mapset, colr, fillcolr);

    exit(0);

}
