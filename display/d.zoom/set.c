#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gis.h"
#include "display.h"
#include "raster.h"
#include "local_proto.h"

static double round_to(double in, int sd) {
    double mult = 1, out;
    
    while ( rint(in * mult) < pow(10,sd-1)  )
	mult *= 10.0;
    
    out = rint(in * mult);
    out = out / mult;
    return out;
}

int set_win ( struct Cell_head *window, double ux1, double uy1, double ux2, double uy2, int hand)
{
    struct Cell_head defwin;
    double north, south, east, west;
    double tnorth, tsouth, teast, twest, td;
    double ncol, nrow, ew, ns, nsr, ewr;
    int resetres, resetwin, limit;
    int screen_x, screen_y, button ;

    resetwin =1;
    
    G_get_default_window(&defwin);

    north = uy1>uy2?uy1:uy2 ;
    south = uy1<uy2?uy1:uy2 ;
    west  = ux1<ux2?ux1:ux2 ;
    east  = ux1>ux2?ux1:ux2 ;
    
    G_limit_south (&south, window->proj);
    G_limit_north (&north, window->proj);
    G_limit_east  (&east, window->proj);
    G_limit_west  (&west, window->proj);
    if (  window->proj == PROJECTION_LL ) {
	if ( (east - west) > 360 ) {
	    fprintf(stderr, "(longitude range > 360) -> resetting\n") ;
	    td = (east + west) / 2;
	    east = td + 180;
	    west = td - 180;
	}
    }

    resetres = 1;
    while ( resetres ) {
	nsr = round_to(window->ns_res, 3); 
	ewr = round_to(window->ew_res, 3); 

	td = ceil (north / nsr);
	tnorth = td * nsr;
	td = floor (south / nsr);
	tsouth = td * nsr;
	td = rint (east / ewr);
	teast = td * ewr;
	td = rint (west / ewr);
	twest = td * ewr;

	ns = tnorth - tsouth;
	ew = teast - twest;
	if ( ( ns < 2 * window->ns_res  || ew < 2 * window->ew_res )  && !hand )   {
	    nsr = round_to (nsr/10.0, 3);
	    ewr = round_to (ewr/10.0, 3);
	    if ( nsr < 0.00000001 || ewr < 0.00000001 ) {
		fprintf(stderr, "Minimum resolution supported by d.zoom reached.\n") ;
		resetwin = 0;
		break;
	    }

	    fprintf(stderr, "\nResolution is too low for selected region.\n") ;
	    fprintf(stderr, "Buttons:\n") ;
	    fprintf(stderr, "%s Increase resolution to n-s = %.20f e-w = %.20f\n", LBTN, nsr, ewr);
	    fprintf(stderr, "%s Cancel (keep previous region)\n", MBTN) ;
	    fprintf(stderr, "%s Cancel (keep previous region)\n", RBTN) ;
	    
	    R_get_location_with_pointer(&screen_x, &screen_y, &button);

	    switch(button) {
		case LEFTB:
		    window->ns_res = nsr;
		    window->ew_res = ewr;
		    break;
		case MIDDLEB:
		case RIGHTB:
		    resetres = 0;
		    resetwin = 0;
		    break;
	     }	
	} else {
	    resetres = 0;
	}
    }

    nrow = ( tnorth - tsouth ) / window->ns_res; 
    ncol = ( teast - twest ) / window->ew_res; 
    if ( ( nrow > 10000000 || ncol > 10000000 ) && !hand ) {
	nsr = round_to ( window->ns_res * 10, 3); 
	ewr = round_to ( window->ew_res * 10, 3); 
	fprintf(stderr, "\nResolution is too high for selected region.\n") ;
	fprintf(stderr, "Buttons:\n") ;
	fprintf(stderr, "%s Decrease resolution to n-s = %.20f e-w = %.20f\n", LBTN, nsr, ewr);
	fprintf(stderr, "%s Keep current resolution\n", MBTN) ;
	fprintf(stderr, "%s Keep current resolution\n", RBTN) ;

	R_get_location_with_pointer(&screen_x, &screen_y, &button);

	switch(button) {
	    case LEFTB:
		window->ns_res = nsr;
		window->ew_res = ewr;
		td = rint (tnorth / nsr);
		tnorth = td * nsr;
		td = rint (tsouth / nsr);
		tsouth = td * nsr;
		td = rint(teast / ewr);
		teast = td * ewr;
		td = rint (twest / ewr);
		twest = td * ewr;
		break;
	    case MIDDLEB:
	    case RIGHTB:
		break;
	 }	
    }

    if ( window->proj == PROJECTION_LL ) {
	if(tnorth > 90)   tnorth = 90;
	if(tsouth < -90)  tsouth = -90;
	if(teast > 360)   teast -= 360; /* allow 0->360 as easting (e.g. Mars) */
	if(twest > 360)   twest -= 360;
	if(teast < -180)   teast += 360;
	if(twest < -180)   twest += 360;
    }

    if ( tnorth == tsouth ) tnorth += window->ns_res;
    if ( window->proj != PROJECTION_LL ) {
	if ( teast == twest )
	    teast += window->ew_res;
    }
    else {
	if( (fabs(teast-twest) <= window->ew_res) || 
	   (fabs(teast-360-twest) <= window->ew_res) ) {
		teast -= window->ew_res;
	}
    }

    if ( resetwin ) {
	window->north = tnorth;
	window->south = tsouth;
	window->east  = teast ;
	window->west  = twest ;

	if ( !hand ) {
	    fprintf(stderr, "\n") ;
	    print_win ( window, north, south, east, west );
	    fprintf(stderr, "\n") ;
	}

	limit = print_limit ( window, &defwin );

	G_adjust_Cell_head (window, 0, 0); 
	G_put_window(window);
	G_set_window(window);
	redraw();
    }
    
    return 1; 
}


