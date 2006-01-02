#include "gis.h"
#include <stdio.h>

/*!
 * \brief align two regions
 *
 *  Modifies the input <b>window</b> to align to
 * <b>ref</b> region. The resolutions in <b>window</b> are set to match those
 * in <b>ref</b> and the <b>window</b> edges (north, south, east, west) are
 * modified to align with the grid of the <b>ref</b> region.
 * The <b>window</b> may be enlarged if necessary to achieve the alignment.
 * The north is rounded northward, the south southward, the east eastward and the
 * west westward. Lon-lon constraints are taken into consideration
 * to make sure that the north doesn't go above 90 degrees (for lat/lon)
 * or that the east does "wrap" past the west, etc.
 * This routine returns NULL if ok, otherwise it returns an error message
 * (do NOT free this message).
 *
 *  \param window
 *  \param ref
 *  \return char * 
 */

char *
G_align_window  (struct Cell_head *window, struct Cell_head *ref)

{
    int preserve;
    double floor(), ceil();

    window->ns_res = ref->ns_res;
    window->ew_res = ref->ew_res;
    window->zone   = ref->zone;
    window->proj   = ref->proj;

    preserve = window->proj == PROJECTION_LL && window->east == (window->west+360);
    window->south =
	G_row_to_northing (ceil(G_northing_to_row (window->south, ref)), ref);
    window->north =
	G_row_to_northing (floor(G_northing_to_row (window->north, ref)), ref);
    window->east =
	G_col_to_easting (ceil(G_easting_to_col (window->east, ref)), ref);
    window->west =
	G_col_to_easting (floor(G_easting_to_col (window->west, ref)), ref);

    if (window->proj == PROJECTION_LL)
    {
	while (window->north > 90.0)
	    window->north -= window->ns_res;
	while (window->south < -90.0)
	    window->south += window->ns_res;
	if (preserve)
	    window->east = window->west + 360;
	else
	    while (window->east - window->west > 360.0)
		window->east -= window->ew_res;
    }

    return G_adjust_Cell_head (window, 0, 0);
}
