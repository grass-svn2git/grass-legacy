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
#include <string.h>
#include <grass/dbmi.h>
#include "globals.h"
#include "proto.h"

int db__driver_list_tables (dbString **tlist, int *tcount, int system)

{
    int i, nrows;
    dbString *list;
    sqlite3_stmt *statement;
    char  *rest;
    int   ret;

    init_error();

    ret = sqlite3_prepare ( sqlite, 
	"select name from sqlite_master where type = 'table'",
	 -1, &statement, &rest );

    if ( ret != SQLITE_OK ) {
        append_error( "Cannot list tables\n");
        append_error ( sqlite3_errmsg(sqlite) );
        report_error();
        sqlite3_finalize ( statement );
        return DB_FAILED;
    }

    nrows = 0;
    while ( sqlite3_step ( statement ) == SQLITE_ROW )
    {
        nrows++;
    }
    sqlite3_reset ( statement );
    
    G_debug ( 3, "nrows = %d", nrows );
    
    list = db_alloc_string_array(nrows);
    
    if (list == NULL ) {
	append_error ( "Cannot db_alloc_string_array()");
	report_error();
	return DB_FAILED;
    }

    i = 0;
    while ( sqlite3_step ( statement ) == SQLITE_ROW )
    {
        G_debug ( 3, "table: %s", sqlite3_column_text ( statement, 0) );
	db_set_string ( &list[i], 
		sqlite3_column_text ( statement, 0) );
        i++;
    }

    sqlite3_finalize ( statement );

    *tlist = list;
    *tcount = nrows;

    return DB_OK;
}
