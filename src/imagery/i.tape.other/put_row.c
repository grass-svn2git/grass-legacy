#include "tape.h"

put_row (fd, buf, row)
    unsigned char *buf;
{
    CELL *c;
    int ncols;

    ncols = G_window_cols();
    c = cellbuf;
    while(ncols-- > 0)
	*c++ = *buf++;
    G_put_map_row (fd, cellbuf, row);
}
