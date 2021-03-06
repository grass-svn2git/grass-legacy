/*
 * Portions Copyright (c) 1988, 1990 by Sam Leffler.
 * All rights reserved.
 *
 * This file is provided for unrestricted use provided that this
 * legend is included on all tape media and as a part of the
 * software program in whole or part.  Users may copy, modify or
 * distribute this file at will.
 */

#include <stdio.h>
#include "gis.h"
#include "rasterfile.h"
#include "tiffio.h"

#define	howmany(x, y)	(((x)+((y)-1))/(y))
#define	streq(a,b)	(strcmp(a,b) == 0)

u_short	config = PLANARCONFIG_CONTIG;
u_short	compression = -1;
u_short	rowsperstrip = 0;
int DEBUG = 0;

main(argc, argv)
char *argv[];
{
	u_char *buf, *tmpptr;
	int row, linebytes;
	TIFF *out;
	int in;
	struct rasterfile h;
	struct Option *inopt, *outopt;
	struct Flag *cflag, *pflag, *vflag;
	CELL *cell, *cellptr;
	struct Cell_head cellhd;
	int rowg,col,nrows,ncols,count,verbose,i;
	char *mapset;
	struct Colors colors;
	int red, grn, blu, mapsize;

	G_gisinit(argv[0]);

	inopt = G_define_option();
	inopt->key             = "input";
	inopt->type            =  TYPE_STRING;
	inopt->required        =  YES;
	inopt->gisprompt	= "old,cell,raster";
	inopt->description     = "Existing raster file name";

	outopt = G_define_option();
	outopt->key             = "output";
	outopt->type            =  TYPE_STRING;
	outopt->required        =  YES;
	outopt->gisprompt	= "new,tiff,tiff";
	outopt->description     = "File name for new TIFF file.";

	pflag = G_define_flag();
	pflag->key		= 'p';
	pflag->description	= "TIFF Pallete output (8bit instead of 24bit).";

	cflag = G_define_flag();
	cflag->key		= 'c';
	cflag->description	= "Compress with LZW routine.";

	vflag = G_define_flag();
	vflag->key		= 'v';
	vflag->description	= "Verbose mode.";


	if (G_parser (argc, argv))
		exit (-1);

	compression = COMPRESSION_NONE;
	if (cflag->answer) compression = COMPRESSION_LZW;
	verbose = vflag->answer;

	mapset = G_find_cell(inopt->answer, "");
	if (!mapset)
	{
		fprintf(stderr, "%s - raster map not found\n", inopt->answer);
		exit(1);
	}
	if ( (G_get_cellhd(inopt->answer, mapset, &cellhd) < 0)){
		fprintf(stderr, "%s - can't read raster cellhd\n", inopt->answer);
		exit(1);
	}
	if ((G_get_window(&cellhd) < 0))
		G_fatal_error("Can't set window");
	G_read_colors(inopt->answer, mapset, &colors);
	cell = G_allocate_cell_buf();
	if((in = G_open_cell_old (inopt->answer, mapset)) < 0){
		fprintf(stderr, "%s - can't open raster map\n", inopt->answer);
		exit(1);
	}
	out = TIFFOpen(outopt->answer, "w");
	if (out == NULL)
		exit(-4);
	h.ras_width = cellhd.cols;
	h.ras_height = cellhd.rows;
	h.ras_depth = 24;
	if (pflag->answer)
		h.ras_depth = 8;
	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, h.ras_width);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h.ras_height);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, h.ras_depth > 8 ? 3 : 1);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, h.ras_depth > 1 ? 8 : 1);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, config);
	mapsize = 1<<h.ras_depth;

	if (pflag->answer) {
		register u_short *redp, *grnp, *blup, *mapptr;
		register int i, j;

		if (DEBUG)
			printf("max %d min %d mapsize %d\n ",colors.cmax, colors.cmin,mapsize);
		mapptr = (u_short *)malloc(mapsize * 3 * sizeof (u_short));
		redp = mapptr;
		grnp = redp + mapsize;
		blup = redp + mapsize * 2;

		/* XXX -- set pointers up before we step through arrays */
#define	SCALE(x)	(((x)*((1L<<16)-1))/255)
		for (i = 0; i <= colors.cmax;i++,redp++,grnp++,blup++){
			G_get_color(i, &red, &grn, &blu, &colors);
			*redp = (u_short)(SCALE(red));
			*grnp = (u_short)(SCALE(grn));
			*blup = (u_short)(SCALE(blu));
			/*printf(" %d : %d %d %d   %d %d %d\n", i,red,grn,blu, *redp, *grnp, *blup);*/
		}
		if ((i = colors.cmax) < mapsize) {
			i = mapsize - i;
			bzero(redp, i*sizeof (u_short));
			bzero(grnp, i*sizeof (u_short));
			bzero(blup, i*sizeof (u_short));
			redp += i;
			grnp += i;
			blup += i;
		}
		TIFFSetField(out, TIFFTAG_COLORMAP,
		    mapptr, mapptr + mapsize, mapptr + mapsize * 2);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
		if (compression == (u_short)-1)
			compression = COMPRESSION_PACKBITS;
		TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
	} else {
		/* XXX this is bogus... */
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, h.ras_depth == 24 ?
		    PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
		if (compression == (u_short)-1)
			compression = COMPRESSION_LZW;
		TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
	}

	linebytes = ((h.ras_depth*h.ras_width+15) >> 3) &~ 1;
	if (DEBUG)
		printf("linebytes = %d, TIFFscanlinesize = %d\n", linebytes,
		    TIFFScanlineSize(out));
	if (TIFFScanlineSize(out) > linebytes)
		buf = (u_char *)malloc(linebytes);
	else
		buf = (u_char *)malloc(TIFFScanlineSize(out));
	if (rowsperstrip != (u_short)-1)
		rowsperstrip = (u_short)(8*1024/linebytes);
	if (DEBUG)
		printf("rowsperstrip = %d\n",rowsperstrip);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP,
	    rowsperstrip == 0 ? 1 : rowsperstrip);

	if (verbose)
		fprintf (stderr, "%s: complete ... ", G_program_name());
	for (row = 0; row < h.ras_height; row++)
	{
		tmpptr = buf;
		if (verbose)
			G_percent (row, h.ras_height, 2);
		if (G_get_map_row (in, cell, row) < 0)
			exit(1);
		cellptr = cell;
		if (pflag->answer){
			for ( col=0; col < h.ras_width; col++){
				*tmpptr++ = (u_char)*cellptr++;
			}
		} else{
			for ( col=0; col < h.ras_width; col++){
				G_get_color( cell[col], &red, &grn, &blu, &colors);
				*tmpptr++ = (u_char) red;
				*tmpptr++ = (u_char) grn;
				*tmpptr++ = (u_char) blu;
			}
		}
		if (TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}
	if (verbose)
		G_percent (row, h.ras_height, 2);

	(void) TIFFClose(out);
}
