#include <stdlib.h>
#include <gis.h>
#include <dbmi.h>

static int cmp();

/* selet array of ordered integers
 *
 * return: number of selected values
 *         -1 on error
 */

int db_select_int (dbDriver *driver, char *tab, char *col, char *where, int **pval)
{
    int i, j, type, more, alloc, count;
    int *val;
    char buf[1024], *sval;
    dbString stmt;
    dbCursor cursor;
    dbColumn *column;
    dbValue *value;
    dbTable *table;
    
    /* allocate */
    alloc = 1000;
    val = (int *) G_malloc ( alloc * sizeof(int));

    if ( where == NULL || strlen(where) == 0 )
        snprintf(buf,1023, "SELECT %s FROM %s\n", col, tab);
    else
        snprintf(buf,1023, "SELECT %s FROM %s WHERE %s\n", col, tab, where);

    db_init_string ( &stmt);
    db_append_string ( &stmt, buf);

    if (db_open_select_cursor(driver, &stmt, &cursor, DB_SEQUENTIAL) != DB_OK)
            return (-1);

    table = db_get_cursor_table (&cursor);
    column = db_get_table_column(table, 0); /* first column */
    value  = db_get_column_value(column);
    type = db_get_column_sqltype(column);
    type = db_sqltype_to_Ctype(type);

    /* fetch the data */
    count = 0;
    while(1)
      {
        if(db_fetch (&cursor, DB_NEXT, &more) != DB_OK)
	    return (-1);

        if (!more) break;  
						
	if ( count == alloc )
	  {
            alloc += 1000;		  
            val = (int *) G_realloc ( val, alloc * sizeof(int));
	  }
	
	switch ( type )
	  {
	    case ( DB_C_TYPE_INT ):
                val[count] = db_get_value_int(value);
	        break;
	    case ( DB_C_TYPE_STRING ):
                sval = db_get_value_string(value);
                val[count] = atoi(sval);
	        break;
	    case ( DB_C_TYPE_DOUBLE ):
                val[count] = (int) db_get_value_double(value);
	        break;
            default:
	    	return (-1);
	  }
	count++;
    }

    db_close_cursor(&cursor);

    qsort( (void *)val, count, sizeof(int), cmp);

    *pval = val; 

    return (count);
}

int cmp ( const void *pa, const void *pb)
{
    int *p1 = (int *) pa;    
    int *p2 = (int *) pb;

    if( *p1 < *p2 )
        return -1;
    if( *p1 > *p2 )
        return 1;
    return 0;
}

