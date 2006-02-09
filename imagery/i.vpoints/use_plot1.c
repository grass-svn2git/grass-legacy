#include <unistd.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include "vectpoints.h" 

#define OVERLAP .75

int 
use_plot1 (char *name, char *mapset)
{
    char buf[128];
    double N,S,E,W;
    double G_window_percentage_overlap();
    struct Cell_head window;
    double x;
    struct Map_info Map;
char msg[100];
/*
    fprintf (stderr, "\nVector file [%s]\n", name);
*/
    /*fd = open_vect (name, mapset);*/

    Vect_set_open_level (1);
    if (0 > Vect_open_old (&Map, name, mapset))
    {
	sprintf (buf, "Cannot open vector file %s", name);
	G_fatal_error (buf);
    }
/*
fprintf(fd,"date = %s\n",Map.head.date);
fprintf(fd,"your_name = %s\n",Map.head.your_name);
fprintf(fd,"W = %lf E = %lf\n",Map.head.W,Map.head.E);
fprintf(fd,"N = %lf S = %lf\n",Map.head.N,Map.head.S);
fprintf(fd,"SCALE = %ld\n",Map.head.orig_scale);
*/
    Vect__get_window (&Map, &N, &S, &E, &W);
/*  Vect_print_header(&Map); */
    Vect_close (&Map);

    G_get_set_window (&window);

/*
sprintf(msg, "s = %lf n = %lf\n",window.south,window.north);
debug(msg);
sprintf(msg, "w = %lf e = %lf\n",window.west,window.east);
debug(msg);
sprintf(msg, "ns = %lf ew = %lf\n",window.ns_res,window.ew_res);
debug(msg);
sprintf(msg, "rows = %d cols = %d\n",window.rows, window.cols);
debug(msg);
*/

    G_format_northing (N, buf, window.proj);
    G_format_northing (S, buf, window.proj);
    G_format_easting (E, buf, window.proj);
    G_format_easting (W, buf, window.proj);

/*
    fprintf (stderr, "\n");
    G_format_northing (N, buf, window.proj);
    fprintf (stderr, " North: %s\n", buf);

    G_format_northing (S, buf, window.proj);
    fprintf (stderr, " South: %s\n", buf);

    G_format_easting (E, buf, window.proj);
    fprintf (stderr, " East:  %s\n", buf);

    G_format_easting (W, buf, window.proj);
    fprintf (stderr, " West:  %s\n", buf);
    fprintf (stderr, "\n");
*/
    x =  G_window_percentage_overlap(&window, N, S, E, W);
    /*
    fprintf (stderr, "%.1lf%% overlap\n", x * 100.0);
    */
/*
fprintf(fd,"x = %lf OVERLAP = .75",x);
pclose(fd);
*/
    return (x > OVERLAP);
}

