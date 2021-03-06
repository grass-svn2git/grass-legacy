#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../lib/graph.h"




extern int SCREEN_RIGHT, SCREEN_BOTTOM ;
extern XFontStruct *fontstruct;
extern Display *dpy;
extern Window grwin;
extern Pixmap bkupmap;
extern int backing_store;
extern GC gc;
extern unsigned SC_WID, SC_HITE;

static char grasstr[] = "GRASS 4.0 - X Display Window";


int Service_Xevent(wait)
int wait;
{
	static int firstime = 1;
	XEvent event;

	/* If wait is zero wait for and service the next X event. If wait
     * is non-zero see if any events are ready. If none just return. */
	if (wait) {
		if (!XPending(dpy)) /* this won't die if X server quits (shapiro) */
		{
/* XSync is a round-trip, we are here once per line. Bypass, w weber */
#ifndef OKI
			XSync(dpy, False); /* check if server still lives (shapiro) */
#endif /* OKI */
			return;             /* no events in queue, return */
		}
		XNextEvent(dpy, &event);
	}
	/* On the first Expose events, write the grass display message. For
     * now, on subsequent expose copy the backup window to the display
     * window. */
	if ((wait == 0) ||
	    (event.type == Expose && event.xexpose.count == 0)) {
		/* Remove any other pending Expose events from the queue to
         * avoid multiple repaints. */
		/*
        while (XCheckTypedEvent(dpy, Expose, &event)) ;
		*/

		if (firstime) {
			XWindowAttributes xwa;
			int x, y;


			firstime = 0;

			/* Get the window's current attributes. */
			if (XGetWindowAttributes(dpy, grwin, &xwa) == 0)
				return;
			x = (xwa.width -
			    XTextWidth(fontstruct, grasstr, strlen(grasstr))) / 2;
			y = (xwa.height + fontstruct->max_bounds.ascent
			    - fontstruct->max_bounds.descent) / 2;
			/* Fill the window with the background color,  and then
             * paint the centered string. */
			XClearWindow(dpy, grwin);
			XDrawString(dpy, grwin, gc, x, y, grasstr, strlen(grasstr));
		} else 
		{
			XWindowAttributes xwa;

			/* Get the window's current attributes. */
			if (XGetWindowAttributes(dpy, grwin, &xwa) == 0)
				return;

			SC_WID  = xwa.width;
			SC_HITE = xwa.height;
			SCREEN_RIGHT = xwa.width - 1;
			SCREEN_BOTTOM = xwa.height - 1;
			Set_window(1, xwa.height-1, 1, xwa.height-1);


			if (backing_store != Always)
				XCopyArea(dpy, bkupmap, grwin, gc, 0, 0, SC_WID, SC_HITE,
				    0, 0);
			firstime = 0;

		}

	}

}






/*** end Serve_Xevent.c ***/

