#include <dbmi.h>
#include <odbc/sql.h>
#include <odbc/sqlext.h>
#include <odbc/sqltypes.h>
#include "globals.h"
#include "proto.h"

db_driver_list_tables (tlist, tcount, system)
    dbString **tlist;
    int *tcount;
    int system;
{
    cursor      *c;
    dbString 	*list;
    int 	count=0;
    SQLCHAR     tableName[SQL_MAX_TABLE_NAME_LEN];      
    SQLINTEGER  indi, nrow=0;   
    SQLRETURN   ret;
    char        ttype[50];
    
    *tlist = NULL;
    *tcount = 0;

    /* allocate cursor */
    c = alloc_cursor();
    if (c == NULL)
        return DB_FAILED;  

    // Execute SQL 
    if (system)
	sprintf(ttype,"SYSTEM TABLE");
    else
	sprintf(ttype,"TABLE, VIEW");

    ret = SQLTables( c->stmt, NULL, 0, NULL, 0, NULL, 0, ttype, sizeof(ttype) );

    if ( ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO )
    {
        report_error("SQLTables()");
	return DB_FAILED;  
    }  

    SQLBindCol( c->stmt, 3, SQL_C_CHAR, tableName, sizeof(tableName),  &indi );

    // Get number of rows
    ret = SQLRowCount(c->stmt, &nrow);
    if ( ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        report_error("SQLRowCount()");
	return DB_FAILED;  
    }
    
    list = db_alloc_string_array(nrow);
    if (list == NULL)
	return DB_FAILED;       

    // Get table names	
    ret = SQLFetch( c->stmt );
    while ( ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO )  
    {
	if ( indi == SQL_NULL_DATA )
	{
	    if(db_set_string (&list[count], "Unknown") != DB_OK)
		return DB_FAILED;
        }
	else  
	{
    	    if(db_set_string (&list[count], (char *)tableName) != DB_OK)
		return DB_FAILED;
	}	
	count++;
        ret = SQLFetch( c->stmt );
    }
    						
    free_cursor (c);

    *tlist = list;
    *tcount = count;
    return DB_OK;
}
