#include <stdio.h>
#include "includes.h"
#include "../lib/colors.h"

/* The systems color represented by "number" is set using the color
 * component intensities found in the "red", "grn", and "blu"
 * variables.  A value of 0 represents 0 intensity; a value of 255
 * represents 100% intensity. */


extern int scrn, NCOLORS;
extern Display *dpy;
extern Colormap floatcmap, fixedcmap;
extern char *calloc();
Window grwin;
u_long *xpixels;
int table_type = FIXED;
static int Red[256], Grn[256], Blu[256];


reset_color(number, red, grn, blu)
int number;
u_char red, grn, blu;
{
    XColor color;

    if ((number >= NCOLORS) || (number < 0)) {  /* ignore out-of-range */
        fprintf(stderr, "reset_color: can't set color %d\n", number);
        return;
    }

    /* convert to the 0-65535 range for X, put into XColor struct, and
     * set. */
    color.pixel = (u_long) number;
    color.red = (u_short) (red * 257);
    color.green = (u_short) (grn * 257);
    color.blue = (u_short) (blu * 257);
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(dpy, floatcmap, &color);
}


Color_table_float()
{
    float span;
    int r, g, b, i;
    unsigned char R, G, B;
    static int n_levels = 0;
    XColor get_clr, bg_color, fg_color;
    Colormap cmap;

    if (!can_do_float())
        return (-1);
    XSetWindowColormap(dpy, grwin, floatcmap);

    cmap = DefaultColormap(dpy, scrn);

    table_type = FLOAT;
    Color_offset(0);
    reset_color(RED, 255, 0, 0);
    reset_color(ORANGE, 255, 127, 0);
    reset_color(YELLOW, 255, 255, 0);
    reset_color(GREEN, 0, 255, 0);
    reset_color(BLUE, 0, 0, 255);
    reset_color(INDIGO, 0, 127, 255);
    reset_color(VIOLET, 255, 0, 255);
    reset_color(WHITE, 255, 255, 255);
    reset_color(BLACK, 0, 0, 0);
    reset_color(GRAY, 127, 127, 127);
    reset_color(BROWN, 180, 75, 25);
    reset_color(MAGENTA, 255, 0, 127);
    reset_color(AQUA, 100, 127, 255);

/*
	cmap = DefaultColormap(dpy, scrn);
	for (i=0; i<17; i++) {
		 get_clr.pixel = i;
		 XQueryColor(dpy, cmap, &get_clr);
		 reset_color(i, get_clr.red/257, get_clr.green/257, get_clr.blue/257);
		 }

		
    i = 17;
    if (n_levels == 0) {
        for (n_levels = 0; n_levels * n_levels * n_levels <= NCOLORS;
                n_levels++) ;
        n_levels--;
    }
    span = 255.0 / (float) n_levels;
    for (r = 0; r < n_levels; r++) {
        R = (int) (r * span + span);
        for (g = 0; g < n_levels; g++) {
            G = (int) (g * span + span);
            for (b = 0; b < n_levels; b++) {
                B = (int) (b * span + span);
				reset_color(i, R, G, B);
				i++;
				}
			}
		}
*/
    return 0;
}


Colormap InitColorTableFixed()
{
    float span;
    int r, g, b, i;
    unsigned char R, G, B;
    static int n_levels = 0;
    XColor xcolor;
    Colormap cmap;

    table_type = FIXED;
    /* figure out how many equal levels of r, g, and b are possible
     * with the available colors */
    if (n_levels == 0) {
        for (n_levels = 0; n_levels * n_levels * n_levels <= NCOLORS;
                n_levels++) ;
        n_levels--;
        /* Create easy lookup tables for _get_look_for_color() */
        for (i = 0; i < 256; i++) {
            Red[i] = (int) ((i / 256.0) * n_levels) * n_levels * n_levels;
            Grn[i] = (int) ((i / 256.0) * n_levels) * n_levels;
            Blu[i] = (int) ((i / 256.0) * n_levels);
        }
	/* allocate xpixels array */
	xpixels = (u_long *) calloc(n_levels*n_levels*n_levels, sizeof(u_long));
    }
    cmap = DefaultColormap(dpy, scrn);
    /* Generate "fixed" color table */
    span = 255.0 / (float) n_levels;
    i = 0;
    xcolor.flags = DoRed | DoGreen | DoBlue;
    for (r = 0; r < n_levels; r++) {
        R = (int) (r * span + span);
        for (g = 0; g < n_levels; g++) {
            G = (int) (g * span + span);
            for (b = 0; b < n_levels; b++) {
                B = (int) (b * span + span);
                xcolor.red = (u_short) (R * 257);
                xcolor.green = (u_short) (G * 257);
                xcolor.blue = (u_short) (B * 257);
                if (XAllocColor(dpy, cmap, &xcolor) == 0) {
                    cmap = XCopyColormapAndFree(dpy, cmap);
                    if (XAllocColor(dpy, cmap, &xcolor) == 0) {
                        fprintf(stderr, "Can't xalloc color %d.\n", i);
                        return 0;
                    }
                }
                xpixels[i++] = xcolor.pixel;
            }
        }
    }
    /* Generate lookup for "standard" colors */
    assign_standard_color(RED, _get_lookup_for_color(255, 0, 0));
    assign_standard_color(ORANGE, _get_lookup_for_color(255, 128, 0));
    assign_standard_color(YELLOW, _get_lookup_for_color(255, 255, 0));
    assign_standard_color(GREEN, _get_lookup_for_color(0, 255, 0));
    assign_standard_color(BLUE, _get_lookup_for_color(0, 0, 255));
    assign_standard_color(INDIGO, _get_lookup_for_color(0, 128, 255));
    assign_standard_color(VIOLET, _get_lookup_for_color(255, 0, 255));
    assign_standard_color(BLACK, _get_lookup_for_color(0, 0, 0));
    assign_standard_color(WHITE, _get_lookup_for_color(255, 255, 255));
    assign_standard_color(GRAY, _get_lookup_for_color(175, 175, 175));
    assign_standard_color(BROWN, _get_lookup_for_color(180, 77, 25));
    assign_standard_color(MAGENTA, _get_lookup_for_color(255, 0, 128));
    assign_standard_color(AQUA, _get_lookup_for_color(100, 128, 255));
    return cmap;
}


Color_table_fixed()
{
    table_type = FIXED;
    XSetWindowColormap(dpy, grwin, fixedcmap);
    return 0;
}

_get_lookup_for_color(red, grn, blu)
int red, grn, blu;
{
    return (Red[red] + Grn[grn] + Blu[blu]);
}

get_table_type()
{
    return table_type;
}

/*** end Clr_table.c ***/
