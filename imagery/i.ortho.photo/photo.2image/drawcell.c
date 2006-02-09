#include <stdlib.h>
#include <grass/gis.h>
#include <grass/display.h>
#include <grass/raster.h>
#include "globals.h"

int drawcell (View *view)
{
    int fd;
    int left, top;
    int ncols, nrows;
    int row;
    CELL *cell;
    struct Colors *colors = NULL;
    int repeat;
    int read_colors = 0;
    char msg[100];


    if (!view->cell.configured) return 0;
    if (view == VIEW_MAP1 || view == VIEW_MAP1_ZOOM)
    {
        colors = &VIEW_MAP1->cell.colors;
        read_colors = view == VIEW_MAP1;
    }
    if (read_colors)
    {
        G_free_colors (colors);
        if(G_read_colors (view->cell.name, view->cell.mapset, colors) < 0)
            return 0;
    }


    display_title (view);

    set_colors (colors);

    G_set_window (&view->cell.head);
    nrows = G_window_rows();
    ncols = G_window_cols();

    left = view->cell.left;
    top = view->cell.top;

    R_standard_color (BLUE);
    Outline_box (top, top+nrows-1, left, left+ncols-1);

    if (getenv("NO_DRAW")) return 1;

    fd = G_open_cell_old (view->cell.name, view->cell.mapset);
    if (fd < 0)
	return 0;
    cell = G_allocate_cell_buf();


    sprintf (msg, "Plotting %s ...", view->cell.name);
    Menu_msg(msg);

    for (row = 0; row < nrows; row += repeat)
    {
	R_move_abs (left, top+row);
	if(G_get_map_row_nomask(fd, cell, row) < 0)
	    break;
	repeat = G_row_repeat_nomask (fd, row);
	D_raster (cell, ncols, repeat, colors);
    }
    G_close_cell (fd);
    G_free (cell);

    if(colors != &VIEW_MAP1->cell.colors)
        set_colors(&VIEW_MAP1->cell.colors);

    return row==nrows;
}
