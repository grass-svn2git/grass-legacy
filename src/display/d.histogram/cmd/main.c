
/* main.c 
 *
 * function defined:
 *
 * main for program Dhistogram 
 *
 *
 * PURPOSE: To draw a bar-chart or a pie-chart representing the
 * histogram statistics of a cell-file
 *
 * The color option specifies the color for the labels, tic-marks,
 * and borders of the chart.  Use one of the 16 standard GRASS color
 * names.  The type option is either "area" or "count," the default
 * is "count" The style option is either "pie" or "bar," the default
 * is "bar"  
 *
 * Dave Johnson
 * DBA Systems, Inc.
 * 10560 Arrowhead Drive
 * Fairfax, Virginia 22030
 */
/******************************************************************************
 * NOTE (shapiro):
 *  This program can NOT handle area information.
 *  Areas (as output by the r.stats command) are doubles.
 *  This program was written assuming areas are integers.
 *
 *  The area option has been #ifdef'ed out of the code until someone
 *   upgrades both the get_stats() and the pie() and bar() routines
 *   as well as the struct stat_list (defined in dhist.h).
 *****************************************************************************/

#include "gis.h"
#define MAIN
#include "options.h"
#include "dhist.h"

main(argc, argv)
	int argc ;
	char **argv ;
{
	int text_height;
	int text_width;
	char buff[256] ;
	char window_name[64] ;
	char *mapset ;
	char *D_color_list();
	struct Categories cats ;
	struct Range range ;
	struct Colors pcolors ;
	char title[512];
	int tt,tb,tl,tr;
	int t,b,l,r;
	int quiet;
	struct Option *opt1 ;
	struct Option *opt2 ;
	struct Option *opt3 ;
	struct Option *opt4 ;
	struct Flag *flag1 ;
	struct Flag *flag2 ;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	opt1             = G_define_option() ;
	opt1->key        = "map" ;
	opt1->description= "Raster map for which histogram will be displayed" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = YES ;
	opt1->gisprompt  = "old,cell,raster" ;

	opt2             = G_define_option() ;
	opt2->key        = "color" ;
	opt2->description= "Color for legend and title";
	opt2->type       = TYPE_STRING ;
	opt2->required   = NO ;
	opt2->answer     = "white" ;
	opt2->options    = D_color_list();

#ifdef CAN_DO_AREAS
	opt3             = G_define_option() ;
	opt3->key        = "type" ;
	opt3->description= "Indicate if cell counts or map areas should be displayed" ;
	opt3->type       = TYPE_STRING ;
	opt3->required   = NO ;
	opt3->answer     = "count" ;
	opt3->options    = "count,area";
#endif

	opt4             = G_define_option() ;
	opt4->key        = "style" ;
	opt4->description= "Indicate if a pie or bar chart is desired" ;
	opt4->type       = TYPE_STRING ;
	opt4->required   = NO ;
	opt4->answer     = "bar" ;

	flag1            = G_define_flag() ;
	flag1->key       = 'z' ;
	flag1->description= "Display zero-data information" ;

	flag2            = G_define_flag() ;
	flag2->key       = 'q' ;
	flag2->description= "Gather the histogram quietly";

	if (G_parser(argc, argv))
		exit(1) ;

	map_name = opt1->answer ;

	color = D_translate_color(opt2->answer) ;

	type = COUNT;
#ifdef CAN_DO_AREAS
	if (strcmp(opt3->answer,"count") == 0)
		   type = COUNT;
	else
		   type = AREA;
#endif

	if (strcmp(opt4->answer,"bar") == 0)
	   style = BAR;
	else
	   style = PIE;

	nodata = flag1->answer ? YES : NO ;
	quiet  = flag2->answer ? YES : NO ;

	/* Make sure map is available */
	mapset = G_find_cell2 (map_name, "") ;
	if (mapset == NULL)
	{
		sprintf(buff,"Raster map [%s] not available", map_name);
		G_fatal_error(buff) ;
	}

	if (G_read_colors(map_name, mapset, &pcolors) == -1)
	{
		sprintf(buff,"color file for [%s] not available", map_name) ;
		G_fatal_error(buff) ;
	}

	if (G_read_cats(map_name, mapset, &cats) == -1)
	{
		sprintf(buff,"Category file for [%s] not available", map_name) ;
		G_fatal_error(buff) ;
	}

	if (G_read_range(map_name, mapset, &range) == -1)
	{
		sprintf(buff,"Range information for [%s] not available", map_name) ;
		G_fatal_error(buff) ;
	}

	/* get the distribution statistics */

	get_stats(map_name,mapset,&dist_stats, quiet);

	/* set up the graphics driver and initialize its color-table */

	R_open_driver();

	D_setup(0);

	D_set_colors(&pcolors);

	/* draw a title for */

	sprintf(title,"%s in mapset %s",map_name,mapset);
	D_get_screen_window(&t, &b, &l, &r);
	text_height = (b-t)*0.05;
	text_width = (r-l)*0.05*0.50;
	R_text_size(text_width,text_height);
	R_get_text_box(title,&tt,&tb,&tl,&tr);
	R_move_abs((int)(l+(r-l)/2-(tr-tl)/2),(int)(t+(b-t)*0.07));
	R_standard_color(color);
	R_text(title);

	/* plot the distributrion statistics */

	if (style == PIE)
		pie(&dist_stats, &pcolors);
	else
		bar(&dist_stats, &pcolors);

	R_flush();
	D_add_to_list(G_recreate_command()) ;
	R_close_driver();
	exit(0);
}
