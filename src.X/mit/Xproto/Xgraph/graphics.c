/*  %W%  %G%  */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display *dpy;
Window win;
Colormap colormap;
int scrn;
GC  gc;

graphics(infile)
FILE *infile;
{
    char buff[128];
    char *got_new;
    char *fgets();
    char *do_poly();

    int x, y;
	unsigned window_width, window_height, border_width, depth;
    Window root_return;
    XWindowAttributes xwa;

    /* Set the display to be the default display */
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, " can't open display\n");
        exit(-1);
    }
    scrn = DefaultScreen(dpy);
    win = XD_get_cur_window(dpy, scrn);
    gc = XCreateGC(dpy, win, 0, None);

    XGetGeometry(dpy, win, &root_return, &x, &y, &window_width,
        &window_height, &border_width, &depth);

    XGetWindowAttributes(dpy, win, &xwa);
    colormap = xwa.colormap;

    set_graph_stuff(window_width, window_height);

	set_Font("romand") ;

    got_new = fgets(buff, 128, infile);

    while (got_new) {
        switch (*buff & 0177) {
        case 't':
            do_text(buff);
            got_new = fgets(buff, 128, infile);
            break;
        case 's':
            do_size(buff);
            got_new = fgets(buff, 128, infile);
            break;
        case 'p':
            got_new = do_poly(buff, infile,
                window_height);
            break;
        case 'c':
            do_color(buff);
            got_new = fgets(buff, 128, infile);
            break;
        case 'm':
            do_move(buff);
            got_new = fgets(buff, 128, infile);
            break;
        case 'd':
            do_draw(buff);
            got_new = fgets(buff, 128, infile);
            break;
        case 'i':
            do_icon(buff, window_height);
            got_new = fgets(buff, 128, infile);
            break;
        case '#':
            got_new = fgets(buff, 128, infile);
            break;
        default:
            printf("Don't understand: %s", buff);
            got_new = fgets(buff, 128, infile);
            break;
        }
    }
    XFlush(dpy);
}
