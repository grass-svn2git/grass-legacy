/* select a monitor for graphics */

#include <grass/raster.h>
#include <grass/display.h>
#include <stdio.h>
#include <stdlib.h>
#include <grass/gis.h>
#include <grass/monitors.h>

int 
main (int argc, char *argv[])
{
	char command[1024];
	char name[128];
	const char *ftfont = getenv("GRASS_FT_FONT");
	const char *ftenc = getenv("GRASS_FT_ENCODING");
	const char *font = getenv("GRASS_FONT");

	if (argc != 2)
	{
		fprintf(stderr,"Usage:  %s monitor_name\n",argv[0]);
		exit(1);
	}

	G_gisinit (argv[0]);
    /* users objected to automatic release of current MONITOR
	sprintf (command, "%s/etc/mon.release -v", G_gisbase());
	system (command);
    */
	G_unsetenv ("MONITOR");

	if (R_parse_monitorcap(MON_NAME,argv[1]) == NULL)
	{
		fprintf(stderr,"No such monitor as '%s'\n",argv[1]);
		exit(1);
	}

	/* change the environment variable */
	G__setenv("MONITOR",argv[1]);

/* now try to run the monitor to see if it is running and to lock it
 * set the font
 * if no current frame create a full screen window.
 */
	/* Don't do anything else if connecting to the driver fails */
	if (R_open_driver() != 0)
	    exit(EXIT_FAILURE);

	R_font((ftfont ? ftfont : (font ? font : "romans")));

	if (ftenc)
		R_charset(ftenc);

	if (D_get_cur_wind(name) != 0)
		D_new_window("full_screen",
			     R_screen_top(), R_screen_bot(),
			     R_screen_left(), R_screen_rite());
	D_set_cur_wind("full_screen");

	R_close_driver();

	/* write the name to the .gisrc file */
	G__write_env();
	exit(0);
}
