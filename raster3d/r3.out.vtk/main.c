
/****************************************************************************
*
* MODULE:       r3.out.vtk  
*   	    	
* AUTHOR(S):    Original author 
*               Soeren Gebbert soerengebbert at gmx de
* 		27 Feb 2006 Berlin
* PURPOSE:      Converts 3D raster maps (G3D) into the VTK-Ascii format  
*
* COPYRIGHT:    (C) 2005 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/G3d.h>
#include <grass/glocale.h>
#include "globalDefs.h"
#include "writeVTKData.h"
#include "writeVTKHead.h"
#include "errorHandling.h"

#define MAIN
#include "parameters.h"


/** prototypes ***************************************************************/

/*Open the rgb voxel maps and write the data to the output */
void OpenWriteRGBMaps(inputMaps * in, G3D_Region region, FILE * fp, int dp);

/*Open the rgb voxel maps and write the data to the output */
void OpenWriteVectorMaps(inputMaps * in, G3D_Region region, FILE * fp, int dp);

/*opens a raster input map */
int OpenInputMap(char *name, char *mapset);

/*Check if all maps are available */
void CheckInputMaps();

/*Initiate the input maps structure */
inputMaps *CreateInputMapsStruct();



/* ************************************************************************* */
/* Open the raster input map *********************************************** */
/* ************************************************************************* */
inputMaps *CreateInputMapsStruct()
{
    inputMaps *in;

    in = (inputMaps *) calloc(1, sizeof(inputMaps));

    in->map = NULL;
    in->map_r = NULL;
    in->map_g = NULL;
    in->map_b = NULL;
    in->map_x = NULL;
    in->map_y = NULL;
    in->map_z = NULL;

    in->top = -1;
    in->bottom = -1;

    in->topMapType = 0;
    in->bottomMapType = 0;

    in->elevmaps = NULL;
    in->elevmaptypes = NULL;
    in->numelevmaps = 0;

    return in;
}

/* ************************************************************************* */
/* Open the raster input map *********************************************** */
/* ************************************************************************* */
int OpenInputMap(char *name, char *mapset)
{
    int fd;

    G_debug(3, "Open Raster file %s in Mapset %s", name, mapset);


    /* open raster file */
    fd = G_open_cell_old(name, mapset);

    if (fd < 0)
	G_fatal_error(_("Could not open map %s"), name);


    return fd;
}


/* ************************************************************************* */
/* Check the input maps **************************************************** */
/* ************************************************************************* */
void CheckInputMaps()
{
    int i = 0;
    char *mapset, *name;

    /*Check top and bottom if surface is requested */
    if (param.structgrid->answer) {

	if (!param.top->answer || !param.bottom->answer)
	    G3d_fatalError(_("You have to specify top and bottom map\n"));

	mapset = NULL;
	name = NULL;
	name = param.top->answer;
	mapset = G_find_cell2(name, "");
	if (mapset == NULL) {
	    G3d_fatalError(_("top cell map <%s> not found\n"),
			   param.top->answer);
	}

	mapset = NULL;
	name = NULL;
	name = param.bottom->answer;
	mapset = G_find_cell2(name, "");
	if (mapset == NULL) {
	    G3d_fatalError(_("bottom cell map <%s> not found\n"),
			   param.bottom->answer);
	}
    }

    /*If input maps are provided, check them */
    if (param.input->answers != NULL) {
	for (i = 0; param.input->answers[i] != NULL; i++) {
	    if (NULL == G_find_grid3(param.input->answers[i], ""))
		G3d_fatalError(_("Requested g3d data map <%s> not found"),
			       param.input->answers[i]);
	}
    }

    /*Check for rgb maps. */
    if (param.rgbmaps->answers != NULL) {
	for (i = 0; i < 3; i++) {
	    if (param.rgbmaps->answers[i] != NULL) {
		if (NULL == G_find_grid3(param.rgbmaps->answers[i], ""))
		    G3d_fatalError(_("Requested g3d RGB map <%s> not found"),
				   param.rgbmaps->answers[i]);
	    }
	    else {
		G3d_fatalError(_("Please provide three g3d RGB maps"));
	    }
	}
    }

    /*Check for vector maps. */
    if (param.vectormaps->answers != NULL) {
	for (i = 0; i < 3; i++) {
	    if (param.vectormaps->answers[i] != NULL) {
		if (NULL == G_find_grid3(param.vectormaps->answers[i], ""))
		    G3d_fatalError(_("Requested g3d vector map <%s> not found"),
				   param.vectormaps->answers[i]);
	    }
	    else {
		G3d_fatalError(_
			       ("Please provide three g3d vector maps [x,y,z]"));
	    }
	}
    }

    if (param.input->answers == NULL && param.rgbmaps->answers == NULL &&
	param.vectormaps->answers == NULL) {
	G_warning(_
		  ("No g3d data, RGB or xyz-vector maps are provided! Will only write the geometry."));
    }

    return;
}


/* ************************************************************************* */
/* Prepare the VTK RGB voxel data for writing ****************************** */
/* ************************************************************************* */
void OpenWriteRGBMaps(inputMaps * in, G3D_Region region, FILE * fp, int dp)
{
    int i, changemask[3] = { 0, 0, 0 };
    void *maprgb = NULL;

    if (param.rgbmaps->answers != NULL) {

	/*Loop over all input maps! */
	for (i = 0; i < 3; i++) {
	    G_debug(3, _("Open rgb g3d raster file %s"),
		    param.rgbmaps->answers[i]);

	    maprgb = NULL;
	    /*Open the map */
	    maprgb =
		G3d_openCellOld(param.rgbmaps->answers[i],
				G_find_grid3(param.rgbmaps->answers[i], ""),
				&region, G3D_TILE_SAME_AS_FILE,
				G3D_USE_CACHE_DEFAULT);
	    if (maprgb == NULL) {
		G_warning(_("Error opening g3d file <%s>"),
			  param.rgbmaps->answers[i]);
		fatalError(_("No RGB Data will be created."), in);
	    }

	    /*if requested set the Mask on */
	    if (param.mask->answer) {
		if (G3d_maskFileExists()) {
		    changemask[i] = 0;
		    if (G3d_maskIsOff(maprgb)) {
			G3d_maskOn(maprgb);
			changemask[i] = 1;
		    }
		}
	    }

	    if (i == 0)
		in->map_r = maprgb;
	    if (i == 1)
		in->map_g = maprgb;
	    if (i == 2)
		in->map_b = maprgb;
	}


	G_debug(3, _("Writing VTK VoxelData\n"));
	writeVTKRGBVoxelData(in->map_r, in->map_g, in->map_b, fp, "RGB_Voxel",
			     region, dp);

	for (i = 0; i < 3; i++) {
	    if (i == 0)
		maprgb = in->map_r;
	    if (i == 1)
		maprgb = in->map_g;
	    if (i == 2)
		maprgb = in->map_b;

	    /*We set the Mask off, if it was off before */
	    if (param.mask->answer) {
		if (G3d_maskFileExists())
		    if (G3d_maskIsOn(maprgb) && changemask[i])
			G3d_maskOff(maprgb);
	    }
	    /* Close the g3d map */
	    if (!G3d_closeCell(maprgb)) {
		fatalError(_("Error closing g3d rgb map."), in);
	    }

	    /*Set the pointer to null so we noe later that these files are already closed */
	    if (i == 0)
		in->map_r = NULL;
	    if (i == 1)
		in->map_g = NULL;
	    if (i == 2)
		in->map_b = NULL;
	}
    }
    return;
}

/* ************************************************************************* */
/* Prepare the VTK vector data for writing ********************************* */
/* ************************************************************************* */
void OpenWriteVectorMaps(inputMaps * in, G3D_Region region, FILE * fp, int dp)
{
    int i, changemask[3] = { 0, 0, 0 };
    void *mapvect = NULL;

    if (param.vectormaps->answers != NULL) {

	/*Loop over all input maps! */
	for (i = 0; i < 3; i++) {
	    G_debug(3, _("Open vector g3d raster file %s"),
		    param.vectormaps->answers[i]);

	    mapvect = NULL;
	    /*Open the map */
	    mapvect =
		G3d_openCellOld(param.vectormaps->answers[i],
				G_find_grid3(param.vectormaps->answers[i], ""),
				&region, G3D_TILE_SAME_AS_FILE,
				G3D_USE_CACHE_DEFAULT);
	    if (mapvect == NULL) {
		G_warning(_("Error opening g3d file <%s>"),
			  param.vectormaps->answers[i]);
		fatalError(_("No vector data will be created."), in);
	    }

	    /*if requested set the Mask on */
	    if (param.mask->answer) {
		if (G3d_maskFileExists()) {
		    changemask[i] = 0;
		    if (G3d_maskIsOff(mapvect)) {
			G3d_maskOn(mapvect);
			changemask[i] = 1;
		    }
		}
	    }

	    if (i == 0)
		in->map_x = mapvect;
	    if (i == 1)
		in->map_y = mapvect;
	    if (i == 2)
		in->map_z = mapvect;
	}


	G_debug(3, _("Writing VTK Vector Data\n"));
	writeVTKVectorData(in->map_x, in->map_y, in->map_z, fp, "Vector_Data",
			   region, dp);

	for (i = 0; i < 3; i++) {
	    if (i == 0)
		mapvect = in->map_x;
	    if (i == 1)
		mapvect = in->map_y;
	    if (i == 2)
		mapvect = in->map_z;

	    /*We set the Mask off, if it was off before */
	    if (param.mask->answer) {
		if (G3d_maskFileExists())
		    if (G3d_maskIsOn(mapvect) && changemask[i])
			G3d_maskOff(mapvect);
	    }

	    /* Close the g3d map */
	    if (!G3d_closeCell(mapvect)) {
		fatalError(_("Error closing g3d vector map."), in);
	    }
	    /*Set the pointer to null so we noe later that these files are already closed */
	    if (i == 0)
		in->map_x = NULL;
	    if (i == 1)
		in->map_y = NULL;
	    if (i == 2)
		in->map_z = NULL;
	}
    }
    return;
}


/* ************************************************************************* */
/* Main function, opens most of the input and output files ***************** */
/* ************************************************************************* */
int main(int argc, char *argv[])
{
    char *output = NULL;
    G3D_Region region;
    struct Cell_head window2d;
    FILE *fp = NULL;
    struct GModule *module;
    int dp, i, changemask = 0;
    int rows, cols;
    char *mapset, *name;

    inputMaps *in;

    /* Initialize GRASS */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->description =
	_("Converts 3D raster maps (G3D) into the VTK-Ascii format");

    /* Get parameters from user */
    SetParameters();

    /* Have GRASS get inputs */
    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);
    /*The precision of the output */
    if (param.decimals->answer) {
	if (sscanf(param.decimals->answer, "%d", &dp) != 1)
	    G_fatal_error(_("failed to interpret dp as an integer"));
	if (dp > 20 || dp < 0)
	    G_fatal_error(_("dp has to be from 0 to 20"));
    }
    else {
	dp = 8;			/*This value is taken from the lib settings in G_format_easting */
    }

    /*Check the input */
    CheckInputMaps();

    /*open the output */
    if (param.output->answer) {
	fp = fopen(param.output->answer, "w");
	if (fp == NULL) {
	    perror(param.output->answer);
	    G_usage();
	    exit(EXIT_FAILURE);
	}
    }
    else
	fp = stdout;

    /* Figure out the region from the map */
    G3d_initDefaults();
    G3d_getWindow(&region);

    /*initiate the input mpas structure */
    in = CreateInputMapsStruct();


    /*Open the top and bottom file */
    if (param.structgrid->answer) {

	/*Check if the g3d-region is equal to the 2d rows and cols */
	rows = G_window_rows();
	cols = G_window_cols();

	/*If not equal, set the 2D windows correct */
	if (rows != region.rows || cols != region.cols) {
	    G_message
		("The 2d and 3d region settings are different. The g3d settings are used to adjust the 2d region.");
	    G_get_set_window(&window2d);
	    window2d.ns_res = region.ns_res;
	    window2d.ew_res = region.ew_res;
	    window2d.rows = region.rows;
	    window2d.cols = region.cols;
	    G_set_window(&window2d);
	}

	/*open top */
	mapset = NULL;
	name = NULL;
	name = param.top->answer;
	mapset = G_find_cell2(name, "");
	in->top = OpenInputMap(name, mapset);
	in->topMapType = G_raster_map_type(name, mapset);

	/*open bottom */
	mapset = NULL;
	name = NULL;
	name = param.bottom->answer;
	mapset = G_find_cell2(name, "");
	in->bottom = OpenInputMap(name, mapset);
	in->bottomMapType = G_raster_map_type(name, mapset);

	/* Write the vtk-header and the points */
	if (param.point->answer) {
	    writeVTKStructuredGridHeader(fp, output, region);
	    writeVTKPoints(in, fp, region, dp, 1);
	}
	else {
	    writeVTKUnstructuredGridHeader(fp, output, region);
	    writeVTKPoints(in, fp, region, dp, 0);
	    writeVTKUnstructuredGridCells(fp, region);
	}

	if (G_close_cell(in->top) < 0) {
	    G_fatal_error(_("unable to close top raster map\n"));
	}
	in->top = -1;

	if (G_close_cell(in->bottom) < 0) {
	    G_fatal_error(_("unable to close bottom raster map\n"));
	}
	in->bottom = -1;
    }
    else {
	/* Write the structured point vtk-header */
	writeVTKStructuredPointHeader(fp, output, region, dp);
    }

    /*Write the normal VTK data (cell or point data) */
    /*Loop over all 3d input maps! */
    if (param.input->answers != NULL) {
	for (i = 0; param.input->answers[i] != NULL; i++) {

	    G_debug(3, _("Open g3d raster file %s"), param.input->answers[i]);

	    /*Open the map */
	    in->map =
		G3d_openCellOld(param.input->answers[i],
				G_find_grid3(param.input->answers[i], ""),
				&region, G3D_TILE_SAME_AS_FILE,
				G3D_USE_CACHE_DEFAULT);
	    if (in->map == NULL) {
		G_warning(_("Error opening g3d file <%s>"),
			  param.input->answers[i]);
		fatalError(" ", in);
	    }

	    /*if requested set the Mask on */
	    if (param.mask->answer) {
		if (G3d_maskFileExists()) {
		    changemask = 0;
		    if (G3d_maskIsOff(in->map)) {
			G3d_maskOn(in->map);
			changemask = 1;
		    }
		}
	    }

	    /* Write the point or cell data */
	    writeVTKData(fp, in->map, region, param.input->answers[i], dp);

	    /*We set the Mask off, if it was off before */
	    if (param.mask->answer) {
		if (G3d_maskFileExists())
		    if (G3d_maskIsOn(in->map) && changemask)
			G3d_maskOff(in->map);
	    }

	    /* Close the g3d map */
	    if (!G3d_closeCell(in->map)) {
		in->map = NULL;
		fatalError(_
			   ("Error closing g3d data map, the VTK file may be incomplete."),
			   in);
	    }

	    in->map = NULL;
	}
    }

    /*Write the RGB voxel data */
    OpenWriteRGBMaps(in, region, fp, dp);
    OpenWriteVectorMaps(in, region, fp, dp);

    /*Close the output file */
    if (param.output->answer && fp != NULL)
	if (fclose(fp))
	    fatalError(_("Error closing VTK-ASCII file"), in);

    /*close all open maps and free memory */
    ReleaseInputMapsStruct(in);

    return 0;
}
