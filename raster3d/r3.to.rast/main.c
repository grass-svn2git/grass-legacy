
/****************************************************************************
*
* MODULE:       r3.to.rast 
*   	    	
* AUTHOR(S):    Original author 
*               Soeren Gebbert soerengebbert@gmx.de
* 		08 01 2005 Berlin
* PURPOSE:      Converts 3D raster maps to 2D raster maps  
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
#include "gis.h"
#include "G3d.h"
#include "glocale.h"


/*- Parameters and global variables -----------------------------------------*/
typedef struct
{
    struct Option *input, *output;
    struct Flag *mask;
} paramType;

paramType param;		/*Parameters */


/*- prototypes --------------------------------------------------------------*/
void fatalError(void *map, int *fd, int depths, char *errorMsg);	/*Simple Error message */
void setParams();		/*Fill the paramType structure */
void G3dToRaster(void *map, G3D_Region region, int *fd);	/*Write the raster */
int open_output_map(const char *name, int res_type);	/*opens the outputmap */
void close_output_map(int fd);	/*close the map */



/* ************************************************************************* */
/* Error handling ********************************************************** */
/* ************************************************************************* */
void fatalError(void *map, int *fd, int depths, char *errorMsg)
{
    int i;

    /* Close files and exit */
    if (map != NULL) {
	if (!G3d_closeCell(map))
	    G3d_fatalError(_("Could not close the map"));
    }

    if (fd != NULL) {
	for (i = 0; i < depths; i++)
	    G_unopen_cell(fd[i]);
    }

    G3d_fatalError(errorMsg);
    exit(EXIT_FAILURE);

}


/* ************************************************************************* */
/* Set up the arguments we are expexting ********************************** */
/* ************************************************************************* */
void setParams()
{
    param.input = G_define_option();
    param.input->key = "input";
    param.input->type = TYPE_STRING;
    param.input->required = YES;
    param.input->gisprompt = "old,grid3,3d-raster";
    param.input->description =
	_("3dcell map(s) to be converted to 2D raster slices");

    param.output = G_define_option();
    param.output->key = "output";
    param.output->type = TYPE_STRING;
    param.output->required = YES;
    param.output->description = _("Basename for resultant raster slice maps");
    param.output->gisprompt = "new,cell,raster";

    param.mask = G_define_flag();
    param.mask->key = 'm';
    param.mask->description = _("Use G3D mask (if exists) with input map");
}



/* ************************************************************************* */
/* Write the slices to seperate raster maps ******************************** */
/* ************************************************************************* */
void G3dToRaster(void *map, G3D_Region region, int *fd)
{
    double d1 = 0, f1 = 0;
    int x, y, z, check = 0;
    int rows, cols, depths, typeIntern, pos = 0;
    FCELL *fcell = NULL;
    DCELL *dcell = NULL;

    rows = region.rows;
    cols = region.cols;
    depths = region.depths;


    G_debug(2, _("G3dToRaster: Writing %i raster maps with rows %i cols."),
	    depths, rows, cols);

    typeIntern = G3d_tileTypeMap(map);

    if (typeIntern == G3D_FLOAT)
	fcell = G_allocate_f_raster_buf();
    else if (typeIntern == G3D_DOUBLE)
	dcell = G_allocate_d_raster_buf();

    pos = 0;
    /*Every Rastermap */
    for (z = depths - 1; z >= 0; z--) {	/*From the bottom to the top */
	G_debug(3, _("Writing raster map %i\n"), z + 1);
	for (y = 0; y < rows; y++) {
	    G_percent(y, rows - 1, 10);

	    for (x = 0; x < cols; x++) {
		if (typeIntern == G3D_FLOAT) {
		    G3d_getValue(map, x, y, z, &f1, typeIntern);
		    if (G3d_isNullValueNum(&f1, G3D_FLOAT))
		        G_set_null_value(&fcell[x], 1, FCELL_TYPE);
		    else
			fcell[x] = (FCELL) f1;
		}
		else {
		    G3d_getValue(map, x, y, z, &d1, typeIntern);
		    if (G3d_isNullValueNum(&d1, G3D_DOUBLE))
		        G_set_null_value(&dcell[x], 1, DCELL_TYPE);
		    else
			dcell[x] = (DCELL) d1;
		}
	    }
	    if (typeIntern == G3D_FLOAT) {
		check = G_put_f_raster_row(fd[pos], fcell);
		if (check != 1)
		    fatalError(map, fd, depths,
			       _("Could not write raster row"));
	    }

	    if (typeIntern == G3D_DOUBLE) {
		check = G_put_d_raster_row(fd[pos], dcell);
		if (check != 1)
		    fatalError(map, fd, depths,
			       _("Could not write raster row"));
	    }
	}
	G_debug(3, _("\nDone\n"));
	pos++;
    }


    if (dcell)
	G_free(dcell);
    if (fcell)
	G_free(fcell);

}


/* ************************************************************************* */
/* Main function, open the G3D map and create the raster maps ************** */
/* ************************************************************************* */
int main(int argc, char *argv[])
{
    G3D_Region region;
    struct GModule *module;
    void *map = NULL;		/*The 3D Rastermap */
    int i = 0, changemask = 0;
    int *fd = NULL, output_type, cols, rows;
    char *RasterFileName;

    /* Initialize GRASS */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->description = _("Converts 3D raster maps to 2D raster maps");

    /* Get parameters from user */
    setParams();

    /* Have GRASS get inputs */
    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);


    /*Check Input, hmmm normaly not needed, but who knowa .... */
    if (param.output->answer == NULL)
	G3d_fatalError(_("No output maps"));

    G_debug(3, _("Open 3DRaster file %s"), param.input->answers[i]);

    if (NULL == G_find_grid3(param.input->answers[i], ""))
	G3d_fatalError(_("Requested g3d file not found"));

    /*Open the g3d map */
    map = G3d_openCellOld(param.input->answer,
			  G_find_grid3(param.input->answer, ""),
			  G3D_DEFAULT_WINDOW, G3D_TILE_SAME_AS_FILE,
			  G3D_USE_CACHE_DEFAULT);

    if (map == NULL)
	G3d_fatalError(_("Error opening g3d file"));

    /*Get the output type */
    output_type = G3d_fileTypeMap(map);

    if (output_type == G3D_FLOAT || output_type == G3D_DOUBLE) {

	/* Figure out the region from the map */
	G3d_initDefaults();
	G3d_getWindow(&region);

	/*Check if the g3d-region is equal to the 2d rows and cols */
	rows = G_window_rows();
	cols = G_window_cols();

	if (rows != region.rows || cols != region.cols) {
	    fatalError(map, NULL, 0,
		       _
		       ("Resolution of raster and raster3d maps should be equal!"));
	}


	/*prepare the filehandler */
	fd = (int *)G_malloc(region.depths * sizeof(int));

	if (fd == NULL)
	    fatalError(map, NULL, 0, _("out of memory!"));

	if (G_legal_filename(param.output->answer) < 0)
	    fatalError(map, NULL, 0, _("Illegal output file name"));

	G_message(_("Creating %i raster maps\n"), region.depths);

	/*Loop over all output maps! open */
	for (i = 0; i < region.depths; i++) {
	    /*Create the outputmaps */
	    G_asprintf(&RasterFileName, "%s_%i", param.output->answer, i + 1);
	    G_message(_("Raster map %i Filename: %s\n"), i + 1, RasterFileName);

	    if (G_find_cell2(RasterFileName, ""))
		G_message(_
			  ("Raster map %i Filename: %s already exists. Will be overwritten!\n"),
			  i + 1, RasterFileName);

	    if (output_type == G3D_FLOAT)
		fd[i] = open_output_map(RasterFileName, FCELL_TYPE);
	    else if (output_type == G3D_DOUBLE)
		fd[i] = open_output_map(RasterFileName, DCELL_TYPE);

	}

	/*if requested set the Mask on */
	if (param.mask->answer) {
	    if (G3d_maskFileExists()) {
		changemask = 0;
		if (G3d_maskIsOff(map)) {
		    G3d_maskOn(map);
		    changemask = 1;
		}
	    }
	}

	/*Create the Rastermaps */
	G3dToRaster(map, region, fd);

	/*Loop over all output maps! close */
	for (i = 0; i < region.depths; i++)
	    close_output_map(fd[i]);

	/*We set the Mask off, if it was off before */
	if (param.mask->answer) {
	    if (G3d_maskFileExists())
		if (G3d_maskIsOn(map) && changemask)
		    G3d_maskOff(map);
	}


	/*Cleaning */
	if (RasterFileName)
	    G_free(RasterFileName);

	if (fd)
	    G_free(fd);
    }
    else {
	fatalError(map, NULL, 0,
		   _("Wrong G3D Datatype! Cannot create raster maps."));
    }

    /* Close files and exit */
    if (!G3d_closeCell(map))
	fatalError(map, NULL, 0, _("Error closing g3d file"));

    map = NULL;

    return (EXIT_SUCCESS);
}



/* ************************************************************************* */
/* Open the raster output map ********************************************** */
/* ************************************************************************* */
int open_output_map(const char *name, int res_type)
{
    int fd;

    fd = G_open_raster_new((char *)name, res_type);
    if (fd < 0)
	G_fatal_error(_("cannot create output map [%s]"), name);

    return fd;
}

/* ************************************************************************* */
/* Close the raster output map ********************************************* */
/* ************************************************************************* */
void close_output_map(int fd)
{
    if (G_close_cell(fd) < 0)
	G_fatal_error(_("unable to close output map"));
}
