/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:       d.erase
 * AUTHOR(S):    James Westervelt - USA CERL
 * PURPOSE:      Erase the current display frame with user defined color.
 * COPYRIGHT:    (C) 2000 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *   	    	 License (>=v2). Read the file COPYING that comes with GRASS
 *   	    	 for details.
 *
 *****************************************************************************/

#include <stdlib.h>
#include "gis.h"
#include "display.h"
#include "D.h"
#include "raster.h"

int main(int argc,char *argv[])
{
	struct Option *color;
	struct GModule *module;
	char name[256], buf[128];
	char *err;

	G_gisinit(argv[0]);

	color = G_define_option();
	color->key = "color";
	color->type = TYPE_STRING;
	color->required = NO;
	color->answer = DEFAULT_BG_COLOR;
	color->description =
	   "Color to erase with, either a standard GRASS color or R:G:B triplet (separated by colons)";

	module = G_define_module();
	module->description = 
	  "Erase the contents of the active display frame with user defined color";

	if (argc > 1 && G_parser(argc, argv))
		exit(1);

	if (R_open_driver() != 0)
		G_fatal_error ("No graphics device selected");

	err = NULL;
	if (D_get_cur_wind(name))
		err = "No current frame" ;
	else if (D_set_cur_wind(name))
		err = "Current frame not available" ;
	else {
		/* D_setup(1);
		R_standard_color(D_translate_color(color->answer));
		D_erase_window(); */
		Derase(color->answer) ;
		sprintf(buf, "d.erase color=%s", color->answer);
		D_add_to_list(buf);
	}

	R_close_driver();
	if (err)
		G_fatal_error(err) ;

	exit(0);
}
