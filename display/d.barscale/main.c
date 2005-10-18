#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "gis.h"
#include "display.h"
#include "raster.h"
#include "options.h"
#include "colors.h"
#include "glocale.h"

int color1;
int color2;
double east;
double north;
int use_feet;
int do_background = 1;
int do_bar = 1;

int main (int argc, char **argv)
{
	char window_name[64] ;
	struct Cell_head window ;
	int t, b, l, r ;
	struct GModule *module;
	struct Option *opt1, *opt2, *opt3 ;
	struct Flag *mouse, *feet, *top, *linescale ;
	struct Cell_head W ;
	int R, G, B;
	const int customFGcolor = MAXCOLORS + 1;
	const int customBGcolor = MAXCOLORS + 2;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]);

	module = G_define_module();
	module->description =
		_("Displays a barscale on GRASS monitor.");

	mouse = G_define_flag() ;
	mouse->key       = 'm';
	mouse->description= _("Use mouse to interactively place scale");

	feet = G_define_flag() ;
	feet->key        = 'f';
	feet->description= _("Use feet/miles instead of meters");

	linescale = G_define_flag() ;
	linescale->key   = 'l';
	linescale->description= _("Draw a line scale instead of a bar scale");

	top = G_define_flag() ;
	top->key         = 't';
	top->description= _("Write text on top of the scale, not to the right");

	opt1 = G_define_option() ;
	opt1->key        = "bcolor" ;
	opt1->type       = TYPE_STRING ;
	opt1->answer     = DEFAULT_BG_COLOR ;
	opt1->required   = NO ;
	opt1->description=
	    _("Background color, either a standard GRASS color, R:G:B triplet, or \"none\"");

	opt2 = G_define_option() ;
	opt2->key        = "tcolor" ;
	opt2->type       = TYPE_STRING ;
	opt2->answer     = DEFAULT_FG_COLOR ;
	opt2->required   = NO ;
/*	opt2->options    = D_color_list(); */
	opt2->description= _("Text color, either a standard GRASS color or R:G:B triplet");

	opt3 = G_define_option() ;
	opt3->key        = "at";
	opt3->key_desc   = "x,y";
	opt3->type       = TYPE_DOUBLE;
	opt3->answer     = "0.0,0.0";
	opt3->options    = "0-100" ;
	opt3->required   = NO;
	opt3->description=
	    _("The screen coordinates for top-left corner of label ([0,0] is top-left of frame)");

	if (G_parser(argc, argv) < 0)
		exit(-1);

	G_get_window(&W) ;
	if (W.proj == PROJECTION_LL)
	    G_fatal_error(_("%s does not work with a latitude-longitude location"), argv[0]);

	if(linescale->answer)
		do_bar = 0;

	use_feet = feet->answer ? 1 : 0;


        /* Parse and select background color */
	if(sscanf(opt1->answer, "%d:%d:%d", &R, &G, &B) == 3) {
	    if (R>=0 && R<256 && G>=0 && G<256 && B>=0 && B<256) {
		R_reset_color(R, G, B, customBGcolor);
		color1 = customBGcolor;
	    }
	}
	else if (!strcmp("none", opt1->answer)) {
	    do_background = 0;
	    color1 = 1;	/* dummy value */
	}
	else
	    color1 = D_translate_color(opt1->answer);

	if(!color1)
	    G_fatal_error(_("[%s]: No such color"), opt1->answer);


        /* Parse and select foreground color */
	if(sscanf(opt2->answer, "%d:%d:%d", &R, &G, &B) == 3) {
	    if (R>=0 && R<256 && G>=0 && G<256 && B>=0 && B<256) {
		R_reset_color(R, G, B, customFGcolor);
		color2 = customFGcolor;
	    }
	}
	else
	    color2 = D_translate_color(opt2->answer);

	if(!color2)
	    G_fatal_error(_("[%s]: No such color"), opt2->answer);


	sscanf(opt3->answers[0], "%lf", &east) ;
	sscanf(opt3->answers[1], "%lf", &north) ;

	if (R_open_driver() != 0)
	    G_fatal_error (_("No graphics device selected"));

	if (D_get_cur_wind(window_name))
	    G_fatal_error(_("No current window")) ;

	if (D_set_cur_wind(window_name))
	    G_fatal_error(_("Current window not available"));

	/* Read in the map window associated with window */
	G_get_window(&window);

	if (D_check_map_window(&window))
	    G_fatal_error(_("Setting map window"));

	if (G_set_window(&window) == -1)
	    G_fatal_error(_("Current window not settable"));

	/* Determine conversion factors */
	if (D_get_screen_window(&t, &b, &l, &r))
	    G_fatal_error(_("Getting screen window")) ;
	if (D_do_conversions(&window, t, b, l, r))
	    G_fatal_error(_("Error in calculating conversions")) ;

	if (!mouse->answer)
	{
		/* Draw the scale */
		draw_scale(NULL, top->answer) ;

		/* Add this command to list */
		D_add_to_list(G_recreate_command()) ;
	}
	else if (mouse_query(top->answer))
	{
		char cmdbuf[255];
		char buffer[255];
		
		sprintf(cmdbuf, "%s at=%f,%f", argv[0],east, north);
		
		if(opt1->answer)
		{
			sprintf(buffer, " bcolor=%s",opt1->answer);
			strcat(cmdbuf, buffer);
		}
		if(opt2->answer)
		{
			sprintf(buffer, " tcolor=%s",opt2->answer);
			strcat(cmdbuf, buffer);
		}
		if(top->answer)
			strcat(cmdbuf, " -t");
		if(feet->answer)
			strcat(cmdbuf, " -f");
		if(linescale->answer)
			strcat(cmdbuf, " -l");

		/* Add this command to list */
		D_add_to_list(cmdbuf) ;
		return 1;
	}

	R_close_driver();

	return 0;
}
