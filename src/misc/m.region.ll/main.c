#include "gis.h"
main(argc,argv) char *argv[];
{
    struct Option *spheroid;
    double lat_nw, lon_nw;
    double lat_ne, lon_ne;
    double lat_sw, lon_sw;
    double lat_se, lon_se;
    double secs;
    double a,e;
    struct Cell_head window;
    char buf[100];
    char *spheroid_list();

    G_gisinit (argv[0]);
    if (G_projection() != PROJECTION_UTM)
    {
	fprintf (stderr, "%s - must be in a UTM database\n", G_program_name());
	exit(1);
    }

    spheroid = G_define_option();
    spheroid->key = "spheroid";
    spheroid->type = TYPE_STRING;
    spheroid->required = YES;
    spheroid->options = spheroid_list();

    if (G_parser(argc,argv))
	exit(1);
    if(!G_get_ellipsoid_by_name(spheroid->answer, &a, &e))
    {
        fprintf (stderr, "%s=%s - unknown spheroid\n",
                spheroid->key, spheroid->answer);
        G_usage();
        exit(1);
    }

    G_get_window (&window);
    printf ("\nREGION %.2lf N  %.2lf E       ", window.north, window.east);
    printf ("ZONE %d\n", window.zone);
    printf ("       %.2lf S  %.2lf W\n", window.south, window.west);

    CC_u2ll_spheroid_parameters (a,e);
    CC_u2ll_zone (window.zone) ;
    CC_u2ll_north (window.north);
    CC_u2ll (window.west, &lat_nw, &lon_nw);
    CC_u2ll (window.east, &lat_ne, &lon_ne);
    CC_u2ll_north (window.south);
    CC_u2ll (window.west, &lat_sw, &lon_sw);
    CC_u2ll (window.east, &lat_se, &lon_se);

    lat_nw /= 3600.0;
    lat_ne /= 3600.0;
    lat_sw /= 3600.0;
    lat_se /= 3600.0;

    lon_nw /= -3600.0;
    lon_ne /= -3600.0;
    lon_sw /= -3600.0;
    lon_se /= -3600.0;

    printf("\n\n");

    G_format_northing (lat_nw, buf, PROJECTION_LL);
    printf ("%-30s",buf);
    G_format_northing (lat_ne, buf, PROJECTION_LL);
    printf ("%s\n",buf);

    G_format_easting (lon_nw, buf, PROJECTION_LL);
    printf ("%-30s",buf);
    G_format_easting (lon_ne, buf, PROJECTION_LL);
    printf ("%s\n",buf);

    printf ("\n\n\n\n\n");


    G_format_northing (lat_sw, buf, PROJECTION_LL);
    printf ("%-30s",buf);
    G_format_northing (lat_se, buf, PROJECTION_LL);
    printf ("%s\n",buf);

    G_format_easting (lon_sw, buf, PROJECTION_LL);
    printf ("%-30s",buf);
    G_format_easting (lon_se, buf, PROJECTION_LL);
    printf ("%s\n",buf);

    printf("\n\n");

    if (secs = lon_nw - lon_ne)
    {
	if (secs < 0) secs = - secs;
	printf ("at northern edge 1 arc second longitude = %lf meters\n",
	    window.ew_res * window.cols / secs);
    }
    if (secs = lon_sw - lon_se)
    {
	if (secs < 0) secs = - secs;
	printf ("at southern edge 1 arc second longitude = %lf meters\n",
	    window.ew_res * window.cols / secs);
    }
    if (secs = lat_nw - lat_sw)
    {
	if (secs < 0) secs = - secs;
	printf ("at western edge 1 arc second latitude = %lf meters\n",
	    window.ns_res * window.rows / secs);
    }
    if (secs = lat_ne - lat_se)
    {
	if (secs < 0) secs = - secs;
	printf ("at eastern edge 1 arc second latitude = %lf meters\n",
	    window.ns_res * window.rows / secs);
    }
    exit(0);
}

usage (me) char *me;
{
    char *CC_spheroid_name();
    int i;

    fprintf (stderr, "usage: %s spheroid\n", me);
    fprintf (stderr, "where spheroid is one of\n");
    for (i = 0; CC_spheroid_name(i); i++)
    {
	if (i%3 == 0) fprintf (stderr, "  ");
	fprintf (stderr, "%-20s ", CC_spheroid_name(i));
	if (i%3 == 2) fprintf (stderr, "\n");
    }
    fprintf (stderr, "\n");
    exit (1);
}
