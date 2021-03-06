/* plot1() - Level One vector reading */

#include "gis.h"
#include "Vect.h"

extern double D_get_d_north();
extern double D_get_d_south();
extern double D_get_d_west();
extern double D_get_d_east();

extern int D_move_abs();
extern int D_cont_abs();


plot1 (name, mapset, Points)
    char *name, *mapset;
    struct line_pnts *Points;
{
    int i;
    struct Map_info Map;
    double *x, *y;

    /*fd = open_vect (name, mapset);*/
    Vect_set_open_level (1);
    if (1 > Vect_open_old (&Map, name, mapset))
	G_fatal_error ("Failed opening vector file");

    G_setup_plot (
	D_get_d_north(), D_get_d_south(), D_get_d_west(), D_get_d_east(),
	D_move_abs, D_cont_abs);


    printf ("Plotting ... "); fflush (stdout);
    while (1)
    {
        switch (Vect_read_next_line (&Map, Points))
	{
	case -1:
	    Vect_close (&Map);
	    fprintf (stderr, "\nERROR: vector file [%s] - can't read\n", name);
	    return -1;
	case -2: /* EOF */
	    printf ("Done\n");
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
