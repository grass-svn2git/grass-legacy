#include "gis.h"
/*
 *   d.frame.choose [frame=name] {use mouse if frame= not specified}
 *
 *   Choose a frame on the screen
 */

main(argc, argv)
    char *argv[];
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
	exit(1);

    R_open_driver ();

    if (frame->answer)
    {
        if(stat = Dchoose(frame->answer))
            fprintf(stderr, "Error choosing frame %s\n", frame->answer) ;
        R_close_driver ();
        exit(stat) ;
    }

/* Save current frame just in case */
    D_get_cur_wind(orig_name) ;

    fprintf(stderr, "\nButtons:\n") ;
    fprintf(stderr, "Left:   Identify a frame\n") ;
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

    exit(0);
}
