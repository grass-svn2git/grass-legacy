/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Original author CERL, probably Dave Gerdes or Mike Higgins.
*               Update to GRASS 5.1 Radim Blazek and David D. Gray.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Vect.h"
#include "dbmi.h"
#include "gis.h"

/*!
 \fn struct dblinks *Vect_new_dblinks_struct ( void )
 \brief create and init new dblinks ctructure
 \return pointer to new dblinks structure
 \param void
*/
struct dblinks *
Vect_new_dblinks_struct ( void )
{
  struct dblinks *p;

  p = (struct dblinks *) G_malloc (sizeof (struct dblinks));

  if (p) {
      p->alloc_fields = p->n_fields = 0;
      p->field = NULL;
  }

  return p;
}

/*!
 \fn void Vect_reset_dblinks ( struct dblinks *p )
 \brief reset dblinks structure
 \return void
 \param pointer to existing dblinks structure
*/
void
Vect_reset_dblinks ( struct dblinks *p )
{
    p->n_fields = 0;
}

/*!
 \fn int Vect_map_add_dblink ( struct Map_info *Map, int number, char *name, char *table, char *key,
                      char *db, char *driver )
 \brief add new db connection to Map_info structure
 \return 0 OK; -1 error
 \param pointer to existing Map structure
*/
int
Vect_map_add_dblink ( struct Map_info *Map, int number, char *name, char *table, char *key, 
	             char *db, char *driver )
{
    int ret;

    if (number == 0) {
        G_warning ("Field number must be 1 or greater.");
	return -1;
    }
    
    if (Map->mode != GV_MODE_WRITE && Map->mode != GV_MODE_RW) {
        G_warning ("Cannot add database link, map is not opened in WRITE mode.");
	return -1;
    }
    
    ret = Vect_add_dblink ( Map->dblnk, number, name, table, key, db, driver );
    if ( ret == -1 ) {
        G_warning ("Cannot add database link.");
	return -1;
    }
    /* write it immediately otherwise it is lost if module crashes */
    ret = Vect_write_dblinks ( Map );
    if ( ret == -1 ) {
        G_warning ("Cannot write database links.");
	return -1;
    }
    return 0;
}

/*!
 \fn int Vect_map_del_dblink ( struct Map_info *Map, int number)
 \brief delete db connection from Map_info structure
 \return 0 deleted, -1 no such field found
 \param pointer to existing Map structure, field number
*/
int
Vect_map_del_dblink ( struct Map_info *Map, int field)
{
    int    i, j, ret;
    struct dblinks *links;

    G_debug(4, "Vect_map_del_dblink() field = %d", field);
    links = Map->dblnk;
    
    ret = -1;
    for (i = 0; i < links->n_fields; i++) {
        if ( links->field[i].number == field ) { /* field found */
            for (j = i; j < links->n_fields - 1; j++) {
		links->field[j].number = links->field[j+1].number;
		links->field[j].name = links->field[j+1].name;
		links->field[j].table = links->field[j+1].table;
		links->field[j].key = links->field[j+1].key;
		links->field[j].database = links->field[j+1].database;
		links->field[j].driver = links->field[j+1].driver;
	    }
	    ret = 0;
            links->n_fields--;
        }
    }

    return ret;
}

/*!
 \fn int Vect_map_check_dblink ( struct Map_info *Map, int field )
 \brief check if db connection exists in dblinks structure
 \return 1 dblink for field exists; 0 dblink does not exist for field
 \param pointer to Map structure, field number
*/
int
Vect_map_check_dblink ( struct Map_info *Map, int field )
{
    return Vect_check_dblink ( Map->dblnk, field );
}

/*!
 \fn int Vect_check_dblink ( struct dblinks *p, int field )
 \brief check if db connection exists in dblinks structure
 \return 1 dblink for field exists; 0 dblink does not exist for field
 \param pointer to existing dblinks structure, field number
*/
int
Vect_check_dblink ( struct dblinks *p, int field )
{
    int i;
    
    G_debug(3,"Vect_check_dblink: field %d", field);
    
    for (i = 0; i < p->n_fields; i++) {
       if ( p->field[i].number == field ) {
	   return 1;
       }
    }
    return 0;
}

    
/*!
 \fn int Vect_add_dblink ( struct dblinks *p, int number, char *name, char *table, char *key, 
                            char *db, char *driver ) 
 \brief add new db connection to dblinks structure
 \return 0 OK; -1 error
 \param pointer to existing dblinks structure
*/
int
Vect_add_dblink ( struct dblinks *p, int number, char *name, char *table, char *key, char *db, char *driver )
{
    int ret;
    
    ret = Vect_check_dblink ( p, number );
    if ( ret == 1 ) {
         G_warning ("Field number <%d> or name <%s> already exists", number, name);
         return -1;
    }

    if ( p->n_fields == p->alloc_fields ) {
       p->alloc_fields += 10;
       p->field = ( struct field_info *) G_realloc ( (void *) p->field, 
	                   p->alloc_fields * sizeof (struct field_info) );
    }

    p->field[p->n_fields].number =  number;
    
    if ( name != NULL ) p->field[p->n_fields].name = G_store ( name );
    else  p->field[p->n_fields].name = NULL;
    
    if ( table != NULL ) p->field[p->n_fields].table = G_store ( table );
    else p->field[p->n_fields].table = NULL;
    
    if ( key != NULL ) p->field[p->n_fields].key = G_store ( key );
    else p->field[p->n_fields].key = NULL;
    
    if ( db != NULL ) p->field[p->n_fields].database = G_store ( db );
    else p->field[p->n_fields].database = NULL;
    
    if ( driver != NULL ) p->field[p->n_fields].driver = G_store ( driver );
    else p->field[p->n_fields].driver = NULL;
    
    p->n_fields++;

    return 0;
}

/*!
 \fn struct field_info *Vect_default_field_info ( struct Map_info *Map, int  field, char *field_name, int  type)
 \brief get default information about link to database for new dblink
 \return pointer to new field_info structure
 \param pointer to map structure, category field
*/
struct field_info
*Vect_default_field_info (
    struct Map_info *Map,  /* pointer to map structure */		
    int  field,    /* category field */
    char *field_name, /* field name or NULL */
    int  type ) /* how many tables are linked to map: GV_1TABLE / GV_MTABLE */
{
    struct field_info *fi;
    char buf[1000];
    char *drv, *db;
    dbConnection  connection;
    
    G_debug (1, "Vect_default_field_info(): map = %s field = %d", Map->name, field);
    
    drv = G__getenv2 ( "GV_DRIVER", G_VAR_MAPSET );
    db = G__getenv2 ( "GV_DATABASE", G_VAR_MAPSET );

    G_debug (2, "drv = %s db = %s", drv, db );

    if ( drv == NULL && db == NULL ) { /* Set default values and create dbf db dir */
	G_warning ( "Default driver / database for vectors set to:\n"
		    "driver: dbf\ndatabase: $GISDBASE/$LOCATION_NAME/$MAPSET/dbf/" );
	G_setenv2 ( "GV_DRIVER", "dbf", G_VAR_MAPSET );
	G_setenv2 ( "GV_DATABASE", "$GISDBASE/$LOCATION_NAME/$MAPSET/dbf/", G_VAR_MAPSET );

	/* Set also DB_DRIVER and DB_DATABASE is not yet set */
	db_get_connection( &connection );
	if ( !connection.driverName && !connection.databaseName ) {
	    G_warning ( "Database connection (for db.* modules) set to:\n"
			"driver: dbf\ndatabase: $GISDBASE/$LOCATION_NAME/$MAPSET/dbf/" );
	    connection.driverName = "dbf";
	    connection.databaseName = "$GISDBASE/$LOCATION_NAME/$MAPSET/dbf/";
	    db_set_connection( &connection );
	}
	
	sprintf ( buf, "%s/%s/%s/dbf", Map->gisdbase, Map->location, Map->mapset );
	G__make_mapset_element ( "dbf" );
	drv = G_store ( "dbf" );
	db = G_store ( "$GISDBASE/$LOCATION_NAME/$MAPSET/dbf" );
    } else if ( drv == NULL ) {
       G_fatal_error ( "Default driver is not set" ); 
    } else if ( db == NULL ) {
       G_fatal_error ( "Default database is not set" ); 
    }
    
    fi = (struct field_info *) G_malloc( sizeof(struct field_info) );
    
    fi->number = field;
    if ( field_name != NULL ) fi->name = G_store ( field_name );
    else fi->name = NULL;
    
    if ( type == GV_1TABLE ) {
        fi->table = G_store ( Map->name );
    } else {
	if ( field_name != NULL && strlen ( field_name ) > 0 )
	    sprintf ( buf, "%s_%s", Map->name, field_name );
	else
	    sprintf ( buf, "%s_%d", Map->name, field );

	fi->table = G_store ( buf );
    }
    
    fi->key = G_store ( "cat" ); /* Should be: id/fid/gfid/... ? */
    fi->database = G_store( db );
    fi->driver = G_store( drv );

    return (fi);
}

/*!
 \fn struct field_info *Vect_get_dblink (  struct Map_info *Map, int link)
 \brief get information about link to database, variables are substituted by values,
        link is index to array of dblinks
 \return pointer to new field_info structure
 \param pointer Map_info structure, link number
*/
struct field_info
*Vect_get_dblink (  struct Map_info *Map, int link )
{
    struct field_info *fi;

    G_debug (1, "Vect_get_dblink(): link = %d", link);

    if ( link >= Map->dblnk->n_fields ) {
	G_warning ( "Requested dblink %d, maximum link number %d", link, Map->dblnk->n_fields - 1 );
	return NULL;
    }

    fi = (struct field_info *) malloc( sizeof(struct field_info) );
    fi->number = Map->dblnk->field[link].number;
    
    if ( Map->dblnk->field[link].name != NULL ) 
        fi->name = G_store ( Map->dblnk->field[link].name );
    else
	fi->name = NULL;
    
    fi->table = G_store ( Map->dblnk->field[link].table );
    fi->key = G_store ( Map->dblnk->field[link].key );
    fi->database = Vect_subst_var ( Map->dblnk->field[link].database, Map );
    fi->driver = G_store ( Map->dblnk->field[link].driver );

    return fi;
}

/*!
 \fn struct field_info *Vect_get_field (  struct Map_info *Map, int field )
 \brief get information about link to database, variables are substituted by values,
        field is number of requested field
 \return pointer to new field_info structure or NULL
 \param pointer Map_info structure, field number
*/
struct field_info
*Vect_get_field (  struct Map_info *Map, int field )
{
    int i;
    struct field_info *fi = NULL;

    G_debug (1, "Vect_get_field(): field = %d, nfields: %d", field);

    for ( i = 0; i < Map->dblnk->n_fields; i++ ) {
        if ( Map->dblnk->field[i].number == field ) {
            fi = Vect_get_dblink ( Map, i );
	    break;
	}
    }
	    
    return fi;
}

/*!
 \fn int *Vect_read_dblinks ( struct Map_info *Map)
 \brief read dblinks to existing structure, variables are not substituted by values
 \return number of links read or -1 on error
 \param pointer to map structure
*/
int
Vect_read_dblinks ( struct Map_info *Map )    
{
    int  ndef;	
    FILE *fd;
    char file[1024], buf[1024];
    char tab[1024], col[1024], db[1024], drv[1024], fldstr[1024], *fldname;
    int  fld;
    char *c;
    int  row, rule;
    struct dblinks *dbl;
    
    G_debug (1, "Vect_read_dblinks(): map = %s, mapset = %s", Map->name, Map->mapset);
    
    dbl = Map->dblnk;
    Vect_reset_dblinks ( dbl );
    
    if ( Map->format == GV_FORMAT_SHAPE ) {
        G_debug ( 3, "Vector format shape");
	Vect_add_dblink ( dbl, 1, NULL, Map->fInfo.shp.baseName, "shp_fid", Map->fInfo.shp.dirName, "shp" );
	return ( 1 );
    } else if ( Map->format != GV_FORMAT_NATIVE &&  Map->format != GV_FORMAT_POSTGIS ) {
	/* Here will be OGR once */
	G_fatal_error ("Don't know how to read links for format %d", Map->format );
    }
    
    sprintf ( file, "%s/%s/%s/%s/%s/%s", Map->gisdbase, Map->location, Map->mapset, GRASS_VECT_DIRECTORY, 
	                                 Map->name, GRASS_VECT_DBLN_ELEMENT );
    G_debug (1, "dbln file: %s", file);

    fd = fopen ( file, "r" );
    if ( fd == NULL ) { /* This may be correct, no tables defined */
	G_debug ( 1, "Cannot open vector database definition file");
	return (-1);
    }

    row = 0;
    rule = 0;
    while (fgets (buf, 1023, fd) != NULL) {
	row++;      
	G_chop ( buf ); 
	G_debug (1, "dbln: %s", buf);

	c = (char *) strchr ( buf, '#');
	if ( c != NULL ) *c = '\0';

	if ( strlen(buf) == 0 ) continue;
	
	ndef = sscanf ( buf, "%s %s %s %s %s", fldstr, tab, col, db, drv);
    
	if ( ndef < 2 || (ndef < 5 && rule < 1 ) ) {
	    G_warning ( "Error in rule on row %d in %s", row, file);
	    continue;
        }

	/* get field and field name */
	fldname = strchr ( fldstr, '/' );
	if ( fldname != NULL ) { /* field has name */
	    fldname[0] = 0;
	    fldname++;
	}
	fld = atoi ( fldstr );
	
	Vect_add_dblink ( dbl, fld, fldname, tab, col, db, drv );

	G_debug (1, "field = %d name = %s, table = %s, key = %s, database = %s, driver = %s",
		     fld, fldname, tab, col, db, drv );

	rule++;
    }
    fclose (fd);

    G_debug (1, "Dblinks read");
    return ( rule );
}

/*!
 \fn int *Vect_write_dblinks ( struct Map_info *Map )
 \brief write dblinks to file
 \return 0 OK, -1 on error
 \param pointer to map name, pointer to mapset name, pointer to dblinks structure
*/
int
Vect_write_dblinks ( struct Map_info *Map )
{
    int    i;	
    FILE *fd;
    char file[1024], buf[1024];
    struct dblinks *dbl;    
    
    G_debug (1, "Vect_write_dblinks(): map = %s, mapset = %s", Map->name, Map->mapset );
    
    dbl = Map->dblnk;

    sprintf ( file, "%s/%s/%s/%s/%s/%s", Map->gisdbase, Map->location, Map->mapset, GRASS_VECT_DIRECTORY, 
	                                 Map->name, GRASS_VECT_DBLN_ELEMENT );
    G_debug (1, "dbln file: %s", file);

    fd = fopen ( file, "w" );
    if ( fd == NULL ) { /* This may be correct, no tables defined */
	G_warning ( "Cannot open vector database definition file: '%s'", file);
	return (-1);
    }

    for ( i = 0; i < dbl->n_fields; i++ ) {
        if ( dbl->field[i].name != NULL )
	    sprintf ( buf , "%d/%s", dbl->field[i].number, dbl->field[i].name );
	else 
	    sprintf ( buf , "%d", dbl->field[i].number );
        	    
        fprintf ( fd, "%s %s %s %s %s\n", buf, dbl->field[i].table, dbl->field[i].key,
		                dbl->field[i].database, dbl->field[i].driver );
	G_debug (1, "%s %s %s %s %s", buf, dbl->field[i].table, dbl->field[i].key,
		                dbl->field[i].database, dbl->field[i].driver );
    }
    fclose (fd);

    G_debug (1, "Dblinks written");
    return 0;
}

/*!
 \fn chart *Vect_subst_var ( char *in, struct Map_info *Map ) 
 \brief substitute variable in string
 \return pointer to new string
 \param pointer to map
*/
char *
Vect_subst_var ( char *in, struct Map_info *Map )
{
    char *c;
    char buf[1000], str[1000];
    
    G_debug (3, "Vect_subst_var(): in = %s, map = %s, mapset = %s", in, Map->name, Map->mapset);
    
    strcpy ( str, in );
    
    strcpy ( buf, str );
    c = (char *) strstr ( buf, "$GISDBASE" );
    if ( c != NULL ) {
        *c = '\0';	      
        sprintf (str, "%s%s%s", buf, Map->gisdbase, c+9);
    }

    strcpy ( buf, str );
    c = (char *) strstr ( buf, "$LOCATION_NAME" );
    if ( c != NULL ) {
        *c = '\0';	      
        sprintf (str, "%s%s%s", buf, Map->location, c+14);
    }

    strcpy ( buf, str );
    c = (char *) strstr ( buf, "$MAPSET" );
    if ( c != NULL ) {
        *c = '\0';	      
        sprintf (str, "%s%s%s", buf, Map->mapset, c+7);
    }
    
    strcpy ( buf, str );
    c = (char *) strstr ( buf, "$MAP" );
    if ( c != NULL ) {
        *c = '\0';	      
        sprintf (str, "%s%s%s", buf, Map->name, c+4 );
    }
    
    G_debug (3, "  -> %s", str);
    return ( G_store(str) );
}


