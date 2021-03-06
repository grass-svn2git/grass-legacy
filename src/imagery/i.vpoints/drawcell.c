#include "globals.h"
drawcell(view,initflag)
    View *view;
    int initflag;
{
    int fd;
    int left, top;
    int ncols, nrows;
    int row;
    CELL *cell;
    int repeat;
    struct Colors *colors;
    char msg[100];
/*
FILE *pfd;
int i;
*/

/*
    if (!view->cell.configured) return 0;
    if (view == VIEW_MAP1 || view == VIEW_MAP1_ZOOM)
    {
	colors = &VIEW_MAP1->cell.colors;
	read_colors = view == VIEW_MAP1;
    }
    else
    {
	colors = &VIEW_MAP2->cell.colors;
	read_colors = view == VIEW_MAP2;
    }
colors = &VIEW_MAP1->cell.colors;
read_colors = view == VIEW_MAP1;
*/

if (!view->cell.configured) return 0;

colors = &VIEW_MAP1->cell.colors;

    display_title (view);

    Menu_msg("Please wait, initializing ...");

    if (initflag)
    {
	G_free_colors (colors);
	if(G_read_colors (view->cell.name, view->cell.mapset, colors) < 0)
	    return 0;
	set_menu_colors(colors);
    }

/*
pfd = popen("lp","w");
fprintf(pfd,"ver = %d, shift = %d, cmin = %d, cmax = %d\n",colors->version,colors->shift,colors->cmin,colors->cmax);

for(i = 0; i < 10; i++)
	fprintf(pfd,"%d. %d %d %d\n",i,colors->fixed.lookup.red[i],colors->fixed.lookup.grn[i],colors->fixed.lookup.blu[i]);

fprintf(pfd,"BLACK = %d WHITE = %d\n",BLACK,WHITE);
fflush(pfd);
pclose(pfd);
*/

R_standard_color(BLACK);

    if (initflag)
        {
        Erase_view (VIEW_TITLE1_ZOOM);
        Erase_view (VIEW_TITLE2_ZOOM);
        Erase_view (VIEW_MAP1_ZOOM);
        Erase_view (VIEW_MAP2);
        Erase_view (VIEW_MAP2_ZOOM);
        }

    G_set_window (&view->cell.head);
    nrows = G_window_rows();
    ncols = G_window_cols();

    left = view->cell.left;
    top = view->cell.top;

    R_standard_color(YELLOW);
    Outline_box (top, top+nrows-1, left, left+ncols-1);

{char *getenv();
 if (getenv("NO_DRAW")) return 1;
}

    fd = G_open_cell_old (view->cell.name, view->cell.mapset);
    if (fd < 0)
	return 0;
    cell = G_allocate_cell_buf();

    sprintf (msg, "Displaying %s ...", view->cell.name);
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
    free (cell);

/*
    if(colors != &VIEW_MAP1->cell.colors)
	set_colors(&VIEW_MAP1->cell.colors);
*/
    if(initflag)
	{
	D_new_window("warp_map",VIEW_MAP1->top,VIEW_MAP1->bottom,VIEW_MAP1->left,VIEW_MAP1->right);  /* initialize for overlay function in drawvect routine */
    return row==nrows;
	}
}
