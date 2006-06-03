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

#ifdef __MINGW32__
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "G.h"

#define FCB G__.fileinfo[fd]
#define FORMAT_FILE "f_format"
#define NULL_FILE   "null"

static int close_old (int);
static int close_new (int,int);
static char CELL_DIR[100];


/*!
 * \brief close a raster file
 *
 * The raster file
 * opened on file descriptor <b>fd</b> is closed. Memory allocated for raster
 * processing is freed. If open for writing, skeletal support files for the new
 * raster file are created as well.
 * <b>Note.</b> If a module wants to explicitly write support files (e.g., a
 * specific color table) for a raster file it creates, it must do so after the
 * raster file is closed. Otherwise the close will overwrite the support files.
 * See Raster_Map_Layer_Support_Routines for routines which write
 * raster support files.
 *
 *  \param fd
 *  \return int
 */

 
/*!
 * \brief 
 *
 * If the map is a new floating point, move the
 * <tt>.tmp</tt> file into the <tt>fcell</tt> element, create an empty file in the
 * <tt>cell</tt> directory; write the floating-point range file; write a default
 * quantization file quantization file is set here to round fp numbers (this is
 * a default for now). create an empty category file, with max cat = max value
 * (for backwards compatibility). Move the <tt>.tmp</tt> NULL-value bitmap file to
 * the <tt>cell_misc</tt> directory.
 *
 *  \return int
 */

int G_close_cell (int fd)
{
    if (fd < 0 || fd >= MAXFILES || FCB.open_mode <= 0)
	return -1;
    if (FCB.open_mode == OPEN_OLD)
	return close_old (fd);

    return close_new (fd, 1);
}


/*!
 * \brief unopen a raster file
 *
 * The raster file
 * opened on file descriptor <b>fd</b> is closed. Memory allocated for raster
 * processing is freed. If open for writing, the raster file is not created and
 * the temporary file created when the raster file was opened is removed (see
 * Creating_and_Opening_New_Raster_Files).
 * This routine is useful when errors are detected and it is desired to not
 * create the new raster file. While it is true that the raster file will not be
 * created if the module exits without closing the file, the temporary file will
 * not be removed at module exit. GRASS database management will eventually
 * remove the temporary file, but the file can be quite large and will take up
 * disk space until GRASS does remove it. Use this routine as a courtesy to the
 * user.  
 *
 *  \param fd
 *  \return int
 */

int G_unopen_cell (int fd)
{
    if (fd < 0 || fd >= MAXFILES || FCB.open_mode <= 0)
	return -1;
    if (FCB.open_mode == OPEN_OLD)
	return close_old (fd);
    else
	return close_new (fd, 0);
}

static int close_old (int fd)
{
   int i;

    /* if G__.auto_mask was only allocated for reading map rows to create
       non-existant null rows, and not for actuall mask, free G__.mask_row 
       if(G__.auto_mask <=0)
          G_free (G__.mask_buf);
       This is obsolete since now the mask_bus is always allocated
    */

    for (i=0;i<NULL_ROWS_INMEM;i++)
       G_free (FCB.NULL_ROWS[i]);
    G_free (FCB.null_work_buf);

    if (FCB.cellhd.compressed)
	G_free (FCB.row_ptr);
    G_free (FCB.col_map);
    G_free (FCB.mapset);
    G_free (FCB.data);
    G_free (FCB.name);
    if (FCB.reclass_flag)
	G_free_reclass (&FCB.reclass);
    FCB.open_mode = -1;

    if(FCB.map_type != CELL_TYPE)
    {
        G_quant_free(&FCB.quant);
        xdr_destroy(&FCB.xdrstream);
    } 
    close (fd);

    return 1;
}

static int close_new (int fd,int ok)
{
    int stat;
    struct Categories cats;
    struct History hist;
    char path[4096];
    CELL cell_min, cell_max;
    int row, i, open_mode;
    char element[100];

    if (ok) {
        switch (FCB.open_mode)
        {
        case OPEN_NEW_COMPRESSED:
            G_debug(1, "close %s compressed", FCB.name); 
            break;
        case OPEN_NEW_UNCOMPRESSED:
            G_debug(1, "close %s uncompressed", FCB.name);
            break;
        case OPEN_NEW_RANDOM:
            G_debug(1, "close %s random", FCB.name);
            break;
        }

	if (FCB.open_mode != OPEN_NEW_RANDOM && FCB.cur_row < FCB.cellhd.rows) {
	    G_zero_raster_buf (FCB.data, FCB.map_type);
	    for (row = FCB.cur_row; row < FCB.cellhd.rows; row++)
	        G_put_raster_row (fd, FCB.data, FCB.map_type);
            G_free (FCB.data);
	    FCB.data = NULL;
	}

        /* create path : full null file name */
        sprintf(element,"cell_misc/%s",FCB.name);
        G__file_name(path, element, NULL_FILE, G_mapset());
        G__make_mapset_element(element);
        remove ( path );

        if(FCB.null_cur_row > 0) {
        /* if temporary NULL file exists, write it into cell_misc/name/null */
            int null_fd;

            null_fd = G__open_null_write(fd);
	    if(null_fd <= 0) return -1;
            if(null_fd < 1) return -1;

            /* first finish writing null file */
            /* write out the rows stored in memory */
	    for (row = FCB.min_null_row; 
                  row < FCB.null_cur_row; row++)
             G__write_null_bits(null_fd, FCB.NULL_ROWS[row - FCB.min_null_row], 
                                               row, FCB.cellhd.cols, fd); 

            /* write missing rows */
	    if (FCB.open_mode != OPEN_NEW_RANDOM 
                && FCB.null_cur_row < FCB.cellhd.rows)
            {
                G__init_null_bits(FCB.null_work_buf, FCB.cellhd.cols);
	        for (row = FCB.null_cur_row; row < FCB.cellhd.rows; row++)
                      G__write_null_bits(null_fd, FCB.null_work_buf, row, 
                                                          FCB.cellhd.cols, fd);
            }
            close (null_fd);
            
#ifdef __MINGW32__
	    if ( CopyFile ( FCB.null_temp_name, path, FALSE ) == 0 ) {
#else
	    if(link (FCB.null_temp_name, path) < 0) {
#endif
		if(rename(FCB.null_temp_name, path)) {
	            G_warning(_("closecell: can't move %s\nto null file %s"),
		                FCB.null_temp_name, path);
	            stat = -1;
	        }
	    } else {
                remove ( FCB.null_temp_name );
            }
        } else {
            remove ( FCB.null_temp_name );
            remove ( path );
        } /* null_cur_row > 0 */

        if (FCB.open_mode == OPEN_NEW_COMPRESSED) { /* auto compression */
            FCB.row_ptr[FCB.cellhd.rows] = lseek (fd, 0L, 1);
            G__write_row_ptrs (fd);
        }

        if(FCB.map_type != CELL_TYPE) {  /* floating point map */
           int cell_fd;

            if (G__write_fp_format(fd) != 0) {
               G_warning(_("Error writing floating point format file for map %s"), FCB.name);
               stat = -1;
            }

            /* now write 0-length cell file */
            G__make_mapset_element ("cell");
            cell_fd = creat (G__file_name(path, "cell", FCB.name, FCB.mapset), 0666);
            close( cell_fd);
            strcpy(CELL_DIR, "fcell");
       } else {
            /* remove fcell/name file */
   	    G__file_name (path, "fcell", FCB.name, FCB.mapset);
            remove ( path );
            /* remove cell_misc/name/f_format */
            sprintf(element,"cell_misc/%s",FCB.name);
   	    G__file_name (path, element, "f_format", FCB.mapset);
            remove ( path );
            strcpy(CELL_DIR, "cell");
            close (fd);
       }
    } /* ok */
    /* NOW CLOSE THE FILE DESCRIPTOR */

    close (fd);
    /* remember open_mode */
    open_mode = FCB.open_mode;
    FCB.open_mode = -1;

    if (FCB.data != NULL)
	G_free (FCB.data);

    if (FCB.null_temp_name != NULL)
    {
	G_free (FCB.null_temp_name);
        FCB.null_temp_name = NULL;
    }

/* if the cell file was written to a temporary file
 * move this temporary file into the cell file
 * if the move fails, tell the user, but go ahead and create
 * the support files
 */
    stat = 1;
    if (ok && (FCB.temp_name != NULL)) {
	G__file_name (path, CELL_DIR, FCB.name, FCB.mapset);
        remove ( path );
#ifdef __MINGW32__
        if ( CopyFile ( FCB.temp_name, path, FALSE ) == 0 ) {
#else
	if(link (FCB.temp_name, path) < 0) {
#endif
	    if(rename(FCB.temp_name, path)) {
	        G_warning(_("closecell: can't move %s\nto cell file %s"),
	                    FCB.temp_name, path);
	        stat = -1;
	    }
        } else {
            remove ( FCB.temp_name );
        }
    }

    if (FCB.temp_name != NULL) {
	G_free (FCB.temp_name);
    }

    if (ok) {
	/* remove color table */
	G_remove_colr (FCB.name);

	/* create a history file */
        G_short_history (FCB.name, "raster", &hist);
	G_write_history (FCB.name, &hist);

	/* write the range */
        if(FCB.map_type == CELL_TYPE) {
       	     G_write_range (FCB.name, &FCB.range);
             G__remove_fp_range(FCB.name);
        }
	/*NOTE: int range for floating point maps is not written out */
        else /* if(FCB.map_type != CELL_TYPE) */
        {
       	     G_write_fp_range (FCB.name, &FCB.fp_range);
	     G_construct_default_range(&FCB.range);
	    /* this range will be used to add default rule to quant structure */
        }

        if ( FCB.map_type != CELL_TYPE)
           FCB.cellhd.format = -1;
        else /* CELL map */
	   FCB.cellhd.format = FCB.nbytes - 1;

	/* write header file */
        G_put_cellhd (FCB.name, &FCB.cellhd);

	/* if map is floating point write the quant rules, otherwise remove f_quant */
        if(FCB.map_type != CELL_TYPE) {
	/* DEFAULT RANGE QUANT
	     G_get_fp_range_min_max(&FCB.fp_range, &dcell_min, &dcell_max);
	     if(!G_is_d_null_value(&dcell_min) && !G_is_d_null_value(&dcell_max))
             {
		G_get_range_min_max(&FCB.range, &cell_min, &cell_max);
	        G_quant_add_rule(&FCB.quant, dcell_min, dcell_max, 
					     cell_min, cell_max);
             }
        */
	     G_quant_round(&FCB.quant);
             if( G_write_quant (FCB.name, FCB.mapset, &FCB.quant) < 0)
                      G_warning(_("unable to write quant file!"));
        } else {
            /* remove cell_misc/name/f_quant */
            sprintf(element,"cell_misc/%s",FCB.name);
   	    G__file_name (path, element, "f_quant", FCB.mapset);
            remove ( path );
        }

	/* create empty cats file */
       G_get_range_min_max(&FCB.range, &cell_min, &cell_max);
       if(G_is_c_null_value(&cell_max)) cell_max = 0;
       G_init_cats (cell_max, (char *)NULL, &cats);
       G_write_cats (FCB.name, &cats);
       G_free_cats (&cats);

	/* write the histogram */
	/* only works for integer maps */
       if((FCB.map_type == CELL_TYPE)
            &&(FCB.want_histogram)) {
        	    G_write_histogram_cs (FCB.name, &FCB.statf);
  	            G_free_cell_stats (&FCB.statf);
       } else {
            G_remove_histogram(FCB.name);
       }
    } /* OK */

    G_free (FCB.name);
    G_free (FCB.mapset);

    for (i=0;i<NULL_ROWS_INMEM;i++)
       G_free (FCB.NULL_ROWS[i]);
    G_free (FCB.null_work_buf);

    if(FCB.map_type != CELL_TYPE)
       G_quant_free(&FCB.quant);

    return stat;
}

/* returns 0 on success, 1 on failure */
int G__write_fp_format (int fd)
{
   struct Key_Value *format_kv;
   char element[100], path[4096];
   int stat;

   if(FCB.map_type == CELL_TYPE)
   {
       G_warning(_("unable to write f_format file for CELL maps"));
       return 0;
   }
   format_kv = G_create_key_value();
   if(FCB.map_type == FCELL_TYPE)
       G_set_key_value ("type", "float", format_kv);
   else 
       G_set_key_value ("type", "double", format_kv);

   G_set_key_value ("byte_order", "xdr", format_kv);

   if (FCB.open_mode == OPEN_NEW_COMPRESSED)
      G_set_key_value ("lzw_compression_bits", "-1", format_kv);

   sprintf(element,"cell_misc/%s",FCB.name);
   G__file_name(path,element,FORMAT_FILE,FCB.mapset);

   G__make_mapset_element(element);
   G_write_key_value_file (path, format_kv, &stat);

   G_free_key_value(format_kv);

   return stat;
}
