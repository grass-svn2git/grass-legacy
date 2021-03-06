#include "gis.h"

main(argc, argv)
	int argc ;
	char **argv ;
{
	char frame[64] ;
	struct
	{
	    struct Option *c1;
	    struct Option *c2;
	} parm;
	int color1, color2;
	char *D_color_list();

/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	parm.c1 = G_define_option();
	parm.c1->key = "c1";
	parm.c1->description = "line color 1";
	parm.c1->type = TYPE_STRING;
	parm.c1->required = NO;
	parm.c1->options=D_color_list();
	parm.c1->answer = "black";

	parm.c2 = G_define_option();
	parm.c2->key = "c2";
	parm.c2->description = "line color 2";
	parm.c2->type = TYPE_STRING;
	parm.c2->required = NO;
	parm.c2->options=D_color_list();
	parm.c2->answer = "white";

	if (G_parser(argc,argv))
	    exit(1);

	R_open_driver();

	if (D_get_cur_wind(frame))
		G_fatal_error("No current frame") ;

	if (D_set_cur_wind(frame))
		G_fatal_error("Current frame not available") ;

	color1 = D_translate_color (parm.c1->answer);
	color2 = D_translate_color (parm.c2->answer);

	measurements(color1, color2) ;

	R_close_driver();
}
