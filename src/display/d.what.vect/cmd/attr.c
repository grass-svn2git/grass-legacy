#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"

int disp_attr(char *tabname, char *key, int *keyval )
{
	int      col, ncols, ctype, sqltype, more; 
        char     buf[5001], *colname;
        dbString str, sout1, sout2, *tstr;
	dbDriver *driver;
	dbHandle handle;
        dbCursor cursor;
        dbTable  *table;
        dbColumn *column;
        dbValue  *value;

        driver = db_start_driver(NULL);
	if (driver == NULL)
	    G_fatal_error ("Cannot open driver");

	db_init_handle (&handle);
	db_set_handle (&handle, NULL, NULL);

	if (db_open_database(driver, &handle) != DB_OK)
	    G_fatal_error ("Cannot open database");
	
	db_init_string (&str);
	db_init_string (&sout1);
	db_init_string (&sout2);	

        sprintf (buf, "select * from %s where %s = %d", tabname, key, keyval);
        db_set_string (&str, buf);  
        if (db_open_select_cursor(driver, &str, &cursor, DB_SEQUENTIAL) != DB_OK) return (-1);
        table = db_get_cursor_table (&cursor);

	db_set_string (&sout1, tabname);
	
        if ( db_fetch (&cursor, DB_NEXT, &more ) != DB_OK ) return (-1);
	ncols = db_get_table_number_of_columns (table);
	for( col = 0; col < ncols; col++) {
	    column = db_get_table_column(table, col);
	    sqltype = db_get_column_sqltype (column);
	    ctype = db_sqltype_to_Ctype(sqltype);
	    value  = db_get_column_value(column);
	    db_convert_value_to_string( value, sqltype, &str);
	    colname = db_get_column_name (column);

	    if ( strcmp (colname, key) == 0 ) tstr = &sout1;
	    else tstr = &sout2;

	    db_append_string ( tstr, "\n");	    
	    db_append_string ( tstr, colname);
	    db_append_string ( tstr, "\n");

	    switch ( ctype ) {
		case DB_C_TYPE_INT:
		    db_append_string ( tstr, "i\n");
		    break;		    
		case DB_C_TYPE_DOUBLE:
		    db_append_string ( tstr, "d\n");		    
		    break;	    
		case DB_C_TYPE_STRING:
		    db_append_string ( tstr, "s\n");
		    break;
		case DB_C_TYPE_DATETIME:
		    db_append_string ( tstr, "t\n");
		    break;
	    }		
	    db_append_string ( tstr, db_get_string(&str));
        } 
        db_close_cursor(&cursor);	 																						    				
	db_append_string ( &sout1, db_get_string(&sout2));
	sprintf ( buf, "echo '%s' | %s/etc/db.attr&", db_get_string(&sout1), G_gisbase());
	system ( buf );
	
	db_free_string (&str);
	db_free_string (&sout1);
	db_free_string (&sout2);

        db_close_database(driver);
	db_shutdown_driver(driver); 
	
	return (DB_OK);
}
