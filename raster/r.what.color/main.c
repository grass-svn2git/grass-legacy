/****************************************************************************
 *
 * MODULE:       r.what
 * AUTHOR(S):    Michael Shapiro, CERL (original contributor)
 *               Markus Neteler <neteler itc.it>,Brad Douglas <rez touchofmadness.com>,
 *               Huidae Cho <grass4u gmail.com>, Glynn Clements <glynn gclements.plus.com>,
 *               Hamish Bowman <hamish_nospam yahoo.com>, Soeren Gebbert <soeren.gebbert gmx.de>
 * PURPOSE:      
 * COPYRIGHT:    (C) 1999-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>

static int do_value(const char *buf, RASTER_MAP_TYPE type, struct Colors *colors)
{
	CELL ival;
	DCELL fval;
	int red, grn, blu;

	switch (type)
	{
	case CELL_TYPE:
		if (sscanf(buf, "%d", &ival) != 1)
			return 0;
		if (!G_get_c_raster_color(&ival, &red, &grn, &blu, colors))
			return 0;
		fprintf(stdout, "%d: %02x:%02x:%02x\n", ival, red, grn, blu);
		return 1;

	case FCELL_TYPE:
	case DCELL_TYPE:
		if (sscanf(buf, "%lf", &fval) != 1)
			return 0;
		if (!G_get_d_raster_color(&fval, &red, &grn, &blu, colors))
			return 0;
		fprintf(stdout, "%f: %02x:%02x:%02x\n", fval, red, grn, blu);
		return 1;
	default:
		return 0;
	}
}

int main(int argc, char **argv)
{
	struct GModule *module;
	struct {
		struct Option *input, *value;
	} opt;
	struct {
		struct Flag *i;
	} flag;
	const char *name;
	const char *mapset;
	struct Colors colors;
	RASTER_MAP_TYPE type;
  
	G_gisinit (argv[0]);
  
	module           = G_define_module();
	module->keywords = _("raster");
	module->description = 
		_("Queries raster map layers on their category values and category labels.");

	opt.input = G_define_option() ;
	opt.input->key          = "input" ;
	opt.input->type         = TYPE_STRING ;
	opt.input->required     = YES ;
	opt.input->multiple     = NO ;
	opt.input->gisprompt    = "old,cell,raster" ;
	opt.input->description  = _("Name of existing raster map to query colors");

	opt.value = G_define_option() ;
	opt.value->key          = "value";
	opt.value->type         = TYPE_DOUBLE;
	opt.value->required     = NO;
	opt.value->multiple     = YES;
	opt.value->description  = _("Values to query colors for");

	flag.i = G_define_flag() ;
	flag.i->key             = 'i' ;
	flag.i->description     = _("Read values from stdin") ;

	if (G_parser(argc, argv))
		exit(EXIT_FAILURE);

	if (!opt.value->answer && !flag.i->answer)
		G_fatal_error(_("Either \"-i\" or \"value=\" must be given"));

	name = opt.input->answer;
	mapset = G_find_cell2(name, "");

	if (!mapset)
		G_fatal_error("Input map %s not found", name);

	type = G_raster_map_type2(name, mapset);
	if (type < 0)
		G_fatal_error("Unable to determine type of input map %s", name);

	if (G_read_colors(name, mapset, &colors) < 0)
		G_fatal_error("Unable to read colors for input map %s", name);

	if (flag.i->answer)
	{
		for (;;)
		{
			char buf[64];

			if (!fgets(buf, sizeof(buf), stdin))
				break;

			do_value(buf, type, &colors);
		}
	}
	else if (opt.value->answer)
	{
		const char *ans;
		int i;

		for (i = 0; ans = opt.value->answers[i], ans; i++)
			do_value(ans, type, &colors);
	}

	return EXIT_SUCCESS;
}

