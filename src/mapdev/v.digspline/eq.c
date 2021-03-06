#include "gis.h"
eq_grey_colors (name, mapset, colors, quiet)
    char *name, *mapset;
    struct Colors *colors;
{
    struct Cell_stats statf;
    CELL *cell;
    int row, nrows, ncols;
    int fd;

    if ((fd = G_open_cell_old (name,mapset)) < 0)
	exit(1);
    cell = G_allocate_cell_buf();
    nrows = G_window_rows();
    ncols = G_window_cols();

    G_init_cell_stats (&statf);
    if (!quiet)
	fprintf (stderr, "Reading %s ...", name);
    for (row = 0; row < nrows; row++)
    {
	if (!quiet)
	    G_percent (row, nrows, 2);
	if (G_get_map_row(fd, cell, row) < 0)
	    exit(1);
	G_update_cell_stats(cell, ncols, &statf);
    }
    if (!quiet)
	G_percent (row, nrows, 2);
    G_close_cell (fd);
    free (cell);
    G_make_histogram_eq_colors (colors, &statf);
    G_free_cell_stats (&statf);
}
