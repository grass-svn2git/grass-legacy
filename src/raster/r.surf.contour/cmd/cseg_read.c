#include "gis.h"
#include "cseg.h"

static char *me="cseg_read_cell";

cseg_read_cell (cseg, map_name, mapset)
	CSEG	*cseg;
	char	*map_name, *mapset;
{
	int	row, nrows;
	int	map_fd;
	char	msg[100];
	CELL	*buffer;

	cseg->name   = NULL;
	cseg->mapset = NULL;

	if ((map_fd = G_open_cell_old(map_name, mapset)) < 0)
	{
		sprintf (msg, "%s(): unable to open file [%s] in [%s]", 
			me, map_name, mapset);
		G_warning (msg);
		return -3;
	}
	nrows = G_window_rows ();
	buffer = G_allocate_cell_buf ();
	for (row=0; row < nrows; row++)
	{
		if (G_get_map_row (map_fd, buffer, row) < 0)
		{
			free (buffer);
			G_close_cell (map_fd);
			sprintf (msg, "%s(): unable to read file [%s] in [%s]",
				me, map_name, mapset);
			G_warning (msg);
			return -2;
		}
		if (segment_put_row (&(cseg->seg), buffer, row) < 0)
		{
			free (buffer);
			G_close_cell (map_fd);
			sprintf (msg, "%s(): unable to segment put row for [%s] in [%s]", me, map_name, mapset);
			G_warning (msg);
			return (-1);
		}
	}

	G_close_cell (map_fd);
	free (buffer);

	cseg->name = G_store (map_name);
	cseg->mapset = G_store (mapset);

	return 0;
}
