/* plot1() - Level One vector reading */

#include <unistd.h>
#include "gis.h"
#include "Vect.h"
#include "display.h"
#include "vectpoints.h" 
#include "globals.h" 

int plot1 (char *name, char *mapset, struct line_pnts *Points)
{
    int i;
    struct Map_info Map;
    double *x, *y;

    /*fd = open_vect (name, mapset);*/
    if (1 > Vect_open_old (&Map, name, mapset))
	G_fatal_error ("Failed opening vector file");

    G_setup_plot (
	D_get_d_north(), D_get_d_south(), D_get_d_west(), D_get_d_east(),
	D_move_abs, D_cont_abs);

    while (1)
    {
        switch (Vect_read_next_line (&Map, Points))
	{
	case -1:
	    Vect_close (&Map);
	    fprintf (stderr, "\nERROR: vector file [%s] - can't read\n", name);
	    return -1;
	case -2: /* EOF */
            
	    Vect_close (&Map);
	    return  0;
	}

	x = Points->x;
	y = Points->y;

	for(i=1; i < Points->n_points; i++)
	{
	    G_plot_line(x[0], y[0], x[1], y[1]);
	    x++;
	    y++;
	}
    }
}

/*------------------------------------------------
*/

int 
plot1_warp (char *name, char *mapset, struct line_pnts *Points, double E[], double N[], int trans_order)
{
    int i;
    struct Map_info Map;
    double *x, *y;

    /*fd = open_vect (name, mapset);*/
    if (1 > Vect_open_old (&Map, name, mapset))
	G_fatal_error ("Failed opening vector file");

    G_setup_plot (
	D_get_d_north(), D_get_d_south(), D_get_d_west(), D_get_d_east(),
	D_move_abs, D_cont_abs);

    while (1)
    {
        switch (Vect_read_next_line (&Map, Points))
	{
	case -1:
	    Vect_close (&Map);
	    fprintf (stderr, "\nERROR: vector file [%s] - can't read\n", name);
	    return -1;
	case -2: /* EOF */
            
	    Vect_close (&Map);
	    return  0;
	}

	x = Points->x;
	y = Points->y;

	CRS_georef(x[0], y[0], &x[0], &y[0], E, N, trans_order);

	for(i=1; i < Points->n_points; i++)
	{
	    CRS_georef(x[1], y[1], &x[1], &y[1], E, N, trans_order);
	    G_plot_line(x[0], y[0], x[1], y[1]);
	    x++;
	    y++;
	}
    }
}


