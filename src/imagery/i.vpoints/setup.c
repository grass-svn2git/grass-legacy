#include "gis.h"

/* D_setup (clear)
 *
 * This is a high level D call.
 * It does a full setup for the current graphics frame.
 *
 *   1. Makes sure there is a current graphics frame
 *      (will create a full-screen one, if not
 *   2. Sets the region coordinates so that the graphics frame
 *      and the active program region agree
 *      (may change active program region to do this).
 *   3. Performs graphic frame/region coordinate conversion intialization
 *
 * Returns: 0 if ok. Exits with error message if failure.
 *
 * Note: Connection to driver must already be made.
 *
 * clear values:
 *   1: clear frame (visually and coordinates)
 *   0: do not clear frame
 */

dsp_setup (blank,cellhead)
int blank;
struct Cell_head *cellhead;
{
    struct Cell_head region;
    char name[128];
    int t,b,l,r;

    if (D_get_cur_wind(name))
    {
	t = R_screen_top();
	b = R_screen_bot();
	l = R_screen_left();
	r = R_screen_rite();
	strcpy (name, "full_screen");
	D_new_window (name, t, b, l, r);
    }

    if (D_set_cur_wind(name))
	G_fatal_error("Current graphics frame not available") ;

    if (D_get_screen_window(&t, &b, &l, &r))
	G_fatal_error("Getting graphics coordinates") ;

/* clear the window, if requested to do so */
    if (blank)
    {
	D_clear_window();
	R_standard_color(blank);
	R_box_abs (l, t, r, b);
    }

    if (D_check_map_window(cellhead))
	G_fatal_error("Setting graphics coordinates") ;

    if(G_set_window (cellhead) < 0)
	G_fatal_error ("Invalid graphics window coordinates");

/* Determine conversion factors */
    if (D_do_conversions(cellhead, t, b, l, r))
	G_fatal_error("Error calculating graphics window conversions") ;

D_set_clip_window(t,b,l,r);

/* set text clipping, for good measure */
    R_set_window (t, b, l, r);
    R_move_abs(0,0);
    D_move_abs(0,0);
    return 0;
}
