/*  %W%  %G%  */

#include "gis.h"

Dcell(name, mapset, overlay)
	char *name ;
	char *mapset ;
	int overlay ;
{
	struct Cell_head wind ;
	struct Colors colors ;
	char window_name[64] ;
	char buff[128] ;
	int offset ;

	G_get_set_window (&wind) ;

	if (D_check_map_window(&wind))
		G_fatal_error("Setting map window") ;

	if (G_set_window(&wind) == -1) 
		G_fatal_error("Current window not settable") ;

/* Get existing map window for this graphics window, or save window */
/* cell maps wipe out a picture, so we clear info on the window too */
	if (!overlay && D_clear_window())
		G_fatal_error("Can't clear current graphics window") ;

/* Save the current map window with the graphics window */
	D_check_map_window(&wind) ;
	G_set_window (&wind);

/* Get color offset value for current window and pass to driver */
	D_offset_is(&offset) ;
	R_color_offset(offset) ;

/* Set the colors for the display */
	if (G_read_colors(name, mapset, &colors) == -1)
	{
		sprintf(buff,"Color file for [%s] not available", name) ;
		G_fatal_error(buff) ;
	}

	D_reset_colors (&colors);
	G_free_colors (&colors);

/* Go draw the cell file */
	cell_draw(name, mapset, overlay) ;

	sprintf (buff, "%s in %s", name, mapset);
	D_set_cell_name (buff) ;
}

static
cell_draw(name, mapset, overlay)
	char *name ;
	char *mapset ;
	int overlay ;
{
	char buff[128] ;
	int cellfile ;
	CELL *xarray ;
	int cur_A_row ;
	int t, b, l, r ;

/* Set up the screen, conversions, and graphics */
	D_get_screen_window(&t, &b, &l, &r) ;
	if (D_cell_draw_setup(t, b, l, r))
	{
		sprintf(buff,"Cannot use current window") ;
		G_fatal_error(buff) ;
	}

/* Make sure map is available */
	if ((cellfile = G_open_cell_old(name, mapset)) == -1) 
	{
		sprintf(buff,"Not able to open cellfile for [%s]", name);
		G_fatal_error(buff) ;
	}

/* Allocate space for cell buffer */
	xarray = G_allocate_cell_buf() ;

/* loop for array rows */
	for (cur_A_row = 0; cur_A_row != -1; )
	{
	/* Get window (array) row currently required */
		G_get_map_row(cellfile, xarray, cur_A_row) ; 
	
	/* Draw the cell row, and get the next row number */
		if (overlay)
			cur_A_row = D_overlay_cell_row(cur_A_row, xarray) ;
		else
			cur_A_row = D_draw_cell_row(cur_A_row, xarray) ;
	}
	R_flush() ;

/* Wrap up and return */
	G_close_cell(cellfile) ;
	free (xarray);
	return(0) ;
}
