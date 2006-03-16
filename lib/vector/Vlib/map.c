/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Radim Blazek
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
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grass/glocale.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>

/*!
 \fn int Vect_copy_map_lines ( struct Map_info *In, struct Map_info *Out )
 \brief copy all alive elements of opened vector map to another opened vector map
 \return 0 on success, 1 on error
 \param  in Map_info structure, out Map_info structure
*/
int 
Vect_copy_map_lines ( struct Map_info *In, struct Map_info *Out )
{
    int    i, type, nlines, ret;
    struct line_pnts *Points;
    struct line_cats *Cats;

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
   
    if ( Vect_level ( In ) < 1 )
	G_fatal_error ("Vect_copy_map_lines(): input vector is not open");
    
    ret = 0;
    /* Note: sometimes is important to copy on level 2 (pseudotopo centroids) 
     *       and sometimes on level 1 if build take too long time */
    if ( Vect_level ( In ) >= 2 ) { 
	nlines = Vect_get_num_lines ( In );
	for ( i = 1; i <= nlines; i++ ) {
	    type =  Vect_read_line (In, Points, Cats, i);
	    if ( type == -1 ) {
		G_warning ("Cannot read vector file\n" );
		ret = 1;
		break;
	    } 
	    if ( type == 0 ) continue; /* dead line */

	    Vect_write_line ( Out, type, Points, Cats );
	}
    } else {  /* Level 1 */
	Vect_rewind ( In );
	while ( 1 ) {
	    type =  Vect_read_next_line (In, Points, Cats);
	    if ( type == -1 ) {
		G_warning ("Cannot read vector file\n" );
		ret = 1;
		break;
	    } else if ( type == -2 ) { /* EOF */ 
		break;
	    } else if ( type == 0 ) { /* dead line */
		continue;
	    }
	    Vect_write_line ( Out, type, Points, Cats );
	}
    }
    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);

    return ret;
}

/* Copy file
 * returns 0 OK
 *         1 error
 */
int
copy_file(const char *src, const char *dst)
{
    char buf[1024];
    int fd, fd2;
    FILE *f2; 
    unsigned int len, len2;

    if((fd = open(src, O_RDONLY)) < 0) return 1;
    
    /* if((fd2 = open(dst, O_CREAT|O_TRUNC|O_WRONLY)) < 0) */
    if((f2 = fopen(dst, "w")) == NULL)
    {
        close(fd);
        return 1;
    }   

    fd2 = fileno(f2);

    while((len = read(fd, buf, 1024)) > 0)
    {
        while(len && (len2 = write(fd2, buf, len)) >= 0)
           len -= len2;
    }

    close(fd);
    /* close(fd2); */
    fclose(f2);

    if ( len == -1 || len2 == -1  ) return 1;

    return 0;
}

/*!
 \fn int Vect_copy ( char *in, char *mapset, char *out, FILE *msgout )
 \brief copy a map including attribute tables
        Old vector is deleted
 \return -1 error, 0 success
 \param in input vector
 \param out output vector
 \param msgout output file for messages or NULL 
*/
int 
Vect_copy ( char *in, char *mapset, char *out, FILE *msgout )
{
    int i, n, ret, type;
    struct Map_info In, Out;
    struct field_info *Fi, *Fin;
    char   old_path[1000], new_path[1000], buf[1000]; 
    struct stat info;
    char *files[] = { GRASS_VECT_FRMT_ELEMENT, GRASS_VECT_COOR_ELEMENT,
                      GRASS_VECT_HEAD_ELEMENT, GRASS_VECT_HIST_ELEMENT,
                      GV_TOPO_ELEMENT, GV_SIDX_ELEMENT, GV_CIDX_ELEMENT,
                      NULL };
      
    dbDriver *driver;

    G_debug (2, "Copy vector '%s' in '%s' to '%s'", in, mapset, out );
    /* check for [A-Za-z][A-Za-z0-9_]* in name */
    if (Vect_legal_filename(out) < 0 )
       G_fatal_error ( _("Map name is not SQL compliant.") );

    /* Delete old vector if it exists */
    if ( G_find_vector2(out, G_mapset()) ) {
	G_warning (_("The vector '%s' already exists and will be overwritten."), out);
	ret = Vect_delete ( out );
	if ( ret != 0 ) {
	    G_warning ( "Cannot copy vector" );
            return -1;
	}
    }

    /* Copy the directory */
    G__make_mapset_element ( GRASS_VECT_DIRECTORY );
    sprintf ( buf, "%s/%s", GRASS_VECT_DIRECTORY, out );
    G__make_mapset_element ( buf );

    i = 0;
    while ( files[i] )
    {
        sprintf ( buf, "%s/%s", in, files[i] );
	G__file_name (old_path, GRASS_VECT_DIRECTORY, buf, mapset );
        sprintf ( buf, "%s/%s", out, files[i] );
	G__file_name (new_path, GRASS_VECT_DIRECTORY, buf, G_mapset() );

        if ( stat (old_path, &info) == 0)       /* file exists? */
        {
           G_debug (0, "copy %s to %s", old_path, new_path );
            if ( copy_file ( old_path, new_path ) )
            {
	        G_warning ( "Cannot copy vector file '%s' to '%s'", old_path, new_path );
            }
        }
        i++;
    }

    G__file_name (old_path, GRASS_VECT_DIRECTORY, in, mapset );
    G__file_name (new_path, GRASS_VECT_DIRECTORY, out, G_mapset() );

    /* Open input */
    Vect_set_open_level (1);
    Vect_open_old_head (&In, in, mapset);

    if ( In.format != GV_FORMAT_NATIVE ) { /* Done */
	Vect_close ( &In );
	return 0;
    }
    
    /* Open output */
    Vect_open_update_head ( &Out, out, G_mapset() );

    /* Copy tables */
    n = Vect_get_num_dblinks ( &In );
    type = GV_1TABLE;
    if ( n > 1 ) type = GV_MTABLE;
    for ( i = 0; i < n; i++ ) {
	Fi = Vect_get_dblink ( &In, i );
	if ( Fi == NULL ) {
	    G_warning ( "Cannot get db link info" );
	    Vect_close ( &In );
	    Vect_close ( &Out );
	    return -1;
	}
	Fin = Vect_default_field_info ( &Out, Fi->number, Fi->name, type );
        G_debug (3, "Copy drv:db:table '%s:%s:%s' to '%s:%s:%s'", 
	              Fi->driver, Fi->database, Fi->table, Fin->driver, Fin->database, Fin->table );
	Vect_map_add_dblink ( &Out, Fi->number, Fi->name, Fin->table, Fi->key, Fin->database, Fin->driver);
        
	ret = db_copy_table ( Fi->driver, Fi->database, Fi->table, 
		    Fin->driver, Vect_subst_var(Fin->database,&Out), Fin->table );
	if ( ret == DB_FAILED ) {
	    G_warning ( "Cannot copy table" );
	    Vect_close ( &In );
	    Vect_close ( &Out );
	    return -1;
	}

	driver = db_start_driver_open_database ( Fin->driver, Vect_subst_var(Fin->database,&Out) );
	if ( driver == NULL ) {
	    G_warning ( "Cannot open database -> create index" );
	} else {
	    if ( db_create_index2(driver, Fin->table, Fi->key ) != DB_OK )
		G_warning ( "Cannot create index" );

	    db_close_database_shutdown_driver ( driver );
	}
    }
    
    Vect_close ( &In );
    Vect_close ( &Out );

    return 0;
}

/*!
 \fn int Vect_rename ( char *in, char *out, FILE *msgout )
 \brief rename a map, attribute tables are created in the same database where input tables were stored.
        The original format (native/OGR) is used.
	Old map ('out') is deleted!!!
 \return -1 error, 0 success
 \param in input vector
 \param out output vector
 \param msgout output file for messages or NULL 
*/
int 
Vect_rename ( char *in, char *out, FILE *msgout )
{
    int i, n, ret, type;
    struct Map_info Map;
    struct field_info *Fin, *Fout;
    int *fields;
    dbDriver *driver;

    G_debug (2, "Rename vector '%s' to '%s'", in, out );
    /* check for [A-Za-z][A-Za-z0-9_]* in name */
    if (Vect_legal_filename(out) < 0 )
       G_fatal_error ( _("Map name is not SQL compliant.") );

    /* Delete old vector if it exists */
    if ( G_find_vector2(out, G_mapset()) ) {
	G_warning (_("The vector '%s' already exists and will be overwritten."), out);
	Vect_delete ( out );
    }

    /* Move the directory */
    ret = G_rename ( GRASS_VECT_DIRECTORY, in, out );

    if ( ret == 0 ) {
	G_warning (_("Input vector '%s' not found"), in );
	return -1;
    } else if ( ret == -1 ) {
	G_warning (_("Cannot copy vector '%s' to '%s'"), in, out );
	return -1;
    }

    /* Rename all tables if the format is native */
    Vect_set_open_level (1);
    Vect_open_update_head ( &Map, out, G_mapset() );

    if ( Map.format != GV_FORMAT_NATIVE ) { /* Done */
	Vect_close ( &Map );
	return 0;
    }

    /* Copy tables */
    n = Vect_get_num_dblinks ( &Map );
    type = GV_1TABLE;
    if ( n > 1 ) type = GV_MTABLE;

    /* Make the list of fields */
    fields = (int *) G_malloc ( n * sizeof(int) );

    for ( i = 0; i < n; i++ ) {
	Fin = Vect_get_dblink ( &Map, i );

	fields[i] = Fin->number;
    }
    
    for ( i = 0; i < n; i++ ) {
	G_debug (3, "field[%d] = %d", i, fields[i] );
	
	Fin = Vect_get_field ( &Map, fields[i] );
	if ( Fin == NULL ) {
	    G_warning ( "Cannot get db link info" );
	    Vect_close ( &Map );
	    return -1;
	}

	Fout = Vect_default_field_info ( &Map, Fin->number, Fin->name, type );
        G_debug (3, "Copy drv:db:table '%s:%s:%s' to '%s:%s:%s'", 
	              Fin->driver, Fin->database, Fin->table, Fout->driver, Fout->database, Fout->table );

	/* TODO: db_rename_table instead of db_copy_table */
	ret = db_copy_table ( Fin->driver, Fin->database, Fin->table, 
		    Fout->driver, Vect_subst_var(Fout->database,&Map), Fout->table );

	if ( ret == DB_FAILED ) {
	    G_warning ( "Cannot copy table" );
	    Vect_close ( &Map );
	    return -1;
	}

	/* Change the link */
	Vect_map_del_dblink ( &Map, Fin->number );
	
	Vect_map_add_dblink ( &Map, Fout->number, Fout->name, Fout->table, Fin->key, 
		                    Fout->database, Fout->driver);

	/* Delete old table */
	ret = db_delete_table ( Fin->driver, Fin->database, Fin->table );
	if ( ret == DB_FAILED ) {
	    G_warning ( "Cannot delete table" );
	    Vect_close ( &Map );
	    return -1;
	}

	driver = db_start_driver_open_database ( Fout->driver, Vect_subst_var(Fout->database, &Map) );
	if ( driver == NULL ) {
	    G_warning ( "Cannot open database -> create index" );
	} else {
	    if ( db_create_index2(driver, Fout->table, Fin->key ) != DB_OK )
		G_warning ( "Cannot create index" );

	    db_close_database_shutdown_driver ( driver );
	}
    }
    
    Vect_close ( &Map );
    free ( fields );

    return 0;
}

/*!
 \fn int Vect_delete ( char *map )
 \brief delete a map including attribute tables
 \return -1 error, 0 success
 \param  map name
*/
int 
Vect_delete ( char *map )
{
    int i, n, ret;
    struct Map_info Map;
    struct field_info *Fi;
    char   buf[5000];
    DIR    *dir;
    struct dirent *ent; 
    char *tmp;

    G_debug (3, "Delete vector '%s'", map );

    G_chop ( map );

    if ( map == NULL || strlen ( map ) == 0 ) {
	G_warning ( "Wrong map name '%s'", map );
	return -1;
    }

    sprintf ( buf, "%s/%s/%s/%s/%s/%s", G_gisdbase(), G_location(), 
              G_mapset(), GRASS_VECT_DIRECTORY, map, 
              GRASS_VECT_DBLN_ELEMENT );

    G_debug (1, "dbln file: %s", buf);  

    if ( access(buf,F_OK) == 0 )
    {
	/* Open input */
	Vect_set_open_level (1); /* Topo not needed */
	ret = Vect_open_old_head (&Map, map, G_mapset());
	if ( ret < 1 ) {
	    G_warning ( "Cannot open vector %s", map );
	    return -1;
	}

	/* Delete all tables, NOT external (OGR) */
	if ( Map.format == GV_FORMAT_NATIVE ) {

	    n = Vect_get_num_dblinks ( &Map );
	    for ( i = 0; i < n; i++ ) {
		Fi = Vect_get_dblink ( &Map, i );
		if ( Fi == NULL ) {
		    G_warning ( "Cannot get db link info" );
		    Vect_close ( &Map );
		    return -1;
		}
		G_debug (3, "Delete drv:db:table '%s:%s:%s'", Fi->driver, Fi->database, Fi->table);
		
		ret = db_table_exists ( Fi->driver, Fi->database, Fi->table );
		if ( ret == -1 ) {
		    G_warning ( "Cannot get info if table '%s' linked to vector exists.", Fi->table );
		    Vect_close ( &Map );
		    return -1;
		}
		
		if ( ret == 1 ) {
		    ret = db_delete_table ( Fi->driver, Fi->database, Fi->table );
		    if ( ret == DB_FAILED ) {
			G_warning ( "Cannot delete table" );
			Vect_close ( &Map );
			return -1;
		    }
		} else {
		    G_warning ( "Table '%s' linked to vector did not exist.", Fi->table );
		}
	    }
	}
	    
	Vect_close ( &Map );
    }

    /* Delete all files from vector/name directory */
    sprintf ( buf, "%s/%s/vector/%s", G_location_path(), G_mapset(), map );
    G_debug (3, "opendir '%s'", buf ); 
    dir = opendir( buf );
    if (dir == NULL) {
	G_warning ( "Cannot open directory '%s'", buf );
	return -1;
    }

    while ( (ent = readdir (dir)) ) {
	G_debug (3, "file = '%s'", ent->d_name );
	if ( (strcmp (ent->d_name, ".") == 0) || (strcmp (ent->d_name, "..") == 0) ) continue;
	sprintf ( buf, "%s/%s/vector/%s/%s", G_location_path(), G_mapset(), map, ent->d_name );
	G_debug (3, "delete file '%s'", buf );
	ret = unlink ( buf );
	if ( ret == -1 ) { 
	    G_warning ( "Cannot delete file '%s'", buf );
	    closedir (dir);
	    return -1;
	}
    }
    closedir (dir);

    /* NFS can create .nfsxxxxxxxx files for those deleted 
     *  -> we have to move the directory to ./tmp before it is deleted */
    sprintf ( buf, "%s/%s/vector/%s", G_location_path(), G_mapset(), map );

    tmp = G_tempfile();

    G_debug (3, "rename '%s' to '%s'", buf, tmp );
    ret = rename ( buf, tmp );

    if ( ret == -1 ) {
	G_warning ( "Cannot rename directory '%s' to '%s'", buf, tmp );
	return -1;
    }

    G_debug (3, "unlink directory '%s'", tmp );
    ret = unlink ( tmp );

    /* It seems that unlink fails if the directory is not empty (it should not)
       but it is not such problem if temporary file is left there (it is empty) */
    /* 
    if ( ret == -1 ) { 
	G_warning ( "Cannot unlink directory '%s'", tmp );
	return -1;
    }
    */

    return 0;
}

/*!
 \fn int Vect_copy_tables ( struct Map_info *In, struct Map_info *Out, int field )
 \brief Copy map tables. All if field = 0, or table defined by given field if field > 0
 \return 0 on success, -1 on error
 \param  in Map_info structure, out Map_info structure, field number 
*/
int 
Vect_copy_tables ( struct Map_info *In, struct Map_info *Out, int field )
{
    int i, n, ret, type;
    struct field_info *Fi, *Fin;
    dbDriver *driver;

    G_debug (2, "Vect_copy_tables()");

    n = Vect_get_num_dblinks ( In );
    type = GV_1TABLE;
    if ( n > 1 ) type = GV_MTABLE;

    for ( i = 0; i < n; i++ ) {
	Fi = Vect_get_dblink ( In, i );
	if ( Fi == NULL ) {
	    G_warning ( "Cannot get db link info" );
	    return -1;
	}
	if ( field > 0 && Fi->number != field ) continue;

	Fin = Vect_default_field_info ( Out, Fi->number, Fi->name, type );
        G_debug (2, "Copy drv:db:table '%s:%s:%s' to '%s:%s:%s'", 
	              Fi->driver, Fi->database, Fi->table, Fin->driver, Fin->database, Fin->table );
	
	ret = Vect_map_add_dblink ( Out, Fi->number, Fi->name, Fin->table, Fi->key, Fin->database, Fin->driver);
	if ( ret == -1 ) {
	    G_warning ( "Cannot add database link" );
	    return -1;
	}
        
	ret = db_copy_table ( Fi->driver, Fi->database, Fi->table, 
		    Fin->driver, Vect_subst_var(Fin->database,Out), Fin->table );
	if ( ret == DB_FAILED ) {
	    G_warning ( "Cannot copy table" );
	    return -1;
	}

	driver = db_start_driver_open_database ( Fin->driver, Vect_subst_var(Fin->database,Out) );
	if ( driver == NULL ) {
	    G_warning ( "Cannot open database -> create index" );
	} else {
	    if ( db_create_index2(driver, Fin->table, Fi->key ) != DB_OK )
		G_warning ( "Cannot create index" );

	    db_close_database_shutdown_driver ( driver );
	}
    }

    return 0;
}

/*!
 \fn int Vect_copy_table ( struct Map_info *In, struct Map_info *Out, int field_in, 
                           int field_out, char *field_name, int type )
 \brief Copy map table.
 \return 0 on success, -1 on error
 \param In 
 \param Out
 \param field_in
 \param field_out
 \param field_name 
 \param type
*/
int 
Vect_copy_table ( struct Map_info *In, struct Map_info *Out, int field_in, 
	           int field_out,  char *field_name, int type )
{
    return Vect_copy_table_by_cats ( In, Out, field_in, field_out, field_name, type, NULL, 0); 
}

/*!
 \fn int Vect_copy_table_by_cats ( struct Map_info *In, struct Map_info *Out, int field_in, 
                           int field_out, char *field_name, int type, int *cats, int ncats )
 \brief Copy map table.
 \return 0 on success, -1 on error
 \param In 
 \param Out
 \param field_in
 \param field_out
 \param field_name 
 \param type
 \param cats pointer to array of cats or NULL
 \param ncats number of cats in 'cats'
*/
int 
Vect_copy_table_by_cats ( struct Map_info *In, struct Map_info *Out, int field_in, 
	           int field_out,  char *field_name, int type, int *cats, int ncats )
{
    int    ret;
    struct field_info *Fi, *Fin;
    char   *name, *key;

    G_debug (2, "Vect_copy_table(): field_in = %d field_out = %d", field_in, field_out);

    Fi = Vect_get_field ( In, field_in );
    if ( Fi == NULL ) {
	G_warning ( "Cannot get db link info" );
	return -1;
    }

    if ( field_name != NULL ) name = field_name;
    else name = Fi->name;
    
    Fin = Vect_default_field_info ( Out, field_out, name, type );
    G_debug (3, "Copy drv:db:table '%s:%s:%s' to '%s:%s:%s'", 
		  Fi->driver, Fi->database, Fi->table, Fin->driver, Fin->database, Fin->table );
    
    ret = Vect_map_add_dblink ( Out, Fin->number, Fin->name, Fin->table, Fi->key, Fin->database, Fin->driver);
    if ( ret == -1 ) {
	G_warning ( "Cannot add database link" );
	return -1;
    }
    
    if ( cats ) 
	key = Fi->key;
    else 
	key = NULL;
    
    ret = db_copy_table_by_ints ( Fi->driver, Fi->database, Fi->table, 
		Fin->driver, Vect_subst_var(Fin->database,Out), Fin->table, key, cats, ncats );
    if ( ret == DB_FAILED ) {
	G_warning ( "Cannot copy table" );
	return -1;
    }

    return 0;
}

/*!
  \brief Set spatial index to be realease when vector is closed, by default, the memory
         occupied by spatial index is not released.
  \param Map pointer to map
*/
void
Vect_set_release_support ( struct Map_info * Map )
{
    Map->plus.release_support = 1;
}

/*!
  \brief By default, category index is not updated if vector is changed, 
         this function sets category index update.
	 WARNING: currently only category for elements is updated 
	 not for areas
  \param Map pointer to map
*/
void
Vect_set_category_index_update ( struct Map_info * Map )
{
    Map->plus.update_cidx = 1;
}

