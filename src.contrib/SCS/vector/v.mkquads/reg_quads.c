

/*
*   Creates a reg file for digit.  
*   Written by Grass Team, Fall of 88, -mh.
*/

#include	<stdio.h>
#include	"quad_structs.h"
#include	"dig_defines.h"

reg_quads (fp_reg, Q)
	FILE  *fp_reg ;
	struct  quads_description  *Q ;
{

	int  i, k ;
	int  num_v_rows, num_v_cols ;
	double  x,  y ;
	double  lon, lat ;
	double  lon_shift,  lat_shift ;

	num_v_rows = Q->num_vect_rows ;
	num_v_cols = Q->num_vect_cols ;


	lat_shift =  Q->lat_shift ;
	lon_shift =  Q->lon_shift ;


/*  write out all the vector lengths (x vectors) of the entire grid  */


	lat = Q->origin_lat ;
	for ( i = 0; i < num_v_rows; ++i)
	{
		lon = Q->origin_lon ;
		for (k = 0; k < num_v_cols; ++k)
		{
			convert_ll_to_utm( lon, lat, &x, &y, Q) ;
			write_reg_line( fp_reg, x, y) ;
			lon += lon_shift ;
		}
		lat += lat_shift ;
	}

	return (0) ;

}


write_reg_line( fp_reg, x, y )
	FILE  *fp_reg ;
	double   x, y ;
{

	fprintf( fp_reg, "  %10.2lf  %10.2lf\n", x, y) ;
}

