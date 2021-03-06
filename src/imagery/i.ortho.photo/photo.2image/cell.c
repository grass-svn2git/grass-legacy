
#include "globals.h"

static int use = 1;

plotcell(x,y)
{
    int cancel();
    int plot();
    static Objects objects[] =
    {
	MENU("CANCEL", cancel, &use),
	INFO("Indicate which side should be plotted", &use),
	OTHER(plot, &use),
	{0}
    };
/*
 * if the target cell file list is ready, ask the user which side
 * should be plotted, otherwise can only plot group files
 */
    if (access (cell_list,0) == 0)
	Input_pointer (objects);
    else
	plot (VIEW_MAP1->left+1,0);
    return 0;
}

static
cancel()
{
    return 1;
}


static
plot(x,y)
{
    char name[40], mapset[40];
    struct Cell_head cellhd;

    if (x > VIEW_MAP1->left && x < VIEW_MAP1->right)
    {
	if (!choose_cellfile(name,mapset))
	    return 1;
	if(G_get_cellhd(name, mapset, &cellhd) < 0)
	    return 1;

	Erase_view (VIEW_MAP1_ZOOM);
	VIEW_MAP1_ZOOM->cell.configured = 0;

	G_adjust_window_to_box (&cellhd, &VIEW_MAP1->cell.head, VIEW_MAP1->nrows, VIEW_MAP1->ncols);
	Configure_view (VIEW_MAP1, name, mapset, cellhd.ns_res, cellhd.ew_res);
	drawcell(VIEW_MAP1);
    }
    else
	return 0; /* ignore mouse click */

    display_ref_points(1);
    return 1;
}

static
choose_cellfile (name, mapset)
    char *name, *mapset;
{
    return ask_gis_files ("cell", cell_list, name, mapset, 1);
}


