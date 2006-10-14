/****************************************************************************
 *
 * MODULE:       r.compress
 *
 * AUTHOR(S):    James Westervelt - CERL
 *               Michael Shapiro - CERL
 *
 * PURPOSE:      Compress and decompress raster map files.
 *
 * COPYRIGHT:    (C) 2003 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 ***************************************************************************/

/****************************************************************************
 * compress_cell converts straight grid_cell files into compressed grid_cell
 * files.  Compressed files have the following format:
 *
 *  - Array of addresses pointing to the internal start of each row
 *    First byte of each row is the nuber of bytes per cell for that row
 *    Remainder of the row is a series of byte groups that describe the data:
 *        First byte: number of cells that contain the category given by second
 *        Next byte(s): category number. The number of bytes is determined
 *                      by the number of bytes in a cell 
 *
 * The normal G_open_cell(), and G_put_raster_row() do the compression
 * This program must only check that the file is not a reclass file and
 * is not already compressed.
 *
 * The only trick is to preserve the support files
 *
 *****************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <grass/gis.h>
#include <grass/glocale.h>

long newsize, oldsize;
int process(char *, int);
int doit(char *, int, RASTER_MAP_TYPE);

int main (int argc, char *argv[])
{
    int stat ;
    int n;
    char *name;
	struct GModule *module;
    struct Option *map;
    struct Flag *uncompress;

    /* please, remove before GRASS 7 released */
    struct Flag *q_flag;

    G_gisinit (argv[0]);

	module = G_define_module();
	module->keywords = _("raster");
    module->description =
		_("Compresses and decompresses raster files.");

    map = G_define_option();
    map->key = "map";
    map->type = TYPE_STRING;
    map->required = YES;
	map->gisprompt  = "old,cell,raster" ;
    map->multiple = YES;
    map->description = _("Name of existing raster map(s)");

    uncompress = G_define_flag();
    uncompress->key = 'u';
    uncompress->description = _("Uncompress the map");

    /* please, remove before GRASS 7 released */
    q_flag = G_define_flag() ;
    q_flag->key         = 'q' ;  
    q_flag->description = _("Run quietly") ;

    if (G_parser(argc,argv))
	exit(EXIT_FAILURE);

    /* please, remove before GRASS 7 released */
    if(q_flag->answer) {
        G_putenv("GRASS_VERBOSE","0");
        G_warning(_("The '-q' flag is superseded and will be removed "
            "in future. Please use '--quiet' instead."));
    }

    stat = 0;
    for (n = 0; (name = map->answers[n]); n++)
	if (process (name, uncompress->answer))
	    stat = 1;
    exit (stat);
}


int 
process (char *name, int uncompress)
{
    struct Colors colr;
    struct History hist;
    struct Categories cats;
    struct Quant quant;
    int colr_ok;
    int hist_ok;
    int cats_ok;
    int quant_ok;
    long diff;
    RASTER_MAP_TYPE map_type;
    char rname[256], rmapset[256];

    if (G_find_cell (name, G_mapset()) == NULL)
    {
	G_warning (_("[%s] not found"), name);
	return 1;
    }
    if (G_is_reclass (name, G_mapset(), rname, rmapset) > 0)
    {
	G_warning (_("[%s] is a reclass file of map <%s> in mapset <%s> - can't %scompress"), name, rname, rmapset, uncompress?"un":"");
	return 1;
    }

    map_type = G_raster_map_type(name, G_mapset());

    G_suppress_warnings(1);
    colr_ok = G_read_colors (name, G_mapset(), &colr) > 0;
    hist_ok = G_read_history (name, G_mapset(), &hist) >= 0;
    cats_ok = G_read_cats (name, G_mapset(), &cats) >= 0;

    if(map_type != CELL_TYPE)
    {
       G_quant_init(&quant);
       quant_ok = G_read_quant(name, G_mapset(), &quant);
       G_suppress_warnings(0);
    }

    if (doit(name,uncompress, map_type)) return 1;

    if (colr_ok)
    {
	G_write_colors (name, G_mapset(), &colr);
	G_free_colors (&colr);
    }
    if (hist_ok)
	G_write_history (name, &hist);
    if (cats_ok)
    {
	cats.num = G_number_of_cats (name, G_mapset());
	G_write_cats (name, &cats);
	G_free_cats (&cats);
    }
    if (map_type != CELL_TYPE && quant_ok)
	G_write_quant (name, G_mapset(), &quant);
    diff = newsize - oldsize;
    if (diff < 0)
    {
	diff = -diff;
        G_message (_("DONE: %scompressed file is %ld byte%s smaller"), 
                uncompress?"un":"", diff, diff==1?"":"s");
    }
    else if (diff > 0)
    {
        G_message (_("DONE: %scompressed file is %ld byte%s bigger"), 
                uncompress?"un":"", diff, diff==1?"":"s");
    }
    else
    {
	G_message ("same size");
    }
    return 0;
}

int 
doit (char *name, int uncompress, RASTER_MAP_TYPE map_type)
{
    struct Cell_head cellhd ;
    int new, old, nrows, row;
    void *rast;

    if (G_get_cellhd (name, G_mapset(), &cellhd) < 0)
    {
	char msg[100];
	sprintf (msg,"%s: Problem reading cell header for [%s]", G_program_name(), name);
	G_warning(msg);
	return 1;
    }


/* check if already compressed/decompressed */
    if (uncompress && cellhd.compressed == 0)
    {
	G_warning (_("[%s] already uncompressed"), name);
	return 1;
    }
    else if (!uncompress && cellhd.compressed > 0)
    {
	G_warning (_("[%s] already compressed"), name);
	return 1;
    }

    G_message (_("\n%sCOMPRESS [%s]"), uncompress?"UN":"", name);

    G_set_window (&cellhd);

    old = G_open_cell_old (name, G_mapset());
    if (old < 0)
	return 1;

    if (uncompress)
    {
        if(map_type == CELL_TYPE)
	{
 	   G_set_cell_format (cellhd.format);
	   new = G_open_cell_new_uncompressed (name);
	}
        else
        {
    	   G_set_fp_type(map_type);
	   new = G_open_fp_cell_new_uncompressed (name);
        }
    }
    else
	new = G_open_raster_new(name, map_type);

    if (new < 0)
	return 1;
    nrows = G_window_rows();
    rast = G_allocate_raster_buf(map_type);

    oldsize = lseek (old, 0L, 2);

    /* the null file is written automatically */
    for (row = 0; row < nrows; row++)
    {
        G_percent (row, nrows, 2);
       if (G_get_raster_row_nomask (old, rast, row, map_type) < 0)
           break;
       if (G_put_raster_row (new, rast, map_type) < 0)
           break;
    }
    G_free (rast);
    G_close_cell (old);
    if (row < nrows)
    {
	G_unopen_cell (new);
	return 1;
    }
    G_close_cell (new);
    newsize = 0;
    old = G_open_cell_old (name, G_mapset());
    newsize = lseek (old, 0L, 2);
    G_close_cell (old);
    return 0;
}
