/*
 * r.in.tiff - Converts from a Tagged Image File Format image to a Grass Raster.
 *
 * tif2ras.c - Converts from a Tagged Image File Format image to a Sun Raster.
 * Portions Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 *   This program takes a MicroSoft/Aldus "Tagged Image File Format" image or
 * "TIFF" file as input and writes a GRASS cell file.
 */
/*typedef unsigned short u_short;
typedef unsigned long u_long;
typedef unsigned char u_char;
	*/

#include <stdio.h>
#include <sys/types.h>
#include "tiffio.h"
#include "gis.h"

typedef struct {
	int	type;
	int	length;
	u_char	*map[3];
} colormap_t;

typedef int boolean;
#define RMT_NONE 0
#define RMT_EQUAL_RGB 1
#define RT_BYTE_ENCODED 2

#define	CVT(x)		((uint16)(((x) * 255) / ((1L<<16)-1)))

boolean     Verbose;
char       *pname;		/* program name (used for error messages) */
static	char  *inf = NULL ;
static	char  *outf = NULL ;

main(argc, argv)
int         argc;
char       *argv[];
{
	int         depth,
	ret,
	numcolors;
	register TIFF *tif;
	register u_char *inp, *outp, tmp;
	register int col,
	row,
	i;
	u_char     *Map = NULL;
	u_char     *buf;
	uint16 *redcmap, *greencmap, *bluecmap;

	colormap_t  Colormap;	/* The Pixrect Colormap */
	u_char      red[256],
		    green[256],
		    blue[256];

	int cellfp;
	CELL *cell;
	struct Cell_head cellhd;
	struct Colors cellcolor;
	int cellrow, cellcol, cellnrows, cellncols, rowpass;
	struct Option *inopt, *outopt;
	struct Flag *vflag;

	u_short	bitspersample,
		samplesperpixel,
		photometric;
	u_long	width,
		height;

	G_gisinit(argv[0]);
	inopt = G_define_option();
	inopt->key		= "input";
	inopt->type		= TYPE_STRING;
	inopt->required		= YES;
	inopt->description	= "Name on TIFF file to input.";

	outopt = G_define_option();
	outopt->key		= "output";
	outopt->type		= TYPE_STRING;
	outopt->required	= YES;
	outopt->gisprompt	= "new,cell,raster";
	outopt->description	= "Name of new raster file.";

	vflag = G_define_flag();
	vflag->key		= 'v';
	vflag->description	= "Verbose mode on.";

	if(G_parser(argc, argv))
		exit(-1);

	Verbose = vflag->answer ;

	inf = inopt->answer;
	tif = TIFFOpen(inf, "r");
	if (tif == NULL)
		G_fatal_error("Error opening TIFF file.");
	if (Verbose)
		fprintf(stderr, "Reading %s...", inf);

	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	if (Verbose)
		TIFFPrintDirectory(tif, stderr, 0l);
	if (bitspersample > 8)
		G_fatal_error("Can't handle more than 8-bits per sample");

	switch (samplesperpixel) {
	case 1:
		if (bitspersample == 1)
			depth = 1;
		else
			depth = 8;
		break;
	case 3:
	case 4:
		depth = 24;
		break;
	default:
		G_fatal_error("Only handle 1-channel gray scale or 3-channel color");
	}

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,&width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH,&height);

	cellhd.zone = G_zone();
	cellhd.proj = G_projection();
	cellnrows = cellhd.rows = cellhd.north = height;
	cellncols = cellhd.cols = cellhd.east = width;
	cellhd.south = cellhd.west = 0.0;
	cellhd.ns_res = cellhd.ew_res = 1;
	cellhd.format = 0;
	cellhd.compressed = 0;
	if(G_set_window(&cellhd) < 0)
		G_fatal_error("couldn't set cellhd.");
	cell = G_allocate_cell_buf();
	outf = outopt->answer;
	G_set_cell_format(0);
	if (!(cellfp = G_open_cell_new_random( outf )))
		G_fatal_error("New raster file couldn't be opened for writing.");
	G_init_colors(&cellcolor);

	if (Verbose)
		fprintf(stderr, "%dx%dx%d image\n ", width, height, depth);
	if (Verbose)
		fprintf(stderr, "%d bits/sample, %d samples/pixel \n",
		    bitspersample, samplesperpixel);
	numcolors = (1 << bitspersample);
		TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
	if (numcolors == 2) {
		if (Verbose)
			fprintf(stderr, "monochrome\n ");
		G_set_color((CELL)(0), 0, 0, 0, &cellcolor);
		G_set_color((CELL)(1), 255, 255, 255, &cellcolor);
		Colormap.map[0] = Colormap.map[1] = Colormap.map[2] = NULL;
	} else {
		switch (photometric) {
		case PHOTOMETRIC_MINISBLACK:
			if (Verbose)
				fprintf(stderr, "%d graylevels (min=black)\n ", numcolors);
			Map = (u_char *) malloc(numcolors * sizeof(u_char));
			for (i = 0; i < numcolors; i++){
				Map[i] = (255 * i) / numcolors;
				G_set_color((CELL)(i), Map[i], Map[i], Map[i], &cellcolor);
			}
			break;
		case PHOTOMETRIC_MINISWHITE:
			if (Verbose)
				fprintf(stderr, "%d graylevels (min=white)\n ", numcolors);
			Map = (u_char *) malloc(numcolors * sizeof(u_char));
			for (i = 0; i < numcolors; i++){
				Map[i] = 255 - ((255 * i) / numcolors);
				G_set_color((CELL)(i), Map[i], Map[i], Map[i], &cellcolor);
			}
			break;
		case PHOTOMETRIC_RGB:
			if (Verbose)
				fprintf(stderr, "%d truecolor NOT SUPPORTED YET\n", numcolors);
			/* Colormap.type = RMT_NONE;
	    Colormap.length = 0;
	    Colormap.map[0] = Colormap.map[1] = Colormap.map[2] = NULL;*/
			break;
		case PHOTOMETRIC_PALETTE:
			if (Verbose)
				fprintf(stderr, " %d colormapped\n ", numcolors);
			memset(red, 0, sizeof(red));
			memset(green, 0, sizeof(green));
			memset(blue, 0, sizeof(blue));
			if (!TIFFGetField(tif,TIFFTAG_COLORMAP,&redcmap,
					&greencmap,&bluecmap)
			) {
				G_fatal_error("Missing required \"Colormap\"");
				break;
			}
			for (i = 0; i < numcolors; i++) {
				red[i] = CVT(redcmap[i]);
				green[i] = CVT(greencmap[i]);
				blue[i] = CVT(bluecmap[i]);
				/*fprintf(stderr, "colors %d %d %d %d %d\n", i,red[i],green[i],blue[i],td->bluecmap[i]);*/
				G_set_color((CELL)(i), red[i], green[i], blue[i], &cellcolor);
			}
			break;
		case PHOTOMETRIC_MASK:
			G_fatal_error("Don't know how to handle PHOTOMETRIC_MASK");
			break;
		default:
			G_fatal_error("Unknown photometric.");
		}
	}

	buf = (u_char *) malloc(TIFFScanlineSize(tif));
	if (buf == NULL)
		G_fatal_error("Can't allocate memory for scanline buffer...");

	for (row = 0; row < height; row++) {
		if (TIFFReadScanline(tif, buf, row, 0) < 0){
			fprintf(stderr, "Bad data read on line: %d\n", row);
			exit(-1);
		}
		inp = buf;

		/*outp = (u_char *) mprd_addr(mpr_d(pix), 0, row);*/

		switch (photometric) {
		case PHOTOMETRIC_RGB:
			/*    if (samplesperpixel == 4)
		for (col = 0; col < width; col++) {
		    *outp++ = *inp++;	/* Blue */
			/*	    *outp++ = *inp++;	/* Green */
			/*	    *outp++ = *inp++;	/* Red */
			/*	    inp++;	/* skip alpha channel */
			/*	}
	    else
		for (col = 0; col < width; col++) {
		    *outp++ = *inp++;	/* Blue */
			/*	    *outp++ = *inp++;	/* Green */
			/*	    *outp++ = *inp++;	/* Red */
			/*	}*/
			break;
		case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
			switch (bitspersample) {
			case 1: {
				int	sz_col = width/8;

				if ( width%8 ) ++sz_col;
				cellcol = 0;
				for (col=0; col<sz_col; col++){
				   if ( cellcol<width ) {
				      cell[cellcol++] = (CELL)((*inp >> 7) & 0x01);
				      if ( cellcol<width ) {
					 cell[cellcol++] = (CELL)((*inp >> 6) & 0x01);
					 if ( cellcol<width ) {
					    cell[cellcol++] = (CELL)((*inp >> 5) & 0x01);
					    if ( cellcol<width ) {
					       cell[cellcol++] = (CELL)((*inp >> 4) & 0x01);
					       if ( cellcol<width ) {
						  cell[cellcol++] = (CELL)((*inp >> 3) & 0x01);
						  if ( cellcol<width ) {
						     cell[cellcol++] = (CELL)((*inp >> 2) & 0x01);
						     if ( cellcol<width ) {
							cell[cellcol++] = (CELL)((*inp >> 1) & 0x01);
							if ( cellcol<width )
							   cell[cellcol++] = (CELL)(*inp++ & 0x01);
						     }
						  }
					       }
					    }
					 }
				      }
				   }
				}
				G_put_map_row_random(cellfp, cell, row, 0, width);
				/* *outp++ = *inp++;*/
				break;
			}
			case 2:
				for (col = 0; col < ((width + 3) / 4); col++) {
					*outp++ = (*inp >> 6) & 3;
					*outp++ = (*inp >> 4) & 3;
					*outp++ = (*inp >> 2) & 3;
					*outp++ = *inp++ & 3;
				}
				break;
			case 4:
				cellcol = 0;
				for (col = 0; col < width / 2; col++) {
					/* *outp++ = *inp >> 4;
		    *outp++ = *inp++ & 0xf;*/
					cell[cellcol++] = (CELL) (*inp >> 4);
					cell[cellcol++] = (CELL) (*inp++ & 0xf);
				}
				G_put_map_row_random(cellfp, cell, row, 0, width);
				break;
			case 8:
				for (col = 0; col < width; col++)
					cell[col] = (CELL) *inp++;
				G_put_map_row_random(cellfp, cell, row, 0, width);
				break;
			default:
				fprintf(stderr, "%s: bad bits/sample: %d\n", bitspersample);
				exit(-1);
			}
			break;
		case PHOTOMETRIC_PALETTE:
			for (col = 0; col < width; col++)
				cell[col] = (CELL) *inp++;
			G_put_map_row_random(cellfp, cell, row, 0, width);
			break;
		default:
			fprintf(stderr, "%s: unknown photometric (write): %d\n", photometric);
			exit(-1);
		}
	}

	free((char *) buf);

	if (Verbose)
		fprintf(stderr, "Creating SUPPORT Files for %s\n", outf);
	G_close_cell( cellfp );
	G_write_colors( outf, G_mapset(), &cellcolor);


	if (Verbose)
		fprintf(stderr, "done.\n");

	exit(0);
}
