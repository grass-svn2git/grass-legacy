/* @(#)find_area.c	2.1   6/26/87 */
#include "ply_to_cll.h"

find_area(xarray, yarray, num_verticies, xy, num_points)
	double xarray[], yarray[] ;
	int num_verticies ;
	struct element xy[] ;
	int *num_points ;
{
	int compare() ;
	int node ;
	int row ;
	int incr ;
	register double A, B ;
	double delta_x, delta_y ;
	int first_row, last_row ;

	/* adjust Y grid coordinates to Y array coordinates */
	yadjust(yarray, num_verticies) ;

	*num_points = 0 ;

	for (node=0; node<num_verticies; node++)
	{
#ifdef DEBUG
	fprintf(stderr,"(x,y) %.2f:%.2f %.2f:%.2f  ",
		xarray[node], yarray[node], xarray[node+1], yarray[node+1]) ;
#endif DEBUG
	/*  generate equation  */
		delta_y = yarray[node+1] - yarray[node] ;
		delta_x = xarray[node+1] - xarray[node] ;
		if (delta_y == 0.0) ;
		else
		{
			B = delta_x / delta_y ;
			A = xarray[node] - B * yarray[node]  ;
		}
#ifdef DEBUG
	fprintf(stderr,"A = %f  B = %f\n", A, B) ;
#endif DEBUG

	/*  determine first and last row involved */
		if (yarray[node+1] > yarray[node])
		{
			if (yarray[node] > 0.0)
				first_row = yarray[node] + 1. ;
			else
				first_row = yarray[node] ;
			if (yarray[node+1] > 0.0)
				last_row = yarray[node+1] ;
			else
				last_row = yarray[node+1] - 1. ;
		}
		else if (yarray[node+1] < yarray[node])
		{
			if (yarray[node+1] > 0.0)
				first_row = yarray[node+1] + 1. ;
			else
				first_row = yarray[node+1] ;
			if (yarray[node] > 0.0)
				last_row = yarray[node] ;
			else
				last_row = yarray[node] - 1. ;
		}

#ifdef DEBUG
fprintf(stderr,"first: %6d  last: %6d\n", first_row, last_row) ;
#endif DEBUG

		if (first_row > last_row)
			continue ;

		if (delta_y == 0.0)
			continue ;

		for (row=first_row; row<=last_row; row++)
		{
			xy[*num_points].row = row ;
			xy[*num_points].col = A + B * row ;
#ifdef DEBUG
fprintf(stderr,"%2d %2d %6.2f\n",
	*num_points, xy[*num_points].row, xy[*num_points].col) ;
#endif DEBUG
				(*num_points)++ ;
		}
	}

	qsort(xy, *num_points, sizeof(struct element), compare) ;

#ifdef DEBUG
	fprintf(stderr,"\n") ;
	for(row=0; row<*num_points; row++)
		fprintf(stderr,"%2d %2d %6.2f\n", row, xy[row].row, xy[row].col) ;
#endif DEBUG
}

compare(element1, element2)
	struct element *element1, *element2 ;
{
	if (element1->row < element2->row)
		return(-1) ;
	if (element1->row > element2->row)
		return(1) ;
	if (element1->col < element2->col)
		return(-1) ;
	if (element1->col > element2->col)
		return(1) ;
	return(0) ;
}
