#include "gis.h"

CELL *
open_cell (name, mapset, fd)
    char *name, *mapset;
    int *fd;
{
    if (mapset == NULL) mapset = G_find_cell (name, "");
    *fd = G_open_cell_old (name, mapset);
    if (*fd >= 0)
	return G_allocate_cell_buf();
    
    fprintf (stderr, "ERROR: unable to open raster map [%s]\n", name);
    exit(1);
}
