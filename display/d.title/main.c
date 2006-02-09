/*
****************************************************************************
*
* MODULE:       d.title
*
* AUTHOR(S):    James Westervelt, US Army CERL
*
* PURPOSE:      print out title for raster on stdout
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <grass/display.h>
#include <grass/raster.h>
#include <grass/gis.h>
#define MAIN
#include "options.h"
#include "local_proto.h"

int main (int argc, char **argv)
{
	char buff[128] ;
	char *mapset ;
	struct Cell_head window ;
	struct Categories cats ;
	struct GModule *module;
	struct Option *opt1, *opt2, *opt3 ;
	struct Flag *flag ;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	module = G_define_module();
	module->description =
		"Outputs a TITLE for a raster map layer in a form suitable "
		"for display by d.text.";

	opt1 = G_define_option() ;
	opt1->key        = "map" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = YES ;
	opt1->description= "Name of existing raster map" ;
	opt1->gisprompt  = "old,cell,raster" ;

	opt2 = G_define_option() ;
	opt2->key        = "color" ;
	opt2->type       = TYPE_STRING ;
	opt2->answer     = DEFAULT_FG_COLOR ;
	opt2->required   = NO ;
	opt2->options    = D_color_list();
	opt2->description= "Sets the text color" ;

	opt3 = G_define_option() ;
	opt3->key        = "size" ;
	opt3->type       = TYPE_DOUBLE ;
	opt3->answer     = "4.0" ;
	opt3->options    = "0-100" ;
	opt3->description= "Sets the text size as percentage of the frame's height" ;

	flag = G_define_flag() ;
	flag->key        = 'f' ;
	flag->description= "Do a fancier title" ;

	/* Check command line */
	if (G_parser(argc, argv))
		exit(-1);

	strcpy(map_name, opt1->answer) ;

	strcpy(color, opt2->answer);

	if (opt3->answer != NULL)
		sscanf(opt3->answer,"%f",&size);

	type = flag->answer ? FANCY : NORMAL ;

	if (! strlen(map_name))
		G_fatal_error("No map name given") ;

	/* Make sure map is available */
	mapset = G_find_cell (map_name, "") ;
	if (mapset == NULL)
	{
		sprintf(buff,"Cellfile [%s] not available", map_name);
		G_fatal_error(buff) ;
	}
	if (G_get_cellhd(map_name, mapset, &window) == -1)
	{
		sprintf(buff,"Cell head file for [%s] not available", map_name) ;
		G_fatal_error(buff) ;
	}

	if (G_read_cats(map_name, mapset, &cats) == -1)
	{
		sprintf(buff,"Category file for [%s] not available", map_name) ;
		G_fatal_error(buff) ;
	}
	if (type == NORMAL)
		normal(mapset, &window, &cats) ;
	else
		fancy(mapset, &window, &cats) ;

	exit(0);
}
