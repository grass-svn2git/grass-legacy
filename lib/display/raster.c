/* routines used by programs such as Dcell, display, combine, and weight
 * for generating raster images (for 1-byte, i.e. not super-cell, data)
 *
 * D_cell_draw_setup(t, b, l, r)
 *    int t, b, l, r    (pixle extents of display window)
 *                      (obtainable via D_get_screen_window(&t, &b, &l, &r)
 *   Sets up the environment for D_draw_cell
 *
 * D_draw_cell(A_row, xarray, colors)
 *    int A_row ;  
 *    CELL *xarray ;
 *   - determinew which pixle row gets the data
 *   - resamples the data to create a pixle array
 *   - determines best way to draw the array
 *      a - for single cat array, a move and a draw
 *      b - otherwise, a call to D_raster()
 *   - returns  -1 on error or end of picture
 *         or array row number needed for next pixle row.
 *
 *   presumes the map is drawn from north to south
 *
 * NOTE: D_cell_draw() must be preceded by a call to D_set_colors()
 *       with the same Colors structure
 *
 * ALSO: if overlay mode is desired, then call D_set_overlay_mode(1)
 *       first.
 */

#include <stdio.h>
#include <stdlib.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>

static int *D_to_A_tab = NULL;
static int D_x_beg, D_y_beg, D_x_end, D_y_end ;
static int cur_D_row ;
static void *raster = NULL ;
static int draw_cell(int,void *,struct Colors *,RASTER_MAP_TYPE);

int D_draw_raster(
    int A_row,
    void *array,
    struct Colors *colors,
    RASTER_MAP_TYPE data_type)
{
    return draw_cell(A_row, array, colors, data_type);
}

int D_draw_d_raster(
    int A_row,
    DCELL *darray,
    struct Colors *colors)
{
    return draw_cell(A_row, (void *) darray, colors, DCELL_TYPE);
}

int D_draw_f_raster(
    int A_row,
    FCELL *farray,
    struct Colors *colors)
{
    return draw_cell(A_row, (void *) farray, colors, FCELL_TYPE);
}

int D_draw_c_raster(
    int A_row,
    CELL *carray,
    struct Colors *colors)
{
    return draw_cell(A_row, (void *) carray, colors, CELL_TYPE);
}


/*!
 * \brief render a raster row
 *
 * The <b>row</b> gives the map array row. The <b>raster</b>
 * array provides the categories for each raster value in that row. The
 * <b>colors</b> structure must be the same as the one passed to
 * <i>D_set_colors.</i>
 * This routine is called consecutively with the information necessary to draw a
 * raster image from north to south. No rows can be skipped. All screen pixel
 * rows which represent the current map array row are rendered. The routine
 * returns the map array row which is needed to draw the next screen pixel row.
 *
 *  \param row
 *  \param raster
 *  \param colors
 *  \return int
 */

int D_draw_cell(
    int A_row,
    CELL *carray,
    struct Colors *colors)
{
    return draw_cell(A_row, (void *) carray, colors, CELL_TYPE);
}

static int draw_cell(
    int A_row,
    void *array,
    struct Colors *colors,
    RASTER_MAP_TYPE data_type)
{
    int D_row ;
    int repeat ;
    char send_raster ;
    int cur_A_row ;

/* Allocate memory for raster */
    if(!raster) 
	raster = G_malloc((D_x_end - D_x_beg) * G_raster_size(data_type)) ;

/* If picture is done, return -1 */
    if (cur_D_row >= D_y_end)
        return(-1) ;

/* Get window (array) row currently required */
    D_row = cur_D_row ;
    cur_A_row = (int)D_d_to_a_row(cur_D_row + 0.5) ;

/* If we need a row further down the array, return that row number */
    if (cur_A_row > A_row)
        return (cur_A_row) ;

/* Find out how many screen lines the current A_row gets repeated */
    repeat = 1 ;
    for (cur_D_row++ ; cur_D_row < D_y_end; cur_D_row++)
    {
        if (A_row == (cur_A_row = (int)D_d_to_a_row(cur_D_row + 0.5)))
            repeat++ ;
        else
            break ;
    }

    /* Make the screen raster */
    {
        register int D_col ;
	void *rasptr, *arr_ptr;

	rasptr = raster;

        for (D_col = D_x_beg; D_col < D_x_end; D_col++)
	{
	    /* copy array[[D_to_A_tab[D_col]] to *raster, advance raster by 1 */

	    arr_ptr = G_incr_void_ptr(array, D_to_A_tab[D_col] * G_raster_size(data_type));
	    G_raster_cpy(rasptr, arr_ptr, 1, data_type);
	    rasptr = G_incr_void_ptr(rasptr, G_raster_size(data_type));
        }
    }

    /* Check to see if raster contains one category */
    {
        register int D_col ;
	void *rasptr, *first_val_ptr;

	first_val_ptr = (void *) G_malloc(G_raster_size(data_type));
	rasptr = raster;
	G_raster_cpy(first_val_ptr, rasptr, 1, data_type);

        send_raster = 0 ;
        for (D_col = D_x_beg; D_col < D_x_end; D_col++)
        {
            if (G_raster_cmp(first_val_ptr, rasptr, data_type) != 0)
            {
		send_raster = 1 ;
		break ;
            }
	    rasptr = G_incr_void_ptr(rasptr, G_raster_size(data_type));
        }
	if (R_transparency(-1) > 0.0)
		send_raster = 1;
    }

    /* Send the raster */
    if (send_raster)
    {
        R_move_abs(D_x_beg, D_row) ;
	D_raster_of_type(raster, D_x_end - D_x_beg, repeat, colors, data_type) ;
    }
    else
    {
	int draw;
	/* this really should be part of D_raster(); */
	extern int D__overlay_mode;
	/* color with the color for first raster value */
	D_color_of_type(raster, colors, data_type);
	draw = !D__overlay_mode || !G_is_null_value(raster, data_type);
	R_move_abs(D_x_beg, D_row) ;
	if (draw)
	    R_box_rel(D_x_end - D_x_beg, repeat);
    }

/* If picture is done, return -1 */
    if (cur_D_row >= D_y_end)
        return(-1) ;

/* Return the array row of the next row needed */
    return (cur_A_row) ;
}


/*!
 * \brief prepare for raster graphic
 *
 * The raster display subsystem establishes
 * conversion parameters based on the screen extent defined by <b>top,
 * bottom, left</b>, and <b>right</b>, all of which are obtainable from
 * <i>D_get_screen_window for the current frame.</i>
 *
 *  \param top
 *  \param bottom
 *  \param left
 *  \param right
 *  \return int
 */

int D_cell_draw_setup(int t,int b,int l,int r)
{
    int D_col ;
    struct Cell_head window ;

    if (G_get_set_window(&window) == -1) 
        G_fatal_error("Current window not available") ;
    if (D_do_conversions(&window, t, b, l, r))
        G_fatal_error("Error in calculating conversions") ;
/* Set up the screen for drawing map */
    D_x_beg = (int)D_get_d_west() ;
    D_x_end = (int)D_get_d_east() ;
    D_y_beg = (int)D_get_d_north() ;
    D_y_end = (int)D_get_d_south() ;
    cur_D_row = D_y_beg ;

    if (D_to_A_tab)
        free (D_to_A_tab) ;

    D_to_A_tab = (int *)G_calloc(D_x_end, sizeof(int)) ;

/* construct D_to_A_tab for converting x screen Dots to x data Array values */
    for (D_col = D_x_beg; D_col < D_x_end; D_col++)
        D_to_A_tab[D_col] = (int)(D_d_to_a_col(D_col + 0.5)) ;

    if (raster)
    {
        free(raster) ;
        raster = NULL ;
    }
    return(0) ;
}

