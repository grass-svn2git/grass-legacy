/*
* Name
* 	r.proj -- convert a map to a new geographic projection.
*
* Usage:
*	r.proj input=name location=name [output=name] [mapset=name]
*		  [dbase=name] [method=name]
*
*
* Description:
*	r.proj converts a map to a new geographic projection. It reads a
*	map from a different location, projects it and write it out
*	to the current location. The projected data is resampled with
*	one of three different methods: nearest neighbor, bilinear and
*	cubic convolution.
*
*
* Parameters:
*
*	input   	input raster map
*	location   	location of input map
*	output		output raster map
*	mapset		mapset of input map
*	dbase		database of input map
*	method		interpolation method to use
*
*
*
* Author:
*		 Martin Schroeder
*		 University of Heidelberg
*		 Dept. of Geography
*		 emes@geo0.geog.uni-heidelberg.de
*
* 		 (With the help of a lot of existing GRASS sources, in 
*		  particular v.proj) 
*
* Changes
*		 Morten Hulden <morten@ngb.se>, Aug 2000:
*		 - aborts if input map is outside current location.
*		 - can handle projections (conic, azimuthal etc) where 
*		 part of the map may fall into areas where south is 
*		 upward and east is leftward.
*		 - avoids passing location edge coordinates to PROJ
*		 (they may be invalid in some projections).
*		 - output map will be clipped to borders of the current region.
*		 - output map cell edges and centers will coinside with those 
*		 of the current region.
*		 - output map resolution (unless changed explicitly) will
*		 match (exactly) the resolution of the current region.
*		 - if the input map is smaller than the current region, the 
*		 output map will only cover the overlapping area.
*                - if the input map is larger than the current region, only the
*		 needed amount of memory will be allocated for the projection
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gis.h"
#include "projects.h"
#include "r.proj.h"

extern void set_datumshift(char *, char *, char *, char *);

/* modify this table to add new methods */
struct menu menu[] = {
	{ p_nearest,	"nearest",	"nearest neighbor" },
	{ p_bilinear,	"bilinear",	"bilinear" },
	{ p_cubic,	"cubic",	"cubic convolution" },
	{ NULL,		NULL,		NULL }
};

static char *make_ipol_list(void);

int main (int argc, char **argv)
{
	char     *mapname,		 /* ptr to name of output layer	 */
	         *setname,		 /* ptr to name of input mapset	 */
	         *ipolname,		 /* name of interpolation method */
	          errbuf[256],		 /* buffer for error messages	 */
		 *in_datum,		 /* data and ellipses for datum  */
		 *in_ellipse,		 /* conversion */
		 *out_datum,
		 *out_ellipse;

	int       fdi,			 /* input map file descriptor	 */
	          fdo,			 /* output map file descriptor	 */
	          method,		 /* position of method in table	 */
	          permissions,		 /* mapset permissions		 */
	          cell_type,		 /* output celltype		 */
	          cell_size,		 /* size of a cell in bytes	 */
	          row, col,		 /* counters			 */
		  irows, icols,		 /* original rows, cols		 */
		  orows, ocols,
		  have_colors;     	 /* Input map has a colour table */

	void     *obuffer,		 /* buffer that holds one output row	 */
	         *obufptr;		 /* column ptr in output buffer	 */
	FCELL   **ibuffer;		 /* buffer that holds the input map	 */
	func      interpolate;		 /* interpolation routine	 */

	double    xcoord1, xcoord2,	 /* temporary x coordinates 	 */
	          ycoord1, ycoord2,	 /* temporary y coordinates	 */
	          col_idx,		 /* column index in input matrix */
	          row_idx,		 /* row index in input matrix	 */
		  onorth, osouth,	 /* save original border coords  */
		  oeast, owest,
		  inorth, isouth,
		  ieast, iwest;

	struct Colors colr;		 /* Input map colour table       */
   
	struct pj_info iproj,		 /* input map proj parameters	 */
	          oproj;		 /* output map proj parameters	 */

	struct Key_Value *in_proj_info,	 /* projection information of 	 */
	         *in_unit_info,		 /* input and output mapsets	 */
	         *out_proj_info,
	         *out_unit_info;

	struct GModule *module;

	struct Flag *list,		 /* list files in source location */
		 *nocrop;		 /* don't crop output map	 */

	struct Option *imapset,		 /* name of input mapset	 */
	         *inmap,		 /* name of input layer		 */
                 *inlocation,            /* name of input location       */
                 *outmap,		 /* name of output layer	 */
	         *indbase,		 /* name of input database	 */
	         *interpol,		 /* interpolation method:
 					    nearest neighbor, bilinear, cubic */
		 *res;			 /* resolution of target map     */
		  struct Cell_head incellhd,	 /* cell header of input map	 */
	          outcellhd;		 /* and output map		 */


	G_gisinit(argv[0]);

	module = G_define_module();
	module->description =
		"re-project a raster map from one location to the current location (no datum transformation yet)";


	inmap = G_define_option();
	inmap->key = "input";
	inmap->type = TYPE_STRING;
	inmap->required = YES;
	inmap->description = "input raster map";

	inlocation = G_define_option();
	inlocation->key = "location";
	inlocation->type = TYPE_STRING;
	inlocation->required = YES;
	inlocation->description = "location of input map";

	imapset = G_define_option();
	imapset->key = "mapset";
	imapset->type = TYPE_STRING;
	imapset->required = NO;
	imapset->description = "mapset of input map";

	indbase = G_define_option();
	indbase->key = "dbase";
	indbase->type = TYPE_STRING;
	indbase->required = NO;
	indbase->description = "path to GRASS database of input location";

	outmap = G_define_option();
	outmap->key = "output";
	outmap->type = TYPE_STRING;
	outmap->required = NO;
	outmap->description = "output raster map";

	ipolname = make_ipol_list();

	interpol = G_define_option();
	interpol->key = "method";
	interpol->type = TYPE_STRING;
	interpol->required = NO;
	interpol->answer = "nearest";
	interpol->options = ipolname;
	interpol->description = "interpolation method to use";

	res = G_define_option();
	res->key = "resolution";
	res->type = TYPE_DOUBLE;
	res->required = NO;
	res->description = "resolution of output map";

	list = G_define_flag();
	list->key = 'l';
	list->description = "List raster files in input location and exit";

	nocrop = G_define_flag();
	nocrop->key = 'n';
	nocrop->description = "Don't attempt to crop output map";

	if (G_parser(argc, argv))
		exit(1);

	/* get the method */
	for (method = 0; ipolname = menu[method].name; method++)
		if (strcmp(ipolname, interpol->answer) == 0)
			break;

	if (!ipolname)
	{
		fprintf(stderr, "<%s=%s> unknown %s\n",
			interpol->key, interpol->answer, interpol->key);
		G_usage();
		exit(1);
	}
	interpolate = menu[method].method;

	mapname = outmap->answer ? outmap->answer : inmap->answer;
	setname = imapset->answer ? imapset->answer : G_store(G_mapset());

	if (strcmp(inlocation->answer, G_location()) == 0)
		G_fatal_error("You have to use a different location for input than the current");

	G_get_window(&outcellhd);

	/* Get projection info for output mapset */
	if ((out_proj_info = G_get_projinfo()) == NULL)
		G_fatal_error("Can't get projection info of output map");

	if ((out_unit_info = G_get_projunits()) == NULL)
		G_fatal_error("Can't get projection units of output map");

	if (pj_get_kv(&oproj, out_proj_info, out_unit_info) < 0)
		G_fatal_error("Can't get projection key values of output map");

	out_datum = G_database_datum_name();
	out_datum = out_datum ? G_store(out_datum) : "";

	out_ellipse = G_database_ellipse_name();
	out_ellipse = out_ellipse ? G_store(out_ellipse) : "";

	/* Change the location 		 */
	G__create_alt_env();
	G__setenv("GISDBASE", indbase->answer ? indbase->answer : G_gisdbase());
	G__setenv("LOCATION_NAME", inlocation->answer);

	permissions = G__mapset_permissions(setname);
	if (permissions < 0)	/* can't access mapset 	 */
		G_fatal_error("Mapset [%s] in input location [%s] - %s\n",
			      setname, inlocation->answer,
			      permissions == 0
			      ? "permission denied"
			      : "not found");

	/* if requested, list the raster files in source location - MN 5/2001*/
	if (list->answer)
	{
		if (isatty(0))  /* check if on command line */
			fprintf(stderr, "Checking location %s, mapset %s:\n",
				inlocation->answer, setname);
		G_list_element ("cell", "raster", setname, 0);
		exit(0); /* leave r.proj after listing*/
	}

	if (!G_find_cell(inmap->answer, setname))
		G_fatal_error("Input map [%s] in location [%s] in mapset [%s] not found.",
			      inmap->answer, inlocation->answer, setname);

	/* Read input map colour table */   
	have_colors = G_read_colors(inmap->answer, setname, &colr);
   
	/* Get projection info for input mapset */
	if ((in_proj_info = G_get_projinfo()) == NULL)
		G_fatal_error("Can't get projection info of input map");

	if ((in_unit_info = G_get_projunits()) == NULL)
		G_fatal_error("Can't get projection units of input map");

	if (pj_get_kv(&iproj, in_proj_info, in_unit_info) < 0)
		G_fatal_error("Can't get projection key values of input map");

	/* this call causes r.proj to read the entire map into memeory */
	G_get_cellhd(inmap->answer, setname, &incellhd);
	/* this call causes r.proj to use the WIND file settings */
	/*G_get_window(&incellhd);*/

	G_set_window(&incellhd);
	cell_type = G_raster_map_type(inmap->answer, setname);

	if (G_projection() == PROJECTION_XY)
		G_fatal_error("Can't work with xy data");

	in_datum = G_database_datum_name();
	in_datum = in_datum ? G_store(in_datum) : "";

	in_ellipse = G_database_ellipse_name();
	in_ellipse = in_ellipse ? G_store(in_ellipse) : "";

	/* determine which do_proj function to use */
	set_datumshift(in_datum, in_ellipse, out_datum, out_ellipse);

	/* Save default borders so we can show them later */
	inorth = incellhd.north;
	isouth = incellhd.south;
	ieast = incellhd.east;
	iwest = incellhd.west;
	irows = incellhd.rows;
	icols = incellhd.cols;

	onorth = outcellhd.north;
	osouth = outcellhd.south;
	oeast = outcellhd.east;
	owest = outcellhd.west;
	orows = outcellhd.rows;
	ocols = outcellhd.cols;

	/* Cut non-overlapping parts of input map */
	if (!nocrop->answer)
		bordwalk(&outcellhd, &incellhd, &oproj, &iproj);

	/* Add 2 cells on each side for bilinear/cubic & future interpolation methods */
	/* (should probably be a factor based on input and output resolution) */
	incellhd.north += 2*incellhd.ns_res;
	incellhd.east  += 2*incellhd.ew_res;
	incellhd.south -= 2*incellhd.ns_res;
	incellhd.west  -= 2*incellhd.ew_res;
	if (incellhd.north > inorth) incellhd.north=inorth;
	if (incellhd.east > ieast) incellhd.east=ieast;
	if (incellhd.south < isouth) incellhd.south=isouth;
	if (incellhd.west < iwest) incellhd.west=iwest;

	G_set_window(&incellhd);

	/* And switch back to original location */

	G__switch_env();

	/* Adjust borders of output map */

	if (!nocrop->answer)
		bordwalk(&incellhd, &outcellhd, &iproj, &oproj);

#if 0
	outcellhd.west=outcellhd.south=HUGE_VAL;
	outcellhd.east=outcellhd.north=-HUGE_VAL;
	for(row=0;row<incellhd.rows;row++)
	{
		ycoord1=G_row_to_northing((double)(row+0.5),&incellhd);
		for(col=0;col<incellhd.cols;col++)
		{
			xcoord1=G_col_to_easting((double)(col+0.5),&incellhd);
			proj_f(&xcoord1,&ycoord1,&iproj,&oproj);
			if(xcoord1>outcellhd.east)outcellhd.east=xcoord1;
			if(ycoord1>outcellhd.north)outcellhd.north=ycoord1;
			if(xcoord1<outcellhd.west)outcellhd.west=xcoord1;
			if(ycoord1<outcellhd.south)outcellhd.south=ycoord1;
		}
	}
#endif

	if (res->answer != NULL)   /* set user defined resolution */
		outcellhd.ns_res = outcellhd.ew_res = atof(res->answer);

	G_adjust_Cell_head(&outcellhd, 0, 0);
	G_set_window(&outcellhd);

	fprintf(stderr, "Input:\n");
	fprintf(stderr, "Cols:	%d (%d)\n", incellhd.cols, icols);
	fprintf(stderr, "Rows:	%d (%d)\n", incellhd.rows, irows);
	fprintf(stderr, "North: %f (%f)\n", incellhd.north, inorth);
	fprintf(stderr, "South: %f (%f)\n", incellhd.south, isouth);
	fprintf(stderr, "West:  %f (%f)\n", incellhd.west, iwest);
	fprintf(stderr, "East:  %f (%f)\n", incellhd.east, ieast);
	fprintf(stderr, "ew-res:	%f\n", incellhd.ew_res);
	fprintf(stderr, "ns-res:	%f\n", incellhd.ns_res);
	fprintf(stderr, "\n");

	fprintf(stderr, "Output:\n");
	fprintf(stderr, "Cols:	%d (%d)\n", outcellhd.cols, ocols);
	fprintf(stderr, "Rows:	%d (%d)\n", outcellhd.rows, orows);
	fprintf(stderr, "North: %f (%f)\n", outcellhd.north, onorth);
	fprintf(stderr, "South: %f (%f)\n", outcellhd.south, osouth);
	fprintf(stderr, "West:  %f (%f)\n", outcellhd.west, owest);
	fprintf(stderr, "East:  %f (%f)\n", outcellhd.east, oeast);
	fprintf(stderr, "ew-res:	%f\n", outcellhd.ew_res);
	fprintf(stderr, "ns-res:	%f\n", outcellhd.ns_res);

	/* open and read the relevant parts of the input map and close it */
	G__switch_env();
	G_set_window(&incellhd);
	fdi = G_open_cell_old(inmap->answer, setname);
	ibuffer = (FCELL **) readcell(fdi);
	G_close_cell(fdi);

	G__switch_env();
	G_set_window(&outcellhd);

	if (strcmp(interpol->answer, "nearest") == 0)
	{
		fdo = G_open_raster_new(mapname, cell_type);
		obuffer = (CELL *) G_allocate_raster_buf(cell_type);
	}
	else
	{
		fdo = G_open_fp_cell_new(mapname);
		cell_type = FCELL_TYPE;
		obuffer = (FCELL *) G_allocate_raster_buf(cell_type);
	}

	cell_size = G_raster_size(cell_type);

	xcoord1 = xcoord2 = outcellhd.west + (outcellhd.ew_res / 2);	/**/
	ycoord1 = ycoord2 = outcellhd.north - (outcellhd.ns_res / 2);	/**/

	/* now invert the sense of the projection */
	INVERSE_FLAG = !INVERSE_FLAG;

	fprintf(stderr, "Projecting... ");
	G_percent(0, outcellhd.rows, 2);

	for (row = 0; row < outcellhd.rows; row++)
	{
		obufptr = obuffer;

		for (col = 0; col < outcellhd.cols; col++)
		{
			/* project coordinates in output matrix to	 */
			/* coordinates in input matrix			 */
			if (proj_f(&xcoord1, &ycoord1, &oproj, &iproj) < 0)
				G_set_null_value(obufptr, 1, cell_type);
			else
			{
				/* convert to row/column indices of input matrix */
				col_idx = (xcoord1 - incellhd.west) / incellhd.ew_res;
				row_idx = (incellhd.north - ycoord1) / incellhd.ns_res;

				/* and resample data point		 */
				interpolate(ibuffer, obufptr, cell_type,
					    &col_idx, &row_idx, &incellhd);
			}

			obufptr = G_incr_void_ptr(obufptr, cell_size);
			xcoord2 += outcellhd.ew_res;
			xcoord1 = xcoord2;
			ycoord1 = ycoord2;
		}

		if (G_put_raster_row(fdo, obuffer, cell_type) < 0)
			G_fatal_error("error writing output map");

		xcoord1 = xcoord2 = outcellhd.west;
		ycoord2 -= outcellhd.ns_res;
		ycoord1 = ycoord2;
		G_percent(row, outcellhd.rows - 1, 2);
	}

	G_close_cell(fdo);
   
	if(have_colors > 0)
	{    
		G_write_colors(mapname, G_mapset(), &colr);
		G_free_colors(&colr);
	}
   
	return 0;		/* OK */
}

static char *make_ipol_list(void)
{
	int size = 0;
	int i;
	char *buf;

	for (i = 0; menu[i].name; i++)
		size += strlen(menu[i].name) + 1;

	buf = G_malloc(size);
	*buf = '\0';

	for (i = 0; menu[i].name; i++)
	{
		if (i) strcat(buf, ",");
		strcat(buf, menu[i].name);
	}

	return buf;
}

