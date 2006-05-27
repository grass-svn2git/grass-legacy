
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
#ifndef __R3_OUT_VTK_ERROR_HANDLING_H__
#define __R3_OUT_VTK_ERROR_HANDLING_H__

struct inputMaps;

/*Simple Error message */
void fatalError(char *errorMsg, inputMaps * in);

/*Free the input maps structure und close all open maps */
void ReleaseInputMapsStruct(inputMaps * in);

/*close a raster input map */
void CloseInputMap(int fd);

#endif
