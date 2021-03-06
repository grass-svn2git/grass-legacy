/* this routine is used by the tape extraction routines
 * it convert row,col windows into grass east,north windows
 * it writes the WIND file and sets the current program window
 * it sets the zone,proj to 0, resolutions to 1.0
 */
#include "gis.h"

int I__firstrow_;
int I__lastrow_;
int I__firstcol_;
int I__lastcol_;

I_set_window (firstrow, lastrow, firstcol, lastcol)
{
    struct Cell_head window;

    I__firstrow_ = firstrow;
    I__lastrow_  = lastrow;
    I__firstcol_ = firstcol;
    I__lastcol_  = lastcol;

    window.south  = -(lastrow + .5);
    window.north  = -(firstrow - .5);
    window.west   = firstcol - .5;
    window.east   = lastcol  + .5;
    window.cols   = lastcol - firstcol + 1;
    window.rows   = lastrow - firstrow + 1;
    window.ns_res = window.ew_res = 1.0;

    window.proj   = 0;
    window.zone   = 0;

    if(G_set_window (&window) < 0)
	return -1;
    return G_put_window (&window);
}
