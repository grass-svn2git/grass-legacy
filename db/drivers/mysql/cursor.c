/*****************************************************************************
*
* MODULE:       MySQL driver forked from DBF driver by Radim Blazek 
*   	    	
* AUTHOR(S):    Alex Shevlakov
*
* PURPOSE:      Simple driver for reading and writing data     
*
* COPYRIGHT:    (C) 2000 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <stdlib.h>
#include <dbmi.h>
#include "gis.h"
#include "globals.h"
#include "proto.h"

int db__driver_close_cursor(dbc)
     dbCursor *dbc;
{
    cursor *c;

    /* get my cursor via the dbc token */
    c = (cursor *) db_find_token(db_get_cursor_token(dbc));
    if (c == NULL)
	return DB_FAILED;

    /* free_cursor(cursor) */
    free_cursor(c);

    return DB_OK;
}


cursor *alloc_cursor()
{
    cursor *c;

    /* allocate the cursor */
    c = (cursor *) db_malloc(sizeof(cursor));
    if (c == NULL) {
	report_error("allocate cursor");
	return c;
    }

    /* tokenize it */
    c->token = db_new_token(c);
    if (c->token < 0) {
	free_cursor(c);
	c = NULL;
	report_error("db_new_token()");
    }

    return c;
}

void free_cursor(cursor * c)
{
    db_drop_token(c->token);
    sqpFreeStmt(c->st);
    G_free (c);
}
