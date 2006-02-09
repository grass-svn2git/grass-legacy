/***********************************************************
*
* MODULE:       SQLite driver 
*   	    	
* AUTHOR(S):    Radim Blazek
*
* COPYRIGHT:    (C) 2005 by the GRASS Development Team
*
* This program is free software under the GNU General Public
* License (>=v2). Read the file COPYING that comes with GRASS
* for details.
*
**************************************************************/
#include <stdio.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include "globals.h"

/* init error message */
void
init_error ( void )
{
    if ( !errMsg ) {
	errMsg = (dbString *) G_malloc(sizeof(dbString));
        db_init_string (errMsg);
    }

    db_set_string ( errMsg, "DBMI-SQLite driver error:\n");
}

/* append error message */
void
append_error ( char *msg )
{
    db_append_string ( errMsg, msg);
}


void
report_error ( void )
{
    db_append_string ( errMsg, "\n");
    db_error ( db_get_string (errMsg) );
}

