#ifndef lint
static char *SCCSid = "@(#)bilinear.c	v1.5 - 27 Jun 1995 	-emes-";
#endif

/*
 * Name
 *  bilinear.c -- use bilinear interpolation for given row, col
 *
 * Description
 *  bilinear interpolation for the given row, column indices.
 *  If the given row or column is outside the bounds of the input map,
 *  the point in the output map is set to NULL.
 *  If any of the 4 surrounding points to be used in the interpolation
 *  is NULL it is filled with is neighbor value
 *
 *  See: Press, W.H. et al. (1992), Numerical recipes in C.
 */

#include <grass/gis.h>
#include <math.h>
#include "r.proj.h"

void p_bilinear(
	struct cache *ibuffer,		/* input buffer			 */
	void *obufptr,			/* ptr in output buffer          */
	int cell_type,			/* raster map type of obufptr    */
	double *col_idx,		/* column index		 */
	double *row_idx,		/* row index			 */
	struct Cell_head *cellhd	/* information of output map	 */
)
{
	int	row0,			/* lower row index for interp	 */
		row1,			/* upper row index for interp	 */
		col0,			/* lower column index for interp */
		col1;			/* upper column index for interp */
	FCELL	t, u,			/* intermediate slope		 */
		tu,			/* t * u	 		 */
		result;			/* result of interpolation	 */
	FCELL *c00, *c01, *c10, *c11;

	/* cut indices to integer */
	row0 = (int) floor(*row_idx);
	col0 = (int) floor(*col_idx);
	row1 = row0 + 1;
	col1 = col0 + 1;

	/* check for out of bounds - if out of bounds set NULL value and return */
	if (row0 < 0 || row1 >= cellhd->rows ||
	    col0 < 0 || col1 >= cellhd->cols)
	{
		G_set_null_value(obufptr, 1, cell_type);
		return;
	}

	c00 = CPTR(ibuffer, row0, col0);
	c01 = CPTR(ibuffer, row0, col1);
	c10 = CPTR(ibuffer, row1, col0);
	c11 = CPTR(ibuffer, row1, col1);

	/* check for NULL values */
	if (G_is_f_null_value(c00) ||
	    G_is_f_null_value(c01) ||
	    G_is_f_null_value(c10) ||
	    G_is_f_null_value(c11))
	{
		G_set_null_value(obufptr, 1, cell_type);
		return;
	}

	/* do the interpolation	 */
	t = *col_idx - col0;
	u = *row_idx - row0;
	tu = t * u;

	result = G_interp_bilinear(t, u, *c00, *c01, *c10, *c11);

	G_set_raster_value_f(obufptr, result, cell_type);
}
