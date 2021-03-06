#ifndef lint 
	static char *SCCSid= "@(#)nearest.c	v1.3 - 27 Jun 1995 	-emes-";
#endif

/*
 * 	nearest.c - returns the nearest neighbor to a given 
 *		    x,y position
 */ 

#include <gis.h>


void p_nearest(ibuffer, obufptr, cell_type, col_idx, row_idx, cellhd)
FCELL	**ibuffer;		/* input matrix			 */
void	*obufptr;		/* ptr in output buffer		 */
int	cell_type;		/* raster map type of obufptr	 */
double 	*col_idx,		/* column index in input matrix  */
	*row_idx;		/* row index in input matrix     */
struct	Cell_head *cellhd; 	/* cell header of input layer	 */                              
{
 int	row,col;		/* row/col of nearest neighbor	 */



   /* cut indices to integer */
	row = (int)(*row_idx + 0.5);
	col = (int)(*col_idx + 0.5);


   /* check for out of bounds - if out of bounds set NULL value	 */
	if (row < 0 || row >= cellhd->rows || 
	    col < 0 || col >= cellhd->cols ||
	    G_is_f_null_value(&ibuffer[row][col])){
		G_set_null_value(obufptr, 1, cell_type);	    
		return;
	}


	switch(cell_type){
		case CELL_TYPE:
			G_set_raster_value_c(obufptr, (CELL)  ibuffer[row][col], cell_type);
			break;
		case FCELL_TYPE:
			G_set_raster_value_f(obufptr, (FCELL) ibuffer[row][col], cell_type);			
			break;
		case DCELL_TYPE:
			G_set_raster_value_d(obufptr, (DCELL) ibuffer[row][col], cell_type);	
			break;
	}
	
	return;
}
