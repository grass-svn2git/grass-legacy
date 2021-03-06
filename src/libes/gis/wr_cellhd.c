/* write cell header, or window.  */

#include "gis.h"

G__write_Cell_head (fd, cellhd, is_cellhd)
    FILE *fd;
    struct Cell_head *cellhd;
{
    char buf[1024];
    int fmt;

    fmt = cellhd->proj;

    fprintf (fd, "proj:       %d\n",  cellhd->proj);
    fprintf (fd, "zone:       %d\n",  cellhd->zone);

    G_format_northing (cellhd->north,buf,fmt);
    fprintf (fd, "north:      %s\n", buf);

    G_format_northing (cellhd->south,buf,fmt);
    fprintf (fd, "south:      %s\n", buf);

    G_format_easting (cellhd->east,buf,fmt);
    fprintf (fd, "east:       %s\n", buf);

    G_format_easting (cellhd->west,buf,fmt);
    fprintf (fd, "west:       %s\n", buf);

    fprintf (fd, "cols:       %d\n", cellhd->cols);
    fprintf (fd, "rows:       %d\n", cellhd->rows);

    G_format_resolution (cellhd->ew_res,buf,fmt);
    fprintf (fd, "e-w resol:  %s\n", buf);

    G_format_resolution (cellhd->ns_res,buf,fmt);
    fprintf (fd, "n-s resol:  %s\n", buf);

    if (is_cellhd)
    {
	fprintf(fd,"format:     %d\n",  cellhd->format);
	fprintf(fd,"compressed: %d\n",  cellhd->compressed);
    }

    return 1;
}
