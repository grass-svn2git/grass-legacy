#include <stdio.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/display.h>
#include <grass/raster.h>

static int get_text_x(void);
static int rhumbline(int);
static int move(int,int);
static int cont(int,int);

#define METERS_TO_MILES(x) ((x) * 6.213712e-04)

static int min_range[5], max_range[5];
static int which_range;
static int change_range;

int setup_plot (void)
{

/* establish the current graphics window */
    D_setup(0);

/* set D clip window */
    D_set_clip_window ( (int)D_get_d_north(),
			(int)D_get_d_south(),
			(int)D_get_d_west(),
			(int)D_get_d_east());

/* setup the G plot to use the D routines */
    G_setup_plot ( D_get_d_north(),
		   D_get_d_south(),
		   D_get_d_west(),
		   D_get_d_east(),
		   move, cont);

    /*
    G_get_ellipsoid_parameters (&a, &e2);
    G_begin_rhumbline_distance (a, e2);
    */
    R_text_size (10, 10);

    return 0;
}

int 
plot (double lon1, double lat1, double lon2, double lat2, int line_color, int text_color)
{
    int text_x, text_y;

    which_range = -1;
    change_range = 1;
    R_standard_color (line_color);
    if (lon1 != lon2)
    {
	G_shortest_way (&lon1, &lon2);
	G_begin_rhumbline_equation (lon1,lat1,lon2,lat2);
	G_plot_fx(G_rhumbline_lat_from_lon, lon1, lon2);
	/*
	text_x = get_text_x();
	if (text_x >= 0)
	    text_y = rhumbline(text_x);
	*/
    }
    else
    {
	G_plot_where_xy (lon1, (lat1+lat2)/2, &text_x, &text_y);
	G_plot_line (lon1, lat1, lon2, lat2);
    }
    R_stabilize();
/*
    distance = G_rhumbline_distance (lon1, lat1, lon2, lat2);
    sprintf (buf, "%.0lf miles\n", METERS_TO_MILES(distance));
    if (text_x >= 0)
    {
	if (text_y + 10 <= D_get_d_north())
	    text_y = D_get_d_north() - 10;
	if (text_x + 10*strlen(buf) >= D_get_d_east())
	    text_x = D_get_d_east() - 10*strlen(buf);
	R_move_abs (text_x, text_y);
	R_standard_color (text_color);
	R_text (buf);
	R_flush();
    }
*/

    return 0;
}

static int cont(int x,int y)
{
    if(D_cont_abs (x, y)) /* clipped */
    {
	change_range = 1;
    }
    else  /* keep track of left,right x for lines drawn in window */
    {
	if (change_range)
	{
	    which_range++;
	    min_range[which_range] = max_range[which_range] = x;
	    change_range = 0;
	}
	else
	{
	    if (x < min_range[which_range])
		min_range[which_range] = x;
	    else if (x > max_range[which_range])
		max_range[which_range] = x;
	}
    }

    return 0;
}

static int move(int x,int y)
{
    D_move_abs(x,y);

    return 0;
}

static int rhumbline(int x)
{
    double lon, lat;
    lon = D_d_to_u_col((double)x) ;
    lat = G_rhumbline_lat_from_lon (lon);
    return (int) D_u_to_d_row (lat);
}

static int get_text_x (void)
{
    int n;
    int len;
    int max;
    int which;

    which = -1;
    max = 0;
    for (n = 0; n <= which_range; n++)
    {
	len = max_range[n] - min_range[n];
	if (len >= max)
	{
	    max = len;
	    which = n;
	}
    }
    if (which < 0) return -1;

    return (max_range[which] + min_range[which])/2;
}
