/***********************************************************************
 *
 *   G_close_cell(fd)
 *      Closes and does housekeeping on an opened cell file
 *
 *   G_unopen_cell(fd)
 *      Closes and does housekeeping on an opened cell file
 *      without creating the cell file
 *
 *   parms:
 *      int fd     open cell file
 *
 *   returns:
 *      -1   on fail
 *       0   on success
 *
 *   note:
 *      On closing of a cell file that was open for writing, dummy cats
 *      and history files are created. Histogram and range info are written.
 *
 **********************************************************************/

#include "G.h"
#include <signal.h>

#define FCB G__.fileinfo[fd]

long lseek();

G_close_cell (fd)
{
    if (fd < 0 || fd >= MAXFILES || FCB.open_mode <= 0)
	return -1;
    if (FCB.open_mode == OPEN_OLD)
	return close_old (fd);
    else
	return close_new (fd, 1);
}

G_unopen_cell (fd)
{
    if (fd < 0 || fd >= MAXFILES || FCB.open_mode <= 0)
	return -1;
    if (FCB.open_mode == OPEN_OLD)
	return close_old (fd);
    else
	return close_new (fd, 0);
}

static
close_old (fd)
{
    if (FCB.cellhd.compressed)
	free (FCB.row_ptr);
    free (FCB.col_map);
    free (FCB.data);
    free (FCB.name);
    free (FCB.mapset);
    if (FCB.reclass_flag)
	G_free_reclass (&FCB.reclass);
    FCB.open_mode = -1;
    close (fd);
    return 1;
}

static
close_new (fd,ok)
{
    int stat;
    struct Categories cats;
    struct History hist;
    char buf[1000];
    char path[500];
    CELL *cell;
    int row;


    if (ok)
    {
#ifdef DEBUG
switch (FCB.open_mode)
{
case OPEN_NEW_COMPRESSED: printf ("close %s compressed\n",FCB.name); break;
case OPEN_NEW_UNCOMPRESSED: printf ("close %s uncompressed\n",FCB.name); break;
case OPEN_NEW_RANDOM: printf ("close %s random\n",FCB.name); break;
}
#endif
	if (FCB.open_mode != OPEN_NEW_RANDOM && FCB.cur_row < FCB.cellhd.rows)
	{
	    cell = (CELL *) G_malloc (FCB.cellhd.cols * sizeof(CELL));
	    G_zero (cell, FCB.cellhd.cols * sizeof(CELL));
	    for (row = FCB.cur_row; row < FCB.cellhd.rows; row++)
		G_put_map_row (fd, cell);
	    free (cell);
	}
	if (FCB.open_mode == OPEN_NEW_COMPRESSED) /* auto compression */
	{
	    long *row_ptr;
	    row_ptr = FCB.row_ptr;
	    row_ptr[FCB.cellhd.rows] = lseek (fd, 0L, 1);
	    G__write_row_ptrs (fd);
	}
	FCB.cellhd.format = FCB.nbytes - 1;
	FCB.cellhd.compressed = (FCB.open_mode == OPEN_NEW_COMPRESSED ? 1 : 0);
    }
    close (fd);
    FCB.open_mode = -1;

    if (FCB.data != NULL)
	free (FCB.data);

/* if the cell file was written to a temporary file
 * move this temporary file into the cell file
 * if the move fails, tell the user, but go ahead and create
 * the support files
 */
    stat = 1;
    if (ok && (FCB.temp_name != NULL))
    {
	G__file_name (path, "cell", FCB.name, FCB.mapset);
	unlink (path);	/* make sure cell file is gone */
	if(link (FCB.temp_name, path) < 0)
	{
	    sprintf(buf,"closecell: can't move %s\nto cell file %s in mapset %s",
		FCB.temp_name, FCB.name, FCB.mapset);
	    G_warning (buf);
	    stat = -1;
	}
	else
	{
	    unlink (FCB.temp_name);
	}
    }
    if (FCB.temp_name != NULL)
    {
	free (FCB.temp_name);
    }

    if (ok)
        G_put_cellhd (FCB.name, &FCB.cellhd);
    if (ok)
    {
/* create empty cats file */
	G_init_cats ((CELL)FCB.range.pmax, (char *)NULL, &cats);
	G_set_cat ((CELL)0, "no data", &cats);
        G_write_cats (FCB.name, &cats);
	G_free_cats (&cats);

/* remove color table */
	G_remove_colr (FCB.name);

/* create a history file */
        G_short_history (FCB.name, "raster", &hist);
	G_write_history (FCB.name, &hist);

/* write the range */
	G_write_range (FCB.name, &FCB.range);
/* write the histogram */
	if (FCB.want_histogram)
	    G_write_histogram_cs (FCB.name, &FCB.statf);
    }

    free (FCB.name);
    free (FCB.mapset);

    if (FCB.want_histogram)
	G_free_cell_stats (&FCB.statf);

    return stat;
}
