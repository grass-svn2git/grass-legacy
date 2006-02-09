/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Radim Blazek
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <grass/Vect.h>

/*!
 \fn int Vect_tin_get_z ( struct Map_info *Map,
		   double tx, double ty,
		   double *tz, double *angle, double *slope)
 \brief calculates z coordinate for point from TIN
 \return 1 on success,
            0 point is not in area,
           -1 area has not 4 points or has island
 \param Map_info structure
*/
int 
Vect_tin_get_z (
		   struct Map_info *Map,
		   double tx, double ty,
		   double *tz, double *angle, double *slope)
{
  int    i, area, n_points;
  struct Plus_head *Plus;
  P_AREA *Area;
  static struct line_pnts *Points;
  static int first_time = 1;
  double *x, *y, *z;
  double vx1, vx2, vy1, vy2, vz1, vz2;
  double a, b, c, d;
  
  /* TODO angle, slope */
  
  Plus = &(Map->plus);
  if (first_time == 1) {
      Points = Vect_new_line_struct ();	
      first_time = 0;
  }

  area = Vect_find_area ( Map, tx, ty );
  G_debug (3, "area = %d", area ); 
  if ( area == 0 ) return 0;
  
  Area = Plus->Area[area];
  if ( Area->n_isles > 0 ) return -1;
  
  Vect_get_area_points (Map, area, Points);
  n_points = Points->n_points;
  if ( n_points != 4 ) return -1;

  x = Points->x;
  y = Points->y;
  z = Points->z;
  for (i = 0; i < 3; i++) {
      G_debug(3, "%d %f %f %f", i, x[i], y[i], z[i] ); 
  }

  vx1 = x[1] - x[0]; 
  vy1 = y[1] - y[0]; 
  vz1 = z[1] - z[0]; 
  vx2 = x[2] - x[0]; 
  vy2 = y[2] - y[0]; 
  vz2 = z[2] - z[0]; 

  a = vy1*vz2 - vy2*vz1;
  b = vz1*vx2 - vz2*vx1;
  c = vx1*vy2 - vx2*vy1;
  d = -a*x[0] - b*y[0] - c*z[0];
 
  /* OK ? */ 
  *tz = -(d + a*tx + b*ty) / c;
  G_debug(3, "z = %f", *tz ); 
  
  return 1;
}


