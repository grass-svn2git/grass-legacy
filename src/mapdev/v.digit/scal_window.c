/*  @(#)scal_window.c    1.0  01/28/90  */
/*
**-->  Written by R.L.Glenn, USDA, SCS
**  from mk_window.c, By Dave Gerdes
**  US Army Construction Engineering Research Lab
*/

#include "digit.h"
#include "raster.h"
#include "wind.h"
#include "dig_curses.h"
#include "Map_proto.h"
#include "keyboard.h"
#include "local_proto.h"
#include "glocale.h"

#define SCALE_FACTOR 0.8

int 
scal_window_w_mouse (unsigned char type, struct line_pnts *Xpoints)
{
    int screen_x, screen_y ;
    int button, yn ;
    double N, S, E, W;
    double delta;
    char buffer[64] ;

    Clear_info ();

top:

    while (1)
    {
	_Clear_base ();
	_Write_base (12, _("Buttons:")) ;
	_Write_base (13, _("   Left:   Zoom in")) ;
	if(another_button){
		_Write_base (14, _("   Middle: Abort/Quit")) ;
		Write_base  (15, _("   Right:  Zoom out")) ;
	}else{
		_Write_base (14, _("   Middle: Zoom out")) ;
		Write_base  (15, _("   Right:  Abort/Quit")) ;
	}

        screen_x = screen_y = 1;
	R_get_location_with_pointer(&screen_x, &screen_y, &button) ;
	flush_keyboard (); /*ADDED*/
	Clear_info ();

	if(button == leftb){
		/* ZOOM IN */
                W = U_west  + (U_east - U_west)   * (1. - SCALE_FACTOR);
                E = U_east  - (U_east - U_west)   * (1. - SCALE_FACTOR);
                S = U_south + (U_north - U_south) * (1. - SCALE_FACTOR);
                N = U_north - (U_north - U_south) * (1. - SCALE_FACTOR);

		Clear_base ();
	        window_rout (N, S, E, W);
                clear_window ();
                replot(CMap); 
		if(Xpoints)
			highlight_line (type, Xpoints, 0, NULL);
		Clear_info ();
	}else
	if(button == middleb){
		/* ZOOM OUT */
                W = U_west  - (U_east - U_west)   * (1. - SCALE_FACTOR);
                E = U_east  + (U_east - U_west)   * (1. - SCALE_FACTOR);
                S = U_south - (U_north - U_south) * (1. - SCALE_FACTOR);
                N = U_north + (U_north - U_south) * (1. - SCALE_FACTOR);

		Clear_base ();
	        window_rout (N, S, E, W);
                clear_window ();
                replot(CMap);
		if(Xpoints)
			highlight_line (type, Xpoints, 0, NULL);
		Clear_info ();
	}else
	if(button == rightb){
                return(0);
	}else{
	        return(1) ;
	} /* end if */

      } /* end while */
}
