#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gis.h"
#include "local_proto.h"
#include "glocale.h"

int main (int argc, char *argv[])
{
        int as_int;
	int verbose;
	int compact;
	int range;
	int windowed;
	int nsteps;
	char name[GNAME_MAX];
	char *mapset;
	char *no_data_str;
	struct GModule *module;
	struct
	{
	  struct Flag *one;
	  struct Flag *r;
	  struct Flag *q;
	  struct Flag *d;
	  struct Flag *i;
	} flag;
	struct
	{
	  struct Option *map;
	  struct Option *nv;
	  struct Option *nsteps;
	} option;

	G_gisinit (argv[0]);

	module = G_define_module();
	module->description =
		_("Prints terse list of category values found in a raster map layer.");

	/* define different options */
	option.map = G_define_standard_option(G_OPT_R_MAP);

	option.nv = G_define_option() ;
	option.nv->key		= "nv" ;
	option.nv->type		= TYPE_STRING ;
	option.nv->required	= NO ;
	option.nv->multiple	= NO ;
	option.nv->answer	= "*" ;
	option.nv->description	= _("string representing no data cell value");

        option.nsteps = G_define_option() ;
        option.nsteps->key          = "nsteps" ;
        option.nsteps->type         = TYPE_INTEGER ;
        option.nsteps->required     = NO ;
        option.nsteps->multiple     = NO ;
        option.nsteps->answer       = "255" ;
        option.nsteps->description  = _("number of quantization steps");

	/*define the different flags */

	flag.one =G_define_flag() ;
	flag.one->key         = '1';
	flag.one->description = _("Print the output one value per line");

	flag.r =G_define_flag() ;
	flag.r->key         = 'r';
	flag.r->description = _("Only print the range of the data");

	flag.q =G_define_flag() ;
	flag.q->key        = 'q';
	flag.q->description = _("Quiet");

	flag.d =G_define_flag() ;
	flag.d->key        = 'd';
	flag.d->description = _("Use the current region");

	flag.i =G_define_flag() ;
	flag.i->key        = 'i';
	flag.i->description = _("Read fp map as integer");

	verbose = 1;

	if (0 > G_parser(argc,argv))
		exit(EXIT_FAILURE);

	verbose = (! flag.q->answer);
	compact = (! flag.one->answer);
	range =  flag.r->answer;
	windowed =  flag.d->answer;
	as_int =  flag.i->answer;
	no_data_str =  option.nv->answer;
	if (sscanf(option.nsteps->answer, "%d", &nsteps) != 1 || nsteps < 1)
	  G_fatal_error ( _("%s = %s -- must be greater than zero"),
               option.nsteps->key,option.nsteps->answer);
	strcpy (name, option.map->answer);

	if (mapset =  G_find_cell2 (name, ""))
	{
		describe(name, mapset, compact, verbose, no_data_str,
			range, windowed, nsteps, as_int);
		exit(EXIT_SUCCESS);
	}
	G_fatal_error ( _("%s: [%s] not found"), G_program_name(), name);
}
