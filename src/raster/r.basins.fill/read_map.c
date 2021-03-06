#include "gis.h"

static int nrows, ncols;

CELL *
read_map (name, mapset, nomask, nrows, ncols)
    char *name;
    char *mapset;
{
    int fd;
    CELL *map;
    int row;
    int (*get_row)();
    int G_get_map_row();
    int G_get_map_row_nomask();

/* allocate entire map */
    map = (CELL *)G_malloc (nrows*ncols*sizeof(CELL));

/* open the map */
    if ((fd = G_open_cell_old (name, mapset)) < 0)
	die (name,mapset,"unable to open");

/* read the map */
    fprintf (stderr,"READING [%s] in [%s] ... ",name,mapset);
    fflush (stderr);

    if (nomask)
	get_row = G_get_map_row_nomask ;
    else
	get_row = G_get_map_row ;

    for (row = 0; row < nrows; row++)
    {
	G_percent (row, nrows, 10);
	if ((*get_row)(fd, map+row*ncols, row) < 0)
	{
	    fprintf (stderr,"\n");
	    die (name,mapset,"error reading");
	}
    }
    G_percent (nrows, nrows, 10);

    G_close_cell (fd);

    return map;
}
