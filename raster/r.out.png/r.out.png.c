/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:       r.out.png
 * AUTHOR(S):    Bill Brown - USA-CERL
 *               Alex Shevlakov - sixote@yahoo.com
 * PURPOSE:      Export GRASS raster as non-georeferenced PNG image.
 * COPYRIGHT:    (C) 2000 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

/* 
 * based on r.out.ppm by
 * Written by Bill Brown, USA-CERL March 21, 1994
 * 
 */

/* Use to convert grass raster file to PNG
 * uses currently selected region
 */

/*              Alex Shevlakov, sixote@yahoo.com, 03/2000 
 */

#include <string.h>
#include <stdlib.h>

#ifndef _MYINCLUDE_H
#define _MYINCLUDE_H
#include <png.h>
#include "pngfunc.h"
/* #include <pnm.h> this is already included from pngfunc.h */
#endif /* _MYINCLUDE_H */

#include "version.h"		/* VERSION macro */

#include "gis.h"

#define DEF_RED 255
#define DEF_GRN 255
#define DEF_BLU 255

typedef int FILEDESC;

int main(int argc, char *argv[])
{
    struct GModule *module;
    struct Option *rast, *png_file;
    struct Flag *bequiet, *gscale;
    char *cellmap, *map, *p, errbuf[100], ofile[1000];
    char rastermap[1024];
    unsigned char *set, *ored, *ogrn, *oblu;
    CELL *cell_buf;
    FCELL *fcell_buf;
    DCELL *dcell_buf;
    void *voidc;
    int rtype, row, col, do_stdout = 0;
    struct Cell_head w;
    FILEDESC cellfile = 0;
    FILE *fp;

    /* now goes from pnmtopng.c* -A.Sh */
    /*
     * ** pnmtopng.c -
     * ** read a portable anymap and produce a Portable Network Graphics file
     * **
     * ** derived from pnmtorast.c (c) 1990,1991 by Jef Poskanzer and some
     * ** parts derived from ppmtogif.c by Marcel Wijkstra <wijkstra@fwi.uva.nl>
     * ** thanks to Greg Roelofs <newt@pobox.com> for contributions and bug-fixes
     * **
     * ** Copyright (C) 1995-1998 by Alexander Lehmann <alex@hal.rhein-main.de>
     * **                        and Willem van Schaik <willem@schaik.com>
     * **
     * ** Permission to use, copy, modify, and distribute this software and its
     * ** documentation for any purpose and without fee is hereby granted, provided
     * ** that the above copyright notice appear in all copies and that both that
     * ** copyright notice and this permission notice appear in supporting
     * ** documentation.  This software is provided "as is" without express or
     * ** implied warranty.
     */



    png_struct *png_ptr;
    png_info *info_ptr;


    png_byte *line;
    png_byte *pp;


    /* these variables are declared static because gcc wasn't kidding
     * about "variable XXX might be clobbered by `longjmp' or `vfork'"
     * (stack corruption observed on Solaris 2.6 with gcc 2.8.1, even
     * in the absence of any other error condition) */
    static int pnm_type;
    static xelval maxmaxval;

    static int depth;
    static int compression;
    static int filter;

    /* these guys are initialized to quiet compiler warnings: */
    maxmaxval = 255;
    depth = 0;



    G_gisinit(argv[0]);

    rast = G_define_option();
    rast->key = "input";
    rast->type = TYPE_STRING;
    rast->required = YES;
    rast->multiple = NO;
    rast->gisprompt = "old,cell,Raster";
    rast->description = "Raster file to be converted.";

    png_file = G_define_option();
    png_file->key = "output";
    png_file->type = TYPE_STRING;
    png_file->required = NO;
    png_file->multiple = NO;
    png_file->answer = "<rasterfilename>.png";
    png_file->description = "Name for new PNG file. (use out=- for stdout)";

    bequiet = G_define_flag();
    bequiet->key = 'q';
    bequiet->description = "Run quietly";

    /* see what can be done to convert'em -A.Sh.
     * gscale = G_define_flag ();
     * gscale->key = 'G';
     * gscale->description = "Output greyscale instead of color";
     */

    module = G_define_module();
    module->description =
	"Export GRASS raster as non-georeferenced PNG image format.";

    if (G_parser(argc, argv))
	exit(-1);

    strncpy(rastermap, rast->answer, 1024);

    if (strcmp(png_file->answer, "<rasterfilename>.png")) {
	if (strcmp(png_file->answer, "-"))
	    strncpy(ofile, png_file->answer, 1000);
	else
	    do_stdout = 1;
    }
    else {
	map = p = rast->answer;
	/* knock off any GRASS location suffix */
	if ((char *)NULL != (p = strrchr(map, '@'))) {
	    if (p != map)
		*p = '\0';
	}
	strncpy(ofile, map, 995);
	strcat(ofile, ".png");
    }

/*G_get_set_window (&w); *//* 10/99 MN: check for current region */
    G_get_window(&w);

    if (!bequiet->answer)
	fprintf(stderr, "rows = %d, cols = %d\n", w.rows, w.cols);

    /* open cell file for reading */
    {
	cellmap = G_find_file2("cell", rastermap, "");
	if (!cellmap) {
	    sprintf(errbuf, "Couldn't find raster file %s", rastermap);
	    G_fatal_error(errbuf);
	}

	if ((cellfile = G_open_cell_old(rast->answer, cellmap)) == -1) {
	    sprintf(errbuf, "Not able to open cellfile for [%s]", rastermap);
	    G_fatal_error(errbuf);
	}
    }

    cell_buf = G_allocate_c_raster_buf();
    fcell_buf = G_allocate_f_raster_buf();
    dcell_buf = G_allocate_d_raster_buf();

    ored = (unsigned char *)G_malloc(w.cols * sizeof(unsigned char));
    ogrn = (unsigned char *)G_malloc(w.cols * sizeof(unsigned char));
    oblu = (unsigned char *)G_malloc(w.cols * sizeof(unsigned char));
    set = (unsigned char *)G_malloc(w.cols * sizeof(unsigned char));

    /* open png file for writing */
    {
	if (do_stdout)
	    fp = stdout;
	else if (NULL == (fp = fopen(ofile, "w"))) {
	    sprintf(errbuf, "Not able to open file for [%s]", ofile);
	    G_fatal_error(errbuf);
	}
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				      &pnmtopng_jmpbuf_struct,
				      pnmtopng_error_handler, NULL);
    if (png_ptr == NULL) {
	fclose(fp);
	fprintf(stderr, "cannot allocate LIBPNG structure\n");
	exit(-1);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
	png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	fclose(fp);
	fprintf(stderr, "cannot allocate LIBPNG structure\n");
	exit(-1);
    }

    if (setjmp(pnmtopng_jmpbuf_struct.jmpbuf)) {
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	fprintf(stderr, "setjmp returns error condition (1)\n");
	exit(-1);
    }

    depth = 8;			/*really??? */

#ifdef OLDPNG
    png_write_init(png_ptr);
    png_info_init(info_ptr);
#endif
    png_init_io(png_ptr, fp);
    info_ptr->width = w.cols;
    info_ptr->height = w.rows;
    info_ptr->bit_depth = depth;

    /* explicit filter-type (or none) required */
    if ((filter >= 0) && (filter <= 4)) {
	png_set_filter(png_ptr, 0, filter);
    }

    /* zlib compression-level (or none) required */
    /* ((compression >= -1) && (compression <= 9)) */
    /* { */
    png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
    /* } */

    if (!bequiet->answer)
	fprintf(stderr, "Converting %s...", rast->answer);

    {
	struct Colors colors;

	G_read_colors(rast->answer, cellmap, &colors);

	rtype = G_raster_map_type(rast->answer, cellmap);
	if (rtype == CELL_TYPE)
	    voidc = (CELL *) cell_buf;
	else if (rtype == FCELL_TYPE)
	    voidc = (FCELL *) fcell_buf;
	else if (rtype == DCELL_TYPE)
	    voidc = (DCELL *) dcell_buf;
	else
	    exit(1);

	/*if(!gscale->answer){ */       /* 24BIT COLOR IMAGE */

	if (1) {

	    info_ptr->color_type = PNG_COLOR_TYPE_RGB;

	    /* write the png-info struct */
	    png_write_info(png_ptr, info_ptr);

	    /* let libpng take care of, e.g., bit-depth conversions */
	    png_set_packing(png_ptr);

	    /* max: 3 color channels, one alpha channel, 16-bit */
	    if ((line = (png_byte *) malloc(w.cols * 8)) == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		G_free_colors(&colors);
		G_free(cell_buf);
		G_free(fcell_buf);
		G_free(dcell_buf);
		G_free(ored);
		G_free(ogrn);
		G_free(oblu);
		G_free(set);
		G_close_cell(cellfile);
		fprintf(stderr, "out of memory allocating PNG row buffer\n");
		exit(-1);
	    }

	    for (row = 0; row < w.rows; row++) {

		if (!bequiet->answer)
		    G_percent(row, w.rows, 5);
		if (G_get_raster_row(cellfile, (void *)voidc, row, rtype) < 0)
		    exit(1);
		G_lookup_raster_colors((void *)voidc, ored, ogrn, oblu, set,
				       w.cols, &colors, rtype);

		pp = line;

		for (col = 0; col < w.cols; col++) {

		    if (set[col]) {
			*pp++ = ored[col];
			*pp++ = ogrn[col];
			*pp++ = oblu[col];
		    }
		    else {
			*pp++ = DEF_RED;
			*pp++ = DEF_GRN;
			*pp++ = DEF_BLU;
		    }


		}

		png_write_row(png_ptr, line);

	    }

	}
	else {			/* GREYSCALE IMAGE */

	    /*    
	     * info_ptr->color_type = PNG_COLOR_TYPE_GRAY;
	     * 
	     */


	    /* pm_message ("don't know yet how to write grey - yumm!!"); */
	    G_warning("don't know how to write grey scale!\n");
	}

	G_free_colors(&colors);

    }
    G_free(cell_buf);
    G_free(fcell_buf);
    G_free(dcell_buf);
    G_free(ored);
    G_free(ogrn);
    G_free(oblu);
    G_free(set);
    G_close_cell(cellfile);



    png_write_end(png_ptr, info_ptr);
    /* png_write_destroy (png_ptr); this is no longer supported with libpng, al 11/2000 */
    /* flush first because free(png_ptr) can segfault due to jmpbuf problems
     * in png_write_destroy */
    fflush(stdout);
    /* free (png_ptr); */
    /* free (info_ptr); */
    png_destroy_write_struct(&png_ptr, &info_ptr);	/* al 11/2000 */


    fclose(fp);

    if (!bequiet->answer)
	fprintf(stderr, "\nDone.\n");

    return (0);
}

#ifdef __STDC__
static void pnmtopng_error_handler(png_structp png_ptr, png_const_charp msg)
#else
static void pnmtopng_error_handler(png_ptr, msg)
    png_structp png_ptr;
    png_const_charp msg;
#endif
{
    jmpbuf_wrapper *jmpbuf_ptr;

    /* this function, aside from the extra step of retrieving the "error
     * pointer" (below) and the fact that it exists within the application
     * rather than within libpng, is essentially identical to libpng's
     * default error handler.  The second point is critical:  since both
     * setjmp() and longjmp() are called from the same code, they are
     * guaranteed to have compatible notions of how big a jmp_buf is,
     * regardless of whether _BSD_SOURCE or anything else has (or has not)
     * been defined. */

    fprintf(stderr, "pnmtopng:  fatal libpng error: %s\n", msg);
    fflush(stderr);

    jmpbuf_ptr = png_get_error_ptr(png_ptr);
    if (jmpbuf_ptr == NULL) {	/* we are completely hosed now */
	fprintf(stderr,
		"pnmtopng:  EXTREMELY fatal error: jmpbuf unrecoverable; terminating.\n");
	fflush(stderr);
	exit(99);
    }

    longjmp(jmpbuf_ptr->jmpbuf, 1);
}
