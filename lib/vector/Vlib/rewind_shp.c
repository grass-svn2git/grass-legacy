/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Original author CERL, probably Dave Gerdes or Mike Higgins.
*               Update to GRASS 5.7 Radim Blazek and David D. Gray.
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include "Vect.h"

/* Rewind vector data file to cause reads to start at beginning. 
** returns 0 on success
**        -1 on error
*/
int 
V1_rewind_shp (struct Map_info *Map)
{
    G_debug (2, "V1_rewind_shp(): name = %s", Map->name);
    
    Map->fInfo.shp.shape = 0;
    Map->fInfo.shp.part = 0;
    
    return 0;
}

int 
V2_rewind_shp (struct Map_info *Map)
{
    G_debug (2, "V2_rewind_shp(): name = %s", Map->name);

    Map->next_line = 1;
    
    return V1_rewind_shp (Map);	/* make sure level 1 reads are reset too */
}

