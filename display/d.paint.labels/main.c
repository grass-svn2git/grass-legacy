#include <stdlib.h>
#include <math.h>
#include "gis.h"
#include "display.h"
#include "raster.h"
#include "local_proto.h"
#include "glocale.h"

int main (int argc, char **argv)
{
    struct Cell_head window ;
    char window_name[64] ;
    char *label_name ;
    char *mapset ;
    double minreg, maxreg, reg, dx, dy;
    FILE *infile ;
    char buff[128] ;
    int t, b, l, r ;
    struct Option *opt1;
    struct Option *maxreg_opt, *minreg_opt; 
    struct GModule *module;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]) ;
    
    /* Set description */
    module              = G_define_module();
    module->description = ""\
    "Displays text labels formatted for use with GRASS paint (p.labels, p.map)"
    " output to the active frame on the graphics monitor.";

    opt1 = G_define_option() ;
    opt1->key        = "labels" ;
    opt1->type       = TYPE_STRING ;
    opt1->required   = YES ;
    opt1->gisprompt  = "old,paint/labels,paint labels" ;
    opt1->description= "Name of label file" ;

    minreg_opt = G_define_option() ;
    minreg_opt->key         = "minreg" ;
    minreg_opt->type        = TYPE_DOUBLE ;
    minreg_opt->required    = NO ;
    minreg_opt->description = "Minimum region size (diagonal) when labels are displayed" ;

    maxreg_opt = G_define_option() ;
    maxreg_opt->key         = "maxreg" ;
    maxreg_opt->type        = TYPE_DOUBLE ;
    maxreg_opt->required    = NO ;
    maxreg_opt->description = "Maximum region size (diagonal) when labels are displayed" ;

    /* Check command line */
    if (G_parser(argc, argv)) exit(-1);

    /* Save map name */
    label_name = opt1->answer ;

    /* Make sure map is available */
    mapset = G_find_file ("paint/labels", label_name, "") ;
    if (mapset == NULL)
	    G_fatal_error( "Label file [%s] not available", label_name);

    if (R_open_driver() != 0)
	    G_fatal_error("No graphics device selected");

    /* Read in the map window associated with window */
    G_get_window(&window) ;

    /* Check min/max region */
    dx = window.east - window.west;
    dy = window.north - window.south;
    reg = sqrt ( dx*dx + dy*dy);
    if ( minreg_opt->answer ) {
        minreg = atof ( minreg_opt->answer );
        if ( reg < minreg ) {
            fprintf ( stdout, _("Region size is lower than minreg, nothing displayed.\n"));
	    D_add_to_list(G_recreate_command()) ;
            R_close_driver();
	    exit (0);
	}
    }
    if ( maxreg_opt->answer ) {
        maxreg = atof ( maxreg_opt->answer );
	if ( reg > maxreg ) {
	    fprintf ( stdout, _("Region size is greater than maxreg, nothing displayed.\n"));
	    D_add_to_list(G_recreate_command()) ;
            R_close_driver();
	    exit (0);
	}
    }
    
    /* Open map is available */
    infile = G_fopen_old ("paint/labels", label_name, mapset) ;
    if (infile == NULL)
    {
	    sprintf(buff,"Cant open labelfile [%s]", label_name);
	    G_fatal_error(buff) ;
    }

    if (D_get_cur_wind(window_name))
	    G_fatal_error("No current window") ;

    if (D_set_cur_wind(window_name))
	    G_fatal_error("Current window not available") ;


    if (D_check_map_window(&window))
	    G_fatal_error("Setting map window") ;

    if (G_set_window(&window) == -1) 
	    G_fatal_error("Current window not settable") ;

    /* Determine conversion factors */
    if (D_get_screen_window(&t, &b, &l, &r))
	    G_fatal_error("Getting screen window") ;
    if (D_do_conversions(&window, t, b, l, r))
	    G_fatal_error("Error in calculating conversions") ;

    /* Go draw the cell file */
    do_labels(infile) ;

    D_add_to_list(G_recreate_command()) ;

    R_close_driver();

    exit(0);
}
