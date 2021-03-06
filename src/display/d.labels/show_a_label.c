
/*
 *
 * show_a_label.c
 *
 * if label is ok, leaves label on screen, and returns 0
 * if label is not ok, erases label, and returns 1
 *
 */

#include "gis.h"
#include "config.h"
int do_label();

int show_a_label(tmp_fname)
char *tmp_fname;
{
	int flag=1;
        struct Cell_head window ;
	struct Colors colors ;
	char window_name[64] ;
	char *label_name ;
	char *mapset ;
	FILE *infile ;
	char buff[128] ;
	int t, b, l, r ;
	int i ;

	R_open_driver();

	if (D_get_cur_wind(window_name))
		G_fatal_error("No current window") ;

	if (D_set_cur_wind(window_name))
		G_fatal_error("Current window not available") ;

/* Read in the map window associated with window */
	G_get_window(&window) ;

	if (D_check_map_window(&window))
		G_fatal_error("Setting map window") ;

	if (G_set_window(&window) == -1) 
		G_fatal_error("Current window not settable") ;

/* Determine conversion factors */
	if (D_get_screen_window(&t, &b, &l, &r))
		G_fatal_error("Getting screen window") ;

	if (D_do_conversions(&window, t, b, l, r))
		G_fatal_error("Error in calculating conversions") ;

/* Go draw the label */
        infile = fopen(tmp_fname,"r");
	flag = do_label(infile);

	R_close_driver();
	
        return(flag);
}
