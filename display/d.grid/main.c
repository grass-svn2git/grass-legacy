/****************************************************************
 *
 * MODULE:       d.grid
 * 
 * AUTHOR(S):    James Westervelt, U.S. Army CERL
 *               Geogrid support: Bob Covill, www.tekmap.ns.ca
 *               
 * PURPOSE:      Draw the coordinate grid the user wants displayed on
 *               top of the current image
 *               
 * COPYRIGHT:    (C) 1999, 2005 by the GRASS Development Team
 *
 *               This program is free software under the 
 *               GNU General Public License (>=v2). 
 *               Read the file COPYING that comes with GRASS
 *               for details.
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/display.h>
#include <grass/raster.h>
#include <grass/gprojects.h>
#include <grass/glocale.h>

#include "local_proto.h"


int 
main (int argc, char **argv)
{
	int colorg = 0;
	int colorb = 0;
	double size=0., gsize=0.; /* initialize to zero */
	double east, north;
	int do_text, fontsize;
	struct GModule *module;
	struct Option *opt1, *opt2, *opt3, *opt4, *fsize;
	struct Flag *noborder, *notext, *geogrid, *nogrid, *wgs84;
	struct pj_info info_in;  /* Proj structures */
	struct pj_info info_out; /* Proj structures */

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	module = G_define_module();
	module->keywords = _("display, cartography");
	module->description =
		_("Overlays a user-specified grid "
		"in the active display frame on the graphics monitor.");

	opt2 = G_define_option() ;
	opt2->key        = "size" ;
	opt2->key_desc   = "value" ;
	opt2->type       = TYPE_STRING ;
	opt2->required   = YES;
	opt2->label      = _("Size of grid to be drawn");
	opt2->description= _("In map units or DDD:MM:SS format. "
			     "Example: \"1000\" or \"0:10\"");

	opt1 = G_define_option() ;
	opt1->key        = "color" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = NO;
	opt1->answer     = "gray" ;
	opt1->description=
	    _("Sets the grid color, either a standard GRASS color or R:G:B triplet (separated by colons)");
	opt1->gisprompt  = GISPROMPT_COLOR ;
	    
	opt3 = G_define_option() ;
	opt3->key        = "origin" ;
	opt3->type       = TYPE_STRING ;
	opt3->key_desc   = "easting,northing" ;
	opt3->answer     = "0,0" ;
	opt3->multiple   = NO;
	opt3->description= _("Lines of the grid pass through this coordinate") ;

	opt4 = G_define_option() ;
	opt4->key        = "bordercolor" ;
	opt4->type       = TYPE_STRING ;
	opt4->required   = NO;
	opt4->answer     = DEFAULT_FG_COLOR;
	opt4->description=
	    _("Sets the border color, either a standard GRASS color or R:G:B triplet");
	opt4->gisprompt  = GISPROMPT_COLOR ;

	fsize = G_define_option();
	fsize->key	  = "fontsize";
	fsize->type	  = TYPE_INTEGER;
	fsize->required   = NO;
	fsize->answer     = "9";
	fsize->options    = "1-72";
	fsize->description= _("Font size for gridline coordinate labels");

	geogrid = G_define_flag();
	geogrid->key = 'g';
	geogrid->description =
	    _("Draw geographic grid (referenced to current ellipsoid)");

	wgs84 = G_define_flag();
	wgs84->key = 'w';
	wgs84->description =
	    _("Draw geographic grid (referenced to WGS84 ellipsoid)");

	nogrid = G_define_flag();
	nogrid->key = 'n';
	nogrid->description = _("Disable grid drawing");

	noborder = G_define_flag();
	noborder->key = 'b';
	noborder->description = _("Disable border drawing");

	notext = G_define_flag();
	notext->key = 't';
	notext->description = _("Disable text drawing");


	/* Check command line */
	if (G_parser(argc, argv))
		exit(EXIT_FAILURE);


	/* do some checking */
	if (nogrid->answer && noborder->answer)
		G_fatal_error(_("Both grid and border drawing are disabled"));
	if (wgs84->answer)
		geogrid->answer = 1; /* -w implies -g */
	if (geogrid->answer && G_projection() == PROJECTION_LL)
		G_fatal_error(_("Geo-Grid option is not available for LL projection"));
	if (geogrid->answer && G_projection() == PROJECTION_XY)
		G_fatal_error(_("Geo-Grid option is not available for XY projection"));
	
	if(notext->answer) do_text = FALSE;
	else do_text = TRUE;

	fontsize = atoi(fsize->answer);

	/* get grid size */
	if (geogrid->answer)
	{
		if(!G_scan_resolution (opt2->answer, &gsize, PROJECTION_LL) || gsize <= 0.0)
			G_fatal_error (_("Invalid geo-grid size <%s>"), opt2->answer);
	} else {
		if(!G_scan_resolution (opt2->answer, &size, G_projection()) || size <= 0.0)
			G_fatal_error (_("Invalid grid size <%s>"), opt2->answer);
	}

	/* get grid easting start */
	if(!G_scan_easting(opt3->answers[0], &east, G_projection()))
	{
		G_usage();
		G_fatal_error (_("Illegal east coordinate <%s>"),
		    opt3->answers[0]);
	}

	/* get grid northing start */
	if(!G_scan_northing(opt3->answers[1], &north, G_projection()))
	{
		G_usage();
		G_fatal_error (_("Illegal north coordinate <%s>"),
		    opt3->answers[1]);
	}

	/* Setup driver and check important information */
	if (R_open_driver() != 0)
		G_fatal_error (_("No graphics device selected"));

	/* Parse and select grid color */
	colorg = D_parse_color (opt1->answer, 0);	
	
	/* Parse and select border color */
	colorb = D_parse_color (opt4->answer, 0);

	D_setup(0);

	/* draw grid */
	if(!nogrid->answer)
	{
		/* Set grid color */
		D_raster_use_color (colorg);

		if (geogrid->answer)
		{
			/* initialzie proj stuff */
			init_proj(&info_in, &info_out, wgs84->answer);
			plot_geogrid(gsize, info_in, info_out, do_text, fontsize);
		} else {
			/* Do the grid plotting */
			plot_grid(size, east, north, do_text, fontsize);
		}
	}

	/* Draw border */
	if(!noborder->answer)
	{
	    if (G_projection() == PROJECTION_LL)
	  	G_warning(_("Border not yet implemented for LatLong locations: border not drawn."));
	    else
	    {
		/* Set border color */
		D_raster_use_color (colorb);
	  
		/* Do the border plotting */
		plot_border(size, east, north) ;
	    }
	}

	D_add_to_list(G_recreate_command()) ;

	R_close_driver();

	exit(EXIT_SUCCESS);
}
