/*
 *   d.colormode
 *
 *
 */

#define USAGE1	"fixed/float"
#define FLOAT	1
#define FIXED	2

#include "gis.h"


main(argc, argv)
int argc ;
char **argv ;
{
	struct Option *option;
	int mode, stat ;

	option = G_define_option() ;
	option->key        = "mode" ;
	option->type       = TYPE_STRING ;
	option->required   = YES;
	option->description="The type of color look-up table (fixed or float)";
	option->options    ="fixed,float";



	/* Parse command line */
	if (G_parser(argc, argv))
		exit(-1);

	if (! strcmp(option->answer,"float"))
		mode = FLOAT ;
	else if (! strcmp(option->answer,"fixed"))
		mode = FIXED ;
	else
	{
		G_usage();
		exit(-1) ;
	}

	R_open_driver();

	stat = 1;
	switch (mode)
	{
	case FLOAT:
		if (stat = R_color_table_float())
			printf("Sorry, floating color table not available on this device\n") ;
		break ;
	case FIXED:
		if (stat = R_color_table_fixed())
			printf("Sorry, fixed color table not available on this device\n") ;
		break ;
	default:
		break ;
	}

	D_add_to_list(G_recreate_command()) ;

	R_close_driver();
	exit(stat);
}
