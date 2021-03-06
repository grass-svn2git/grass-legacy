/*
 *   Digitize a raster map on the screen image
 */

#include "gis.h"

main(argc, argv)
    int argc ;
    char **argv ;
{
    char *polyfile;
    char name[256];
    FILE *fd;
    int any;

/* Initialize the GIS calls */
    G_gisinit(argv[0]) ;

#ifdef DEBUG
    polyfile = "/tmp/r.digit.out";
#else
    polyfile = G_tempfile();
#endif
    fd = fopen (polyfile, "w");
    if (fd == NULL)
    {
	perror (polyfile);
	exit(1);
    }

/* open the graphics and get it setup */
    R_open_driver();
    setup_graphics();

/* Do the digitizing and record the output into the polyfile */
    any = digitize(fd) ;
    fclose (fd);

/* close the graphics */
    R_close_driver();


#ifdef DEBUG
    printf ("Output is in %s\n", polyfile);
    exit(0);
#endif

/* ask for a map name */
    if (any && get_map_name(name))
	create_map (name, polyfile);
    else
	printf ("No map created\n");
    unlink (polyfile);
}
