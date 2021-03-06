#include "gis.h"
#include "cseg.h"

static char *me = "dseg_write_cell";

dseg_write_cellfile (dseg, map_name)
	DSEG	*dseg;
	char	*map_name;
{
	int	map_fd;
	int	row, col, nrows, ncols;
	char	msg[100];
	CELL	*buffer;
	double	*dbuffer;

	map_fd = G_open_cell_new (map_name);
	if (map_fd < 0)
	{
		sprintf (msg,"%s(): unable to open new map layer [%s]", 
			me, map_name);
		G_warning (msg);
		return -1;
	}
	nrows = G_window_rows ();
	ncols = G_window_cols ();
	buffer = G_allocate_cell_buf ();
	dbuffer = (double *) G_malloc (ncols * sizeof (double));
	segment_flush (&(dseg->seg));
	for (row=0; row<nrows; row++)
	{
		segment_get_row (&(dseg->seg), (char *)dbuffer, row);
		for (col = ncols -1; col >= 0; col--)
		{
			buffer[col] = (CELL) (dbuffer[col] + 0.5);
		}
		if (G_put_map_row (map_fd, buffer, row) < 0)
		{
			free (buffer);
			free (dbuffer);
			G_unopen_cell (map_fd);
			sprintf (msg,
			    "%s(): unable to write new map layer [%s], row %d", 
				me, map_name, row);
			G_warning (msg);
			return -2;
		}
	}
	free (buffer);
	free (dbuffer);
	G_close_cell (map_fd);
	return 0;
}
