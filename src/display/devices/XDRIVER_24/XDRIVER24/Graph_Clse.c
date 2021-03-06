/* Close down the graphics processing.  This gets called only at driver
 * termination time. */

#include "includes.h"

Graph_Close()
{
    extern Display *dpy;
    extern Window grwin;

    XDestroyWindow(dpy, grwin);
    return (0);
}
