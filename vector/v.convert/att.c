#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"
#include "Vect.h"

/* *  convert dig_cats to database table
*  return: number of records inserted
*          -1 error */
int attributes (char *in, struct Map_info *Out )
{
    int    i, cat, count;
    char   *mapset;
    struct Categories Cats;
    char   buf[1024];
    dbString sql, lab;
    dbDriver *driver;
    dbHandle handle;
    struct field_info *fi;
    int   len, clen;

    db_init_string (&sql);
    db_init_string (&lab);

    /* Find dig_cats if exists */
    if ( NULL == (mapset = G_find_file ("dig_cats", in, "") )) {
        fprintf(stderr,"No category labels (dig_cats) found, no table created.\n") ;
	return 0;
    }
    
    if (G_read_vector_cats ( in, mapset, &Cats) == -1) {
	G_warning ("Cannot open dig_cats file.");
	return -1;
    }
	
    fi = Vect_default_field_info ( Out, 1, NULL, GV_1TABLE );
    Vect_map_add_dblink ( Out, 1, NULL, fi->table, "cat", fi->database, fi->driver);

    /* Get maximum column length */
    clen = 0;
    for ( i = 0; i < Cats.ncats; i++) {
	len = strlen ( Cats.labels[i] );
        if ( len > clen ) clen = len;
    }
    clen += 10; 
	
    /* Create new table */
    sprintf ( buf, "create table %s ( cat integer, label varchar(%d) )", fi->table, clen );
    db_append_string ( &sql, buf );
    
    G_debug ( 1, db_get_string ( &sql ) );
    
    driver = db_start_driver( fi->driver );
    if (driver == NULL) G_fatal_error ( "Cannot open driver %s", fi->driver );
    db_init_handle (&handle);
    db_set_handle (&handle, Vect_subst_var(fi->database, Out), NULL);
    if (db_open_database(driver, &handle) != DB_OK) {
	db_shutdown_driver(driver);
	G_fatal_error ( "Cannot open database %s", fi->database );
    }

    if (db_execute_immediate (driver, &sql) != DB_OK ) {
	db_close_database(driver);
	db_shutdown_driver(driver);
	G_fatal_error ( "Cannot create table: %s", db_get_string ( &sql )  );
    }

    G_debug ( 1, "ncats = %d", Cats.ncats );	
    count = 0;
    for ( i = 0; i < Cats.ncats; i++) {
	cat = (int) Cats.q.table[i].dLow; /* cats are in dLow/High not in cLow/High !!! */ 
        G_debug ( 3, "%d cat = %d label = %s", i, cat, Cats.labels[i] );
	
	db_set_string ( &lab, Cats.labels[i]);
	db_double_quote_string ( &lab );
	sprintf (buf, "insert into %s values ( %d, '%s')", fi->table, cat, db_get_string(&lab) );
	db_set_string ( &sql, buf);
        G_debug ( 3, db_get_string ( &sql ) );

	if (db_execute_immediate (driver, &sql) != DB_OK ) {
	    db_close_database(driver);
	    db_shutdown_driver(driver);
	    G_fatal_error ( "Cannot insert into table: %s", db_get_string ( &sql )  );
	}
        count++;
    }

    db_close_database(driver);
    db_shutdown_driver(driver);
    
    return count;
}

