#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include "buttonbm.h"

#define MAXMAPS 60

make_pixmaps(w)
    Widget w;
{   
    int i;
    XImage *image;



    static char *name[] = {
    "colors",    "overlay",  "digitizer",
    "edit",      "help",     "label",
    "light",     "redraw",   "tool",
    "win",       "zoom",     "rmblock",
    "rmline",    "rmsite",   "snap",
    "break",     "mvlos",    "mvpt",
    "retype",    "line",     "area",
    "site",      "multiplelines", "bulk", 
    "contour",   "mouse",    "points",
    "markers",   "arealabs", "labline",
    "linelabs",  "sitelabs", "clear",
    "scale",     "where",    "backdrop",
    "unlabarea", "openarea", "dup",
    "nodeline",  "island",   "reg",
    "neat",      "beep",     "options",
    "spline",    "rmpoint",  "addpt",
    "debug",     "node",    "stream",
    "node1line", "bg",       "hilight",
    "nodeinthresh", "greymouse",
    NULL };

    char *maps[MAXMAPS]; 
    
    maps[0] = colors_bits; 
    maps[1] = overlay_bits; 
    maps[2] = dig_bits; 
    maps[3] = edit_bits;
    maps[4] = help_bits;
    maps[5] = label_bits;
    maps[6] = light_bits;
    maps[7] = redraw_bits;
    maps[8] = tool_bits;
    maps[9] = win_bits;
    maps[10] = zoom_bits;
    maps[11] = rmblock_bits;
    maps[12] = rmline_bits;
    maps[13] = rmsite_bits;
    maps[14] = snap_bits;
    maps[15] = break_bits;
    maps[16] = mvlos_bits;
    maps[17] = mvpt_bits;
    maps[18] = retype_bits;
    maps[19] = line_bits;
    maps[20] = area_bits;
    maps[21] = site_bits;
    maps[22] = multiplelines_bits;
    maps[23] = bulk_bits;
    maps[24] = contour_bits;
    maps[25] = mouse_bits;
    maps[26] = points_bits;
    maps[27] = markers_bits;
    maps[28] = arealabs_bits;
    maps[29] = labline_bits;
    maps[30] = linelabs_bits;
    maps[31] = sitelabs_bits;
    maps[32] = clear_bits;
    maps[33] = scale_bits;
    maps[34] = where_bits;
    maps[35] = backdrop_bits;
    maps[36] = unlabarea_bits;
    maps[37] = openarea_bits;
    maps[38] = dup_bits;
    maps[39] = nodeline_bits;
    maps[40] = island_bits;
    maps[41] = reg_bits;
    maps[42] = neat_bits;
    maps[43] = beep_bits;
    maps[44] = options_bits;
    maps[45] = spline_bits;
    maps[46] = rmpoint_bits;
    maps[47] = addpt_bits;
    maps[48] = debug_bits;
    maps[49] = nodes_bits;
    maps[50] = line_bits;
    maps[51] = node1line_bits;
    maps[52] = bg_bits;
    maps[53] = hilight_bits;
    maps[54] = nodeinthresh_bits;
    maps[55] = greymouse_bits;

    for (i = 0; name[i] != NULL; i++)
    {
	image = XCreateImage (XtDisplay (w),
			      DefaultVisualOfScreen (XtScreen(w)),
			      1, XYBitmap, 0, maps[i], 32, 32, 
			      BitmapPad(XtDisplay(w)), 4);
			      /*
	image->bitmap_bit_order = !BitmapBitOrder (XtDisplay (w));
	image->byte_order = !ImageByteOrder (XtDisplay (w));
	*/
	image->bitmap_bit_order = LSBFirst;
	image->byte_order = LSBFirst;
	XmInstallImage (image, name[i]);
    }
}

