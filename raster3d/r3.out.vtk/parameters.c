
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
#include <grass/gis.h>
#include <grass/glocale.h>
#include "parameters.h"

/* ************************************************************************* */
/* Setg up the arguments we are expecting ********************************** */
/* ************************************************************************* */
void set_params()
{
    param.input = G_define_option();
    param.input->key = "input";
    param.input->type = TYPE_STRING;
    param.input->required = NO;
    param.input->gisprompt = "old,grid3,3d-raster";
    param.input->multiple = YES;
    param.input->description =
	_("G3D map(s) to be converted to VTK-ASCII data format");

    param.rgbmaps = G_define_option();
    param.rgbmaps->key = "rgbmaps";
    param.rgbmaps->type = TYPE_STRING;
    param.rgbmaps->required = NO;
    param.rgbmaps->gisprompt = "old,grid3,3d-raster";
    param.rgbmaps->multiple = YES;
    param.rgbmaps->description =
	_
	("Three (r,g,b) 3d raster maps which are used to create rgb values [redmap,greenmap,bluemap]");

    param.vectormaps = G_define_option();
    param.vectormaps->key = "vectormaps";
    param.vectormaps->type = TYPE_STRING;
    param.vectormaps->required = NO;
    param.vectormaps->gisprompt = "old,grid3,3d-raster";
    param.vectormaps->multiple = YES;
    param.vectormaps->description =
	_
	("Three (x,y,z) 3d raster maps which are used to create vector values [xmap,ymap,zmap]");

    param.top = G_define_option();
    param.top->key = "top";
    param.top->type = TYPE_STRING;
    param.top->required = NO;
    param.top->gisprompt = "old,cell,raster";
    param.top->multiple = NO;
    param.top->description = _("top surface 2D raster map");

    param.bottom = G_define_option();
    param.bottom->key = "bottom";
    param.bottom->type = TYPE_STRING;
    param.bottom->required = NO;
    param.bottom->gisprompt = "old,cell,raster";
    param.bottom->multiple = NO;
    param.bottom->description = _("bottom surface 2D raster map");

    param.output = G_define_option();
    param.output->key = "output";
    param.output->type = TYPE_STRING;
    param.output->gisprompt = "new_file,file,output";
    param.output->required = NO;
    param.output->description = _("Name for VTK-ASCII output file");


    param.null_val = G_define_option();
    param.null_val->key = "null";
    param.null_val->type = TYPE_DOUBLE;
    param.null_val->required = NO;
    param.null_val->description =
	_("Float value to represent no data cell/points");
    param.null_val->answer = "-99999.99";


    param.elevscale = G_define_option();
    param.elevscale->key = "elevscale";
    param.elevscale->type = TYPE_DOUBLE;
    param.elevscale->required = NO;
    param.elevscale->description = _("Scale factor for elevation");
    param.elevscale->answer = "1.0";

    param.decimals = G_define_option();
    param.decimals->key = "dp";
    param.decimals->type = TYPE_INTEGER;
    param.decimals->required = NO;
    param.decimals->multiple = NO;
    param.decimals->answer = "12";
    param.decimals->options = "0-20";
    param.decimals->description =
	_("Number of significant digits (floating point only)");

    param.mask = G_define_flag();
    param.mask->key = 'm';
    param.mask->description = _("Use g3d mask (if exists) with input maps");

    param.point = G_define_flag();
    param.point->key = 'p';
    param.point->description =
	_("Create VTK pointdata instead of VTK celldata (celldata is default)");

    param.origin = G_define_flag();
    param.origin->key = 'o';
    param.origin->description = _("Scale factor effects the origin");

    param.structgrid = G_define_flag();
    param.structgrid->key = 's';
    param.structgrid->description =
	_
	("Create 3d elevation output with a top and a bottom surface, both raster maps are required.");

    param.coorcorr = G_define_flag();                                            
    param.coorcorr->key = 'c';                                                   
    param.coorcorr->description = _("Correct the coordinates to fit the VTK-OpenGL precision");
    
    /* Maybe needed in the future
     * param.xml = G_define_flag ();
     * param.xml->key = 'x';
     * param.xml->description = "Write XML-VTK-format";
     */

    return;
}

