#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "display.h"
#include "D.h"
#include "raster.h"
#define GLOBAL
#include "options.h"
#include "local_proto.h"
#include "colors.h"

int main (int argc, char **argv)
{
	char *mapset;
	char msg[200];
	char window_name[64] ;
	int t, b, l, r ;
	int R, G, B;
	const int customcolor = MAXCOLORS + 1;
	struct Cell_head window ;
	struct GModule *module;
	struct Option *opt1, *opt2, *opt3, *opt4;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	module = G_define_module();
	module->description =
		"Displays site markers in the active display frame "
		"on the graphics monitor.";

	opt4 = G_define_option() ;
	opt4->key        = "sitefile";
	opt4->type       = TYPE_STRING;
	opt4->required   = YES;
	opt4->gisprompt  = "old,site_lists,sites";
	opt4->description= "Name of a site file" ;

	opt1 = G_define_option() ;
	opt1->key        = "color" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = NO ;
	opt1->answer     = "gray" ;
/*	opt1->options    = D_color_list(); */
	opt1->description=
	    "Either a GRASS standard color or R:G:B triplet (separated by colons)";

	opt2 = G_define_option() ;
	opt2->key        = "size" ;
	opt2->type       = TYPE_INTEGER ;
	opt2->required   = NO ;
	opt2->answer     = "5" ;
	opt2->options    = "0-1000" ;
	opt2->description= "Size, in pixels, in which the icon is to be drawn" ;

	opt3 = G_define_option() ;
	opt3->key        = "type" ;
	opt3->type       = TYPE_STRING ;
	opt3->required   = NO ;
	opt3->answer     = "x" ;
	opt3->options    = "x,diamond,box,+" ;
	opt3->description= "Specify the type of the icon" ;

	/* Check command line */

	if (G_parser(argc, argv))
		exit(1);

        /* Parse and select text color */
	if(sscanf(opt1->answer, "%d:%d:%d", &R, &G, &B) == 3) {
		if (R>=0 && R<256 && G>=0 && G<256 && B>=0 && B<256) {
			R_reset_color(R, G, B, customcolor);
			color = customcolor;
		}
		else
			color = 0;
	}
	else {
		color = D_translate_color(opt1->answer);
	}

	if(!color)
		G_fatal_error("[%s]: No such color", opt1->answer);


	mapset = G_find_file ("site_lists", opt4->answer, "");
	if (mapset == NULL)
		G_fatal_error ("sites file [%s] not found", opt4->answer);

	infile = G_fopen_sites_old (opt4->answer, mapset);
	if (infile == NULL)
	{
		sprintf (msg, "can't open sites file [%s]", opt4->answer);
		G_fatal_error (msg);
	}

	sscanf(opt2->answer,"%d",&size);

	if (! strcmp(opt3->answer, "x"))
		type = TYPE_X ;
	else if (! strcmp(opt3->answer, "+"))
		type = TYPE_PLUS ;
	else if (! strcmp(opt3->answer, "box"))
		type = TYPE_BOX ;
	else if (! strcmp(opt3->answer, "diamond"))
		type = TYPE_DIAMOND ;

	/* Setup driver and check important information */
	if (R_open_driver() != 0)
		G_fatal_error ("No graphics device selected");

	if (D_get_cur_wind(window_name))
		G_fatal_error("No current frame") ;

	if (D_set_cur_wind(window_name))
		G_fatal_error("Current frame not available") ;

	/* Read in the map window associated with window */
	G_get_window(&window) ;

	if (D_check_map_window(&window))
		G_fatal_error("Setting map region") ;

	if (G_set_window(&window) == -1)
		G_fatal_error("Current frame not settable") ;

	/* Determine conversion factors */
	if (D_get_screen_window(&t, &b, &l, &r))
		G_fatal_error("Getting screen frame") ;
	if (D_do_conversions(&window, t, b, l, r))
		G_fatal_error("Error in calculating conversions") ;

	/* Do the plotting */
	if(color > MAXCOLORS)     /* ie custom RGB color */
		R_color(color);
	else
		R_standard_color(color);

	switch(type)
	{
	case TYPE_X:
		draw_points_x(&window) ;
		break ;
	case TYPE_PLUS:
		draw_points_plus(&window) ;
		break ;
	case TYPE_BOX:
		draw_points_box(&window) ;
		break ;
	case TYPE_DIAMOND:
		draw_points_diamond(&window) ;
		break ;
	}

	D_add_to_list(G_recreate_command()) ;

	D_set_site_name(G_fully_qualified_name(opt4->answer, mapset));
	D_add_to_site_list(G_fully_qualified_name(opt4->answer, mapset));

	R_close_driver();
	exit(0);
}
