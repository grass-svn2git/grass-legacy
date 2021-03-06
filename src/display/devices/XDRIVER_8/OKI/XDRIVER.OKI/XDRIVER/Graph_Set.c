/* This driver extensively updated by P. Thompson
 * (phils@athena.mit.edu) on 9/13/90 Driver modified to work with
 * Decstation X11r3 by David B. Satnik on 8/90 */
#include <stdio.h>
#include <signal.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include "icon.bit"
#include "win.h"

#define WIN_NAME   "OKIGRASS"
#define BORDER  1
#define FONT "9x15"

#define HORIZ_PIXELS 1000        /* 512 x 512 for small driver */
#define VERT_PIXELS  800        /* 1000 x 800 for big driver */


/* This program is a rewrite of the original Graph_Set from the GRASS
 * 3.0 version. All suncore and sunview related stuff (which was the
 * bulk of the original code) has been replaced by X11 library calls.
 * All non-suncore code has been retained. */

/* declare global variables */
int SCREEN_LEFT, SCREEN_RIGHT, SCREEN_BOTTOM, SCREEN_TOP;
unsigned SC_WID, SC_HITE;

/* DMJ - the OKI machine has 16 bit-planes and can probably handle more
 * colors than 256, you might want to experiment with NCOLOR to find
 * an optimal value. 
 */
int NCOLORS = 256, backing_store;

/*
Display *dpy = NULL;
Window grwin;
*/
int scrn = 0;
GC gc;
Colormap floatcmap, fixedcmap, InitColorTableFixed();
/*
Colormap fixedcmap, InitColorTableFixed();
*/
Cursor grcurse, grxh;
u_long gemask;
char *grasstr = "OKIGRASS";
XFontStruct *fontstruct;        /* Font descpritor */
Atom wmProtocolsAtom;
Pixmap bkupmap = (Pixmap)0;


int Graph_Set(argc, argv)
int argc;
char **argv;
{
    int i;
    XGCValues gcv;              /* Struct for creating GC */
    XSetWindowAttributes xswa;  /* Set Window Attribute struct */
    XWindowAttributes xwa;      /* Get Window Attribute struct */
    Atom closedownAtom;
    XTextProperty windowName, iconName;
    XSizeHints *szhints;
    XClassHint *clshints;
    XWMHints *wmhints;
    int sigint();
    XColor fg_color, bg_color, xcolors[256];

    /* Open the display using the $DISPLAY environment variable to
     * locate the X server. Return 0 if cannot open. */
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "Graph_Set: can't open %s\n", XDisplayName(NULL));
        return 0;
    }
    scrn = DefaultScreen(dpy);
    backing_store = DoesBackingStore(DefaultScreenOfDisplay(dpy));

    /* Load the font to use. GRASS doesn't at this point use fonts, but
     * may someday in the future. */
    if (!(fontstruct = XLoadQueryFont(dpy, FONT))) {
        fprintf(stderr, "Graph_SetX: display %s doesn't know font %s\n",
                DisplayString(dpy), FONT);
        return 0;
    }
    /* Allocate and set the fixed colormap, in the default X colormap
     * if possible, and set background and foreground of gc */
    /* DMJ - NCOLORS = DisplayCells(dpy, scrn);*/
    bg_color.pixel = BlackPixel(dpy, scrn);
    fg_color.pixel = WhitePixel(dpy, scrn);
    XQueryColor(dpy, DefaultColormap(dpy, scrn), &bg_color);
    XQueryColor(dpy, DefaultColormap(dpy, scrn), &fg_color);
    fixedcmap = InitColorTableFixed();
    (void) XAllocColor(dpy, fixedcmap, &bg_color);
    (void) XAllocColor(dpy, fixedcmap, &fg_color);

    /* Allocate floating colormap and set with b/w stripes */
    /*
     * DMJ - changed AllocAll to AllocAll to account for OKI's
     * DirectColor method of mapping colors (see Clr_table.c)
     */
    floatcmap = XCreateColormap(dpy, DefaultRootWindow(dpy),
            DefaultVisual(dpy, scrn), AllocNone);
    for (i = 0; i < 256; i++) {
        xcolors[i].pixel = i;
	/*
        xcolors[i].flags = DoRed | DoGreen | DoBlue;
        if (i & 1)
            xcolors[i].red = xcolors[i].green = xcolors[i].blue =
                    (u_short) 65535;
        else
            xcolors[i].red = xcolors[i].green = xcolors[i].blue = 0;
	*/
    }
    /* DMJ - can't store colors in an unallocated color table 
    XQueryColors(dpy, fixedcmap, xcolors, NCOLORS);
    XStoreColors(dpy, floatcmap, xcolors, NCOLORS);
    */

    /* Deal with providing the window with an initial position & size.
     * Window is is not resizable */
    szhints = XAllocSizeHints();
	/*
    szhints->flags = (PPosition | PSize | PMinSize | PMaxSize);
    szhints->flags = (USPosition | USSize | PMinSize | PMaxSize);
    szhints->width = szhints->min_width = szhints->max_width =
            HORIZ_PIXELS;
    szhints->height = szhints->min_height = szhints->max_height =
            VERT_PIXELS;
	*/
    szhints->flags = (USPosition | USSize | PMinSize | PMaxSize);
	szhints->width	= 450;
	szhints->height = 400;
	szhints->max_width	= DisplayWidth(dpy, scrn) - 20;
	szhints->min_width	= 0;
	szhints->min_height = 0;
	szhints->max_height = DisplayHeight(dpy, scrn) - 30 ;
    szhints->x = szhints->y = 10 ;

    /* Start with the window's colormap field pointing to the default
     * colormap. Later pointer enter events will cause a switch to the
     * grass color map. Also, set Bit Gravity to reduce Expose events. */
    xswa.event_mask = gemask = ExposureMask | ButtonPressMask | SubstructureNotifyMask ;

    xswa.backing_store = Always;
    xswa.colormap = fixedcmap;
    xswa.border_pixel = fg_color.pixel;
    xswa.background_pixel = bg_color.pixel;
    /* Create the Window with the information in the XSizeHints, the
     * border width,  and the border & background pixels. */
	 /*
    grwin = XCreateWindow(dpy, DefaultRootWindow(dpy), szhints->x,
            szhints->y, (unsigned)szhints->width,
			(unsigned)szhints->height, BORDER, DefaultDepth(dpy, scrn),
			InputOutput, DefaultVisual(dpy, scrn),
			(CWEventMask | CWBackingStore | CWColormap | CWBorderPixel |
			CWBackPixel), &xswa);
			*/

    grwin = XCreateWindow(dpy, DefaultRootWindow(dpy), szhints->x,
            szhints->y, (unsigned)szhints->width, (unsigned)szhints->height, 
	                BORDER,DefaultDepth(dpy, scrn),
			InputOutput, DefaultVisual(dpy, scrn),
			(CWEventMask | CWBackingStore | CWColormap | CWBorderPixel |
			CWBackPixel), &xswa);

    /* properties for window manager */
    wmhints = XAllocWMHints();
    wmhints->icon_pixmap = XCreateBitmapFromData(dpy, grwin, icon_bits,
            icon_width, icon_height);
    wmhints->flags |= IconPixmapHint;

    iconName.encoding = XA_STRING;
    iconName.format = 8;
    iconName.value = (u_char *) WIN_NAME;
    iconName.nitems = strlen((char *) iconName.value);
    windowName.encoding = iconName.encoding = XA_STRING;
    windowName.format = iconName.format = 8;
    windowName.value = (u_char *) WIN_NAME;
    windowName.nitems = strlen((char *) windowName.value);

    clshints = XAllocClassHint();
    clshints->res_name = NULL;
    clshints->res_class = WIN_NAME;
    XSetWMProperties(dpy, grwin, &windowName, &iconName, argv, argc,
            szhints, wmhints, clshints);
    closedownAtom = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmProtocolsAtom = XInternAtom(dpy, "WM_PROTOCOLS", False);
    (void) XSetWMProtocols(dpy, grwin, &closedownAtom, 1);

    /* Create the cursors to be used later */
    grcurse = grxh = XCreateFontCursor(dpy, XC_crosshair);

    /* Create the GC for writing the text. */
    gcv.font = fontstruct->fid;
    gcv.foreground = fg_color.pixel;
    gcv.background = bg_color.pixel;
    gc = XCreateGC(dpy, grwin, (GCFont | GCForeground | GCBackground), &gcv);

    /* Map the window to make it visible. This causes an expose event */
    XMapWindow(dpy, grwin);

    /* Find out how big the window really is (in case window manager
     * overrides our request) and set the SCREEN values. */
    SCREEN_LEFT = SCREEN_TOP = 0;
    if (XGetWindowAttributes(dpy, grwin, &xwa) == 0) {
        fprintf(stderr, "Graph_Set: cannot get window attributes\n");
        return 0;
    }
    SCREEN_RIGHT = xwa.width - 1;
    SCREEN_BOTTOM = xwa.height - 1;
    SC_WID = xwa.width;
    SC_HITE = xwa.height;

    if (backing_store != Always) {
        /* Now create a pixmap that will contain same contents as the
         * window. It will be used to redraw from after expose events */
        bkupmap = XCreatePixmap(dpy, grwin, SC_WID, SC_HITE, xwa.depth);
        XCopyArea(dpy, grwin, bkupmap, gc, 0, 0, (unsigned) SC_WID,
                (unsigned) SC_HITE, 0, 0);
    }
    /* prepare to catch signals */
    signal(SIGHUP, sigint);
    signal(SIGINT, sigint);
    signal(SIGQUIT, sigint);
    signal(SIGILL, sigint);
    signal(SIGTSTP, SIG_IGN);
    XFlush(dpy);
    return 1;
}

static sigint()
{
    Graph_Close();
    exit(-1);
}

/*** end Graph_Set.c ***/
