#include "gis.h"
#include "Vect.h"
#include "dbmi.h"
#include "display.h"
#include "raster.h"
#include "symbol.h"
#include "global.h"

#define PI  3.14159265358979323846

int 
pie ( double cx, double cy, int size, double *val, int ncols, COLOR *ocolor, COLOR *colors ) 
{
    int i, j;
    double a, end_ang, ang, tot_sum, sum, step, r;
    double x, y;
    struct line_pnts *Points;

    G_debug (4, "pie(): cx = %f cy = %f", cx, cy );
    
    Points = Vect_new_line_struct ();

    /* Calc sum */
    tot_sum = 0;
    for ( i = 0; i < ncols; i++ ) tot_sum += val[i];
    
    step = PI / 180;
    r = ( D_d_to_u_col(2) - D_d_to_u_col(1) ) * size / 2; /* do it better */
    /* Draw polygon for each value */
    sum = 0; ang = 0;
    for ( i = 0; i < ncols; i++ ) {
	sum += val[i];
        end_ang = 2 * PI * sum / tot_sum;	
	Vect_reset_line ( Points );

	if (val[0] != tot_sum) /* all in one slice, don't draw line to center */
	    Vect_append_point ( Points, cx, cy, 0);

	a = ang;
	while ( 1 ) {
	   if ( a > end_ang ) a = end_ang;
	   x = cx + r * cos ( a );
	   y = cy + r * sin ( a );
	   Vect_append_point ( Points, x, y, 0);

	   if ( a == end_ang ) {
	       ang = end_ang;
	       break;
	   } else {
	       a += step;
	   }
	}

	if (val[0] != tot_sum)
	    Vect_append_point ( Points, cx, cy, 0);

	if(!colors[i].none) {
	    R_RGB_color ( colors[i].r, colors[i].g, colors[i].b );
	    G_plot_polygon ( Points->x, Points->y, Points->n_points);
	}

	R_RGB_color ( ocolor->r, ocolor->g, ocolor->b );
	for (j = 1; j < Points->n_points; j++ ) { 
	    G_plot_line(Points->x[j], Points->y[j], Points->x[j-1], Points->y[j-1]);
	}
    }

    Vect_destroy_line_struct (Points);
    
    return 0;
}


