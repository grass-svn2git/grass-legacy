

/*  This takes all the information we have accumulated so far and creates
*   the vector file.  Creates the quads line by line.
*   lat is north/south,  lon is east/west.
*   A quad is not a square box.  
*   Written by Grass Team, Fall of 88, -mh.
*/

#include	<stdio.h>
#include	"gis.h"
#include	"quad_structs.h"
#include	"dig_defines.h"

#define  WIND_DIR  "windows"

window_quads ( Q, WW)
	struct  quads_description  *Q ;
	struct  Cell_head  *WW ;           /*  work window  */
{

	int  i, k ;
	int  window_num ;
	int  rows, cols ;
	int  num_v_rows, num_v_cols ;
	double  x,  y ;
	double  x2,  y2 ;
	double  opp_x,  opp_y ;
	double  lon, lat ;
	double  lon_shift,  lat_shift ;
	double  opp_lon,  opp_lat ;
	char  buffer[128] ;

	num_v_rows = Q->num_vect_rows ;
	num_v_cols = Q->num_vect_cols ;

	rows = Q->num_rows ;
	cols = Q->num_cols ;

	lat_shift =  Q->lat_shift ;
	lon_shift =  Q->lon_shift ;


/*  write out all the vector lengths (x vectors) of the entire grid  */


	lat = Q->origin_lat ;
	for ( i = 0; i < rows; ++i)
	{
		lon = Q->origin_lon ;
		for (k = 0; k < cols; ++k)
		{
			opp_lon = lon + lon_shift ;
			opp_lat = lat + lat_shift ;

		    /*  lower left point of window  */
			convert_ll_to_utm( lon, lat, &x, &y, Q) ;
		    /*  opposite point of window  */
			convert_ll_to_utm( opp_lon, opp_lat, &opp_x, &opp_y, Q) ;
			window_num = i * cols + k + 1 ;

			write_window( window_num, x, y, opp_x, opp_y, WW ) ;

			lon = opp_lon ;
		}

		lat += lat_shift ;
	}




	return (0) ;


}



write_window( window_num, x1, y1, x2, y2, WW )
	int  window_num ;
	double  x1, y1, x2, y2 ;
	struct  Cell_head  *WW ;           /*  work window  */
{

	int  round ;
	char  name[156] ;

	roundit(&x1) ;
	roundit(&x2) ;
	roundit(&y1) ;
	roundit(&y2) ;

	WW->east = (x1 > x2) ? x1 : x2 ;
	WW->west = (x1 < x2) ? x1 : x2 ;
	WW->north = (y1 > y2) ? y1 : y2 ;
	WW->south = (y1 < y2) ? y1 : y2 ;

	sprintf( name, "quad.%d", window_num) ;

	G__put_window( WW, WIND_DIR, name) ;
}

roundit( num)
	double *num ;
{
	int  round ;

	round = *num + 0.5 ;
	*num = round ;


}

