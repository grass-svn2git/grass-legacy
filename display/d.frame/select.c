#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>
#include <grass/D.h>
#include <grass/glocale.h>

/*
 *   d.frame.choose [frame=name] {use mouse if frame= not specified}
 *
 *   Choose a frame on the screen
 */

int 
main (int argc, char *argv[])
{
    char orig_name[256] ;
    char cur_name[256] ;
    int stat ;
    int button ;

    struct Option *frame;

    frame = G_define_option();
    frame->key = "frame";
    frame->type= TYPE_STRING;
    frame->required = NO;
    frame->description = "Name of frame to choose (use mouse if not specified)";
    frame->answer = NULL;

    if (argc > 1 && G_parser(argc, argv))
	exit(EXIT_FAILURE);

    if (R_open_driver () != 0)
	G_fatal_error(_("No graphics device selected"));

    if (frame->answer)
    {
        if(stat = Dchoose(frame->answer))
            fprintf(stderr, _("Error choosing frame [%s]\n"), frame->answer);
        R_close_driver ();
        exit(stat) ;
    }

/* Save current frame just in case */
    D_get_cur_wind(orig_name) ;

    fprintf(stderr, "\nButtons:\n") ;
/* non functional */
/*    fprintf(stderr, "Left:   Identify a frame\n") ; */
    fprintf(stderr, "Middle: Keep original frame\n") ;
    fprintf(stderr, "Right:  Accept frame\n") ;

    button = ident_win(cur_name) ;

    if (button == 2)
    {
        D_set_cur_wind(orig_name) ;
	strcpy (cur_name, orig_name) ;
    }
    
    D_timestamp() ;

    R_close_driver() ;

    exit(EXIT_SUCCESS);
}
