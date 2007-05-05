/*
**********************************************************************
*
* MODULE:       r.support (GRASS core)
*
* AUTHOR(S):    Original by Michael Shapiro - CERL
*               Preliminary parser support by Markus Neteler, rast parameter
*               Port to 6.x by Brad Douglas
*
* PURPOSE:      Build support files for raster map
*               - Edit header
*               - Update status (histogram, range)
*
* COPYRIGHT:    (C) 2000-2007 by the GRASS Development Team
*
*               This program is free software under the GNU General 
*               Public License (>=v2). Read the file COPYING that comes
*               with GRASS for details.
*
**********************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "local_proto.h"

/* two less than lib/gis/put_title.c  G_put_cell_title()
    if only one less a newline gets appended in the cats file. bug? */
#define MAX_TITLE_LEN 1022

int main(int argc, char *argv[])
{
    char rname[GNAME_MAX];	/* Reclassed map name */
    char rmapset[GMAPSET_MAX];	/* Reclassed mapset   */
    char *mapset;		/* Raster mapset      */
    struct Cell_head cellhd;
    struct GModule *module;
    struct Option *raster, *title_opt, *history_opt, *map_opt;
    char buf[512];
    int cellhd_ok;		/* Is cell header OK? */
    int is_reclass;		/* Is raster reclass? */
    char *infile, *cmapset;
    char title[MAX_TITLE_LEN+1];
    struct History hist;

    /* Initialize GIS engine */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->keywords = _("raster");
    module->description = _("Allows creation and/or modification of "
                          "raster map layer support files.");

    raster = G_define_standard_option(G_OPT_R_INPUT);
    raster->key = "map";
    raster->required = YES;

    title_opt = G_define_option();
    title_opt->key = "title";
    title_opt->key_desc   = "\"phrase\"";
    title_opt->type        = TYPE_STRING;
    title_opt->required    = NO;
    title_opt->description = _("Text to use for new map title");

    history_opt = G_define_option();
    history_opt->key = "history";
    history_opt->key_desc   = "\"phrase\"";
    history_opt->type        = TYPE_STRING;
    history_opt->required    = NO;
    history_opt->description = _("Text to append to the next line of the map's metadata file");

    map_opt = G_define_option();
    map_opt->key = "rast";
    map_opt->type = TYPE_STRING;
    map_opt->required = NO;
    map_opt->gisprompt = "old,cell,raster";
    map_opt->description = _("Raster map name from which to copy category table");

    /* Parse command-line options */
    if (G_parser(argc,argv))
	exit(EXIT_FAILURE);

    /* Make sure raster exists and set mapset */
    infile = raster->answer;
    mapset = G_find_cell2(infile, G_mapset()); /* current mapset only for editing */
    if (mapset == NULL)
        G_fatal_error(_("Raster map <%s> not found in current mapset"), infile);

    cellhd_ok = (G_get_cellhd(raster->answer, mapset, &cellhd) >= 0);
    is_reclass = (G_is_reclass(raster->answer, mapset, rname, rmapset) > 0);

    if(title_opt->answer) {
	strncpy(title, title_opt->answer, MAX_TITLE_LEN);
	title[MAX_TITLE_LEN] = '\0'; /* strncpy doesn't null terminate oversized input */
	G_strip(title);
	G_debug(3, "map title= [%s]  (%d chars)", title, strlen(title));
	G_put_cell_title(raster->answer, title);

	if(! history_opt->answer)
	    exit(EXIT_SUCCESS);
    }

    if(history_opt->answer) {
	G_read_history (raster->answer, mapset, &hist);

	if(hist.edlinecnt >= MAXEDLINES)
	    G_fatal_error(_("Not enough room in history file"));

	/* two less than defined as if only one less a newline gets appended in the hist file. bug? */
        /* Should be RECORD_LEN, but r.info truncates at > 71 chars */
        if (strlen (history_opt->answer) > 71)
        {
            int i;

            for (i = 0; i < strlen (history_opt->answer); i += 71)
            {
                char *tmp = &history_opt->answer[i];

                strncpy (hist.edhist[hist.edlinecnt], tmp, 71);

                /* strncpy doesn't null terminate oversized input */
                hist.edhist[hist.edlinecnt][RECORD_LEN - 2] = '\0';
                hist.edlinecnt++;

                G_debug (3, "new history line= [%s] (%d chars)",
                            hist.edhist[hist.edlinecnt],
                            strlen (hist.edhist[hist.edlinecnt]));
            }
        } else {
            strncpy (hist.edhist[hist.edlinecnt], history_opt->answer, RECORD_LEN-2);

            /* strncpy doesn't null terminate oversized input */
            hist.edhist[hist.edlinecnt][RECORD_LEN-2] = '\0';
            hist.edlinecnt++;

            G_debug (3, "new history line= [%s] (%d chars)",
                        hist.edhist[hist.edlinecnt],
                        strlen (hist.edhist[hist.edlinecnt]));
        }

	G_write_history(raster->answer, &hist);
	exit(EXIT_SUCCESS);
    }

    if (map_opt->answer) {	/* use cats from another map */
	int fd;
	struct Categories cats;

	cmapset = G_find_cell2(map_opt->answer, "");
	if (cmapset == NULL)
	    G_fatal_error(_("Raster map <%s> not found"), map_opt->answer);

	if((fd = G_open_cell_old(infile,mapset)) < 0)
		G_fatal_error(_("Cannot open raster map <%s>"), infile);
	G_init_cats ((CELL)0, "", &cats);
	if (0 > G_read_cats(map_opt->answer, cmapset, &cats))
	    G_fatal_error(_("Cannot read category file of raster map <%s>"),
			  map_opt->answer);

	if (G_write_cats(infile, &cats) >= 0)
	    G_message(_("cats table for [%s] set to %s"), infile, map_opt->answer);
	G_close_cell(fd);
	G_free_cats(&cats);
	exit(EXIT_SUCCESS);
    }

    /* Cell header */
    sprintf(buf, _("Edit header for [%s]? "), raster->answer);
    if (is_reclass) {
        G_message(_("\nNOTE: [%s] is a reclass of [%s in %s]"),
                  raster->answer, rname, rmapset);
    } else if (G_yes(buf, cellhd_ok ? 0 : 1)) {
	G_clear_screen();

        run_etc_support("modhead", G_fully_qualified_name(raster->answer, mapset));

        if ((cellhd_ok = G_get_cellhd(raster->answer, mapset, &cellhd) > 0)) {
            hitreturn();
            G_clear_screen();
        } else if (!cellhd_ok)
            G_fatal_error(_("Canceling from edit header."));
    }

    /* Check the histogram and range */
    check_stats(raster->answer, mapset);

    /* Category file */
    sprintf(buf, _("Edit the category file for [%s]? "), raster->answer);
    if (G_yes(buf, 0)) {
        G_clear_screen();
        run_etc_support("modcats", G_fully_qualified_name(raster->answer, mapset));
        hitreturn();
        G_clear_screen();
    }

    /* Color table */
    sprintf(buf, _("Create/Update the color table for [%s]? "), raster->answer);
    if (G_yes(buf, 0)) {
        G_clear_screen();
        run_etc_support("modcolr", G_fully_qualified_name(raster->answer, mapset));
        hitreturn();
        G_clear_screen();
    }

    /* History file */
    sprintf(buf, _("Edit the history file for [%s]? "), raster->answer);
    if (G_yes(buf, 0)) {
        G_clear_screen();
        run_etc_support("modhist", G_fully_qualified_name(raster->answer, mapset));
        hitreturn();
        G_clear_screen();
    }

    /* null file */
    G_message(_("\nThe null file for [%s] may indicate that some "
              "cells contain\n no data. If the null file for [%s] "
              "doesn't exist, zero cells in\n it are treated by "
              "GRASS application programs as no data."), 
              raster->answer, raster->answer);

    sprintf(buf, _("\nDo you want to create/reset the null file "
            "for [%s] so that null cell values are considered valid data? "), raster->answer);
    if (G_yes(buf, 0)) {
        unsigned char *null_bits;
        int row, col;
        int null_fd;

        if (is_reclass)
            G_fatal_error(_("[%s] is a reclass of another map. Exiting."), raster->answer);

        G_clear_screen();
        /* Create a file of no-nulls */
        null_bits = G__allocate_null_bits(cellhd.cols);
        for (col = 0; col < G__null_bitstream_size(cellhd.cols); col++)
            null_bits[col] = 0;

        /* Open null file for writing */
        null_fd = G_open_new_misc("cell_misc", "null", raster->answer);
 
        G_message(_("Writing new null file for [%s]... "), raster->answer);
        for (row = 0; row < cellhd.rows; row++) {
            G_percent(row, cellhd.rows, 1);
            if (G__write_null_bits(null_fd, null_bits, row, cellhd.cols, 0) < 0)
                G_fatal_error(_("Error writing null row [%d]."), row);
        }
        G_percent(row, cellhd.rows, 1);

        /* Cleanup */
        close(null_fd);
        G_free(null_bits);

        hitreturn();
        G_clear_screen();
    }

    sprintf(buf, _("\nDo you want to delete the null file for [%s]\n"
            "(all zero cells will then be considered no data)? "), raster->answer);
    if (G_yes(buf, 0)) {
        int null_fd;
        char path[GPATH_MAX];

        if (is_reclass)
            G_fatal_error(_("[%s] is a reclass of another map. Exiting."), raster->answer);

        G_clear_screen();

        /* Write a file of no-nulls */
        G_message(_("Removing null file for [%s]...\n"), raster->answer);

        null_fd = G_open_new_misc("cell_misc", "null", raster->answer);
        G__file_name_misc(path, "cell_misc", "null", raster->answer, mapset);
        unlink(path);
        close(null_fd);

        G_done_msg(_("Done."));
    }

    return EXIT_SUCCESS;
}
