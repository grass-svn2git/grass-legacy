#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "display.h"
#include "raster.h"
#include "local_proto.h"

int main (int argc, char *argv[])
{
    int line_color;
    int text_color;
    int use_mouse;
    double lon1,lat1,lon2,lat2;
    char msg[100];
    char *deftcolor;
	struct GModule *module;
    struct
    {
	struct Option *lcolor, *tcolor, *coor;
    } parm;

    G_gisinit (argv[0]);

	module = G_define_module();
	module->description =
		"Displays a geodesic line, tracing the shortest distance "
		"between two geographic points along a great circle, in "
		"a longitude/latitude data set.";

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

    if (G_parser(argc, argv))
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
	    G_usage();
	    G_fatal_error ("%s - illegal longitude", parm.coor->answers[0]);
	}
        if (!G_scan_northing (parm.coor->answers[1], &lat1, G_projection())) 
	{
	    G_usage();
	    G_fatal_error ("%s - illegal longitude", parm.coor->answers[1]);
	}
        if (!G_scan_easting (parm.coor->answers[2], &lon2, G_projection())) 
	{
	    G_usage();
	    G_fatal_error ("%s - illegal longitude", parm.coor->answers[2]);
	}
        if (!G_scan_northing (parm.coor->answers[3], &lat2, G_projection())) 
	{
	    G_usage();
	    G_fatal_error ("%s - illegal longitude", parm.coor->answers[3]);
	}
	use_mouse = 0;
    }

    if (R_open_driver() != 0)
	    G_fatal_error ("No graphics device selected");

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

