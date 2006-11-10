#define MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "global.h"

int 
main (int argc, char *argv[])
{
    char *title;
    struct GModule *module;
    struct
    {
	struct Option *input, *output, *title;
	struct Flag *a, *d;
    } parm;

    /* any interaction must run in a term window */
    G_putenv("GRASS_UI_TERM","1");

    G_gisinit (argv[0]);

    module = G_define_module();
    module->keywords = _("raster");
    module->description = _("Recode categorical raster maps.");
		        
    parm.input = G_define_standard_option(G_OPT_R_INPUT);
    parm.input->description =  _("Raster map to be recoded");

    parm.output = G_define_standard_option(G_OPT_R_OUTPUT);

    parm.title = G_define_option();
    parm.title->key = "title";
    parm.title->required = NO;
    parm.title->type = TYPE_STRING;
    parm.title->description =  _("Title for the resulting raster map");

    parm.a = G_define_flag() ;
    parm.a->key         = 'a' ;
    parm.a->description = _("Align the current region to the input map");

    parm.d = G_define_flag() ;
    parm.d->key         = 'd' ;
    parm.d->description = _("Force output to double map type (DCELL)");

    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);


    name       = parm.input->answer;
    result     = parm.output->answer;
    title      = parm.title->answer;
    align_wind = parm.a->answer;
    make_dcell = parm.d->answer;


    mapset = G_find_cell2 (name, "");
    if (mapset == NULL)
	G_fatal_error(_("Raster map [%s] not found"), name);

    if (G_legal_filename(result) < 0)
	G_fatal_error(_("[%s] is an illegal file name"), result);

    if (strcmp(name,result)==0 && strcmp(mapset,G_mapset())== 0)
	G_fatal_error(_("input map can NOT be the same as output map"));

    if (!read_rules())
    {
	if (isatty(0))
	    G_fatal_error(_("no rules specified. [%s] not created"), result);
	else
	    G_fatal_error (_("no rules specified"));
    }

    no_mask = 0;

    do_recode();

    exit(EXIT_SUCCESS);
}
