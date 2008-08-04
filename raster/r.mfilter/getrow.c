#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "glob.h"
#include "local_proto.h"

int getmaprow(int fd, void *buf, int row, int len)
{
    if (G_get_map_row(fd, (CELL *) buf, row) < 0)
	G_fatal_error(_("Cannot read raster row %d"), row);
    return 1;
}

int getrow(int fd, void *buf, int row, int len)
{
    if (direction > 0)
	lseek(fd, (off_t) row * len, 0);
    else
	lseek(fd, (off_t) (nrows - row - 1) * len, 0);
    if (read(fd, (CELL *) buf, len) != len)
	G_fatal_error(_("Error reading temporary file"));
    return 1;
}
