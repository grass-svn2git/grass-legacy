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
#include <stdlib.h>
#include <dbmi.h>
#include "gis.h"
#include "globals.h"
#include "proto.h"

int db__driver_open_select_cursor (dbString *sel, dbCursor *dbc, int mode)

{
    cursor   *c;
    dbTable  *table;
    char     *str, *rest;
    int      ret;

    init_error();

    /* allocate cursor */
    c = alloc_cursor();
    if (c == NULL)
	return DB_FAILED;

    db_set_cursor_mode(dbc, mode);
    db_set_cursor_type_readonly(dbc);

    /* \ must be escaped, see explanation in db_driver_execute_immediate() */
    str = G_str_replace ( db_get_string(sel), "\\", "\\\\" );
    G_debug ( 3, "Escaped SQL: %s", str );

    ret = sqlite3_prepare ( sqlite, str, -1, 
			    &(c->statement), &rest );

    if ( str )
	G_free ( str );

    if ( ret != SQLITE_OK )
    {
        append_error("Error in sqlite3_prepare():");
	append_error(db_get_string(sel) );
	append_error( "\n" );
        append_error ( sqlite3_errmsg(sqlite) );
        report_error( );
        return DB_FAILED;
    }

    if ( describe_table( c->statement, &table, c) == DB_FAILED ) {
	append_error("Cannot describe table\n");
	report_error();
	return DB_FAILED;
    }

    c->nrows = -1;
    c->row = -1;

    /* record table with dbCursor */
    db_set_cursor_table(dbc, table);

    /* set dbCursor's token for my cursor */
    db_set_cursor_token(dbc, c->token);

    return DB_OK;
}
