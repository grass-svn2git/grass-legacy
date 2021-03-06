/*  @(#)slid_window.c    1.0  12/18/89  */
/*
**-->  Written by Ron Glenn  12/1989
**  US Dept. Agri., Soil COnservation Service
**   based upon mk_window.c,  by Dave Gerdes, CERL
*/

#include "digit.h"
#include "wind.h"

slid_window_w_mouse ()
{
    char buffer[64] ;
    int screen_x, screen_y ;
    double ux1, uy1 ;
    double ux2, uy2 ;
    int button ;
    double N, S, E, W;
    int yn;
    double tmp;



    /*
    XtVaSetValues ( XtParent(choose), XtNx, Winx, XtNy, Winy + Hght/2, NULL);
    XmUpdateDisplay (toplevel);
    */
   
    show_select_dialog(NULL, "abort", "Specify new window center", 0) ;
    while (1)
    {


	get_location_with_pointer (&screen_x, &screen_y, &button);

	switch (button)
	{
	    case FIND:
		screen_to_utm ( screen_x, screen_y, &ux1, &uy1) ;
		tmp =  (ux1 - ((U_east + U_west) / 2));
		W = U_west + tmp;
		E = U_east + tmp;

		tmp =  (uy1 - ((U_north + U_south) / 2));
		S = U_south + tmp;
		N = U_north + tmp;

	        window_rout (N, S, E, W);
		/*
		clear_window();
	        replot(CM);
		*/
		redraw();

		break;

        case DONE:
	default:
    		XmUpdateDisplay (toplevel);
	        return(1) ;
	        break ;
	} /* end of switch */
    } /* end of while */
}
