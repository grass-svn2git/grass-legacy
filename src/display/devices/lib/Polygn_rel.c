#include "driver.h"
Polygon_rel(xarray, yarray, number)
	int *xarray, *yarray ;
	int number ;
{
	int incr ;

	xarray[0] += cur_x ;
	yarray[0] += cur_y ;

	for (incr=1; incr<number; incr++)
	{
		xarray[incr] += xarray[incr-1] ;
		yarray[incr] += yarray[incr-1] ;
	}

	incr-- ;
	cur_x = xarray[incr] ;
	cur_y = yarray[incr] ;

	Polygon_abs(xarray, yarray, number) ;
}
