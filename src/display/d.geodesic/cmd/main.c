#include "gis.h"

main(argc, argv) 
    int argc;
    char *argv[];
{
    int line_color;
    int text_color;
    int use_mouse;
    double lon1,lat1,lon2,lat2;
    char msg[100];
    char *deftcolor;
    char *D_color_list();
    struct
    {
	struct Option *lcolor, *tcolor, *coor;
    } parm;

    G_gisinit (argv[0]);

    parm.coor = G_define_option() ;
    parm.coor->key        = "coor" ;
    parm.coor->key_desc   = "lon1,lat1,lon2,lat2";
    parm.coor->type       = TYPE_STRING ;
    parm.coor->required   = NO ;
    parm.coor->description= "Starting and ending coordinates" ;

    parm.lcolor = G_define_option() ;
    parm.lcolor->key        = "lcolor" ;
    parm.lcolor->type       = TYPE_STRING ;
    parm.lcolor->required   = NO ;
    parm.lcolor->description= "Line color" ;
    parm.lcolor->options    = D_color_list();
    parm.lcolor->answer     = "white";

    parm.tcolor = G_define_option() ;
    parm.tcolor->key        = "tcolor" ;
    parm.tcolor->type       = TYPE_STRING ;
    parm.tcolor->required   = NO ;
    parm.tcolor->description= "Text color" ;
    parm.tcolor->options    = D_color_list();

    if (argc > 1 && G_parser(argc, argv))
        exit(-1);

    if (G_projection() != PROJECTION_LL)
    {
	sprintf (msg, "%s: database is not a %s\n",
		argv[0], G__projection_name(PROJECTION_LL));
	G_fatal_error (msg);
	exit(1);
    }

    use_mouse = 1;
    if (parm.coor->answer)
    {
        if (!G_scan_easting (parm.coor->answers[0], &lon1, G_projection())) 
	{
	    fprintf (stderr, "%s - illegal longitude\n", parm.coor->answers[0]);
	    G_usage();
            exit(-1);
	}
        if (!G_scan_northing (parm.coor->answers[1], &lat1, G_projection())) 
	{
	    fprintf (stderr, "%s - illegal longitude\n", parm.coor->answers[1]);
	    G_usage();
            exit(-1);
	}
        if (!G_scan_easting (parm.coor->answers[2], &lon2, G_projection())) 
	{
	    fprintf (stderr, "%s - illegal longitude\n", parm.coor->answers[2]);
	    G_usage();
            exit(-1);
	}
        if (!G_scan_northing (parm.coor->answers[3], &lat2, G_projection())) 
	{
	    fprintf (stderr, "%s - illegal longitude\n", parm.coor->answers[3]);
	    G_usage();
            exit(-1);
	}
	use_mouse = 0;
    }

    R_open_driver();

    line_color = D_translate_color (parm.lcolor->answer);
    if (!line_color)
	line_color = D_translate_color (parm.lcolor->answer = "white");

    if(strcmp (parm.lcolor->answer, "white") == 0)
	deftcolor = "red";
    else
	deftcolor = "white";

    if (parm.tcolor->answer == NULL)
	parm.tcolor->answer = deftcolor;
    text_color = D_translate_color (parm.tcolor->answer);
    if (!text_color)
	text_color = D_translate_color (deftcolor);

    setup_plot();
    if (use_mouse)
	mouse (line_color, text_color);
    else
	plot (lon1, lat1, lon2, lat2, line_color, text_color);

    if (!use_mouse)
		D_add_to_list(G_recreate_command()) ;

    R_close_driver();
    exit(0);
}

