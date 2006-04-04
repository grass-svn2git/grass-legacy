/****************************************************************************
 *
 * MODULE:       v.patch
 * AUTHOR(S):    Dave Gerdes, U.S.Army Construction Engineering Research Laboratory (original contributor)
 *               Radim Blazek <radim.blazek gmail.com> (update to GRASS 6)
 *               Glynn Clements <glynn gclements.plus.com>, Markus Neteler <neteler itc.it>
 * PURPOSE:      
 * COPYRIGHT:    (C) 2002-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/
/*
**  v.patch  input=file1,file2,.... output=composite
**
**   patch 2 or more vector files together creating composite
**
**
**  no checking is done for overlapping lines.
**  header information will have to be editted afterwards.
*/

/*
**  Written by Dave Gerdes  8/1988, US Army Construction Engineering Research Lab
**  Upgrade to 5.7 Radim Blazek
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>

int patch(struct Map_info *, struct Map_info *, int, int *);
int copy_records ( dbDriver *driver_in, dbString *table_name_in,
                   dbDriver *driver_out, dbString *table_name_out,
                   int, int );
int max_cat ( struct Map_info *Map, int layer );

int 
main (int argc, char *argv[])
{
    int i, ret;
    char *in_name, *out_name;
    struct GModule *module;
    struct Option *old, *new;
    struct Flag *append, *table_flag;
    struct Map_info InMap, OutMap;
    int  n_files;
    int do_table;
    struct field_info *fi_in, *fi_out;
    dbString sql, table_name_in, table_name_out;
    dbDriver *driver_in, *driver_out;
    dbTable *table_in, *table_out;
    char *key = NULL;
    int keycol = -1;
    int maxcat = 0;

    G_gisinit (argv[0]);

    module = G_define_module();
    module->description = "Creates a new binary vector map layer "
			  "by combining other binary vector map layers.";

    old = G_define_option();
    old->key		= "input";
    old->type		= TYPE_STRING;
    old->required		= YES;
    old->multiple		= YES;
    old->gisprompt		= "old,vector,vector";
    old->description	= "vector map(s)--source for composite";

    new = G_define_option();
    new->key		= "output";
    new->type		= TYPE_STRING;
    new->required		= YES;
    new->multiple		= NO;
    new->gisprompt		= "any,vector,vector";
    new->description	= "new vector composite";

    append = G_define_flag();
    append->key = 'a';
    append->description = "Append files to existing file";

    table_flag = G_define_flag();
    table_flag->key = 'e';
    table_flag->description = "Copy also attribute table. "
          "Only the table of layer 1 is currently supported";
    
    if (G_parser(argc, argv)) exit (EXIT_FAILURE);

    out_name = new->answer;
    do_table = table_flag->answer;

    db_init_string(&table_name_in);
    db_init_string(&table_name_out);
    db_init_string(&sql);

    /* TODO: check first if any of input is 3D and open output as 3D */
    i=0;
    while (old->answers[i]) {
	in_name = old->answers[i++];
	Vect_check_input_output_name ( in_name, new->answer, 
				       GV_FATAL_EXIT );
    }

    table_out = NULL;
    /* Check input table structures */
    if ( do_table ) 
    {
	if ( append->answer  ) 
	{
	    Vect_set_open_level (1);
	    Vect_open_old_head ( &OutMap, out_name, G_mapset() );

	    fi_out = Vect_get_field ( &OutMap, 1 );
	    if ( fi_out ) 
	    {
                key = G_store ( fi_out->key );
		driver_out = db_start_driver_open_database ( 
			      fi_out->driver, fi_out->database );
		if ( ! driver_out )
		{
		    G_fatal_error ( "Cannot open database %s "
		     "by driver %s", fi_out->database, fi_out->driver );
		}

		db_set_string(&table_name_out,fi_out->table);
		if ( db_describe_table (driver_out, &table_name_out, &table_out) 
		     != DB_OK) 
		{
		    G_fatal_error ( "Cannot describe table %s", 
				    fi_out->table);
		}
		db_close_database_shutdown_driver ( driver_out );
	    }
	    Vect_close ( &OutMap );
	}

	i=0;
	while (old->answers[i]) 
	{
	    in_name = old->answers[i];
	    Vect_set_open_level (1);
	    Vect_open_old_head ( &InMap, in_name, "" );

	    fi_in = Vect_get_field ( &InMap, 1 );
	    table_in = NULL;
	    if ( fi_in ) 
	    {
		dbTable **table;
		driver_in = db_start_driver_open_database ( 
			      fi_in->driver, fi_in->database );
		if ( ! driver_in )
		{
		    G_fatal_error ( "Cannot open database %s "
		     "by driver %s", fi_in->database, fi_in->driver );
		}

		if ( !append->answer && i == 0 )
                { 
		    table = &table_out;
                    key = G_store ( fi_in->key );
                }
		else
                {
		    table = &table_in;
                }

		db_set_string(&table_name_in,fi_in->table);
		if ( db_describe_table (driver_in, &table_name_in, table) 
		     != DB_OK) 
		{
		    G_fatal_error ( "Cannot describe table %s", 
				    fi_in->table);
		}
		db_close_database_shutdown_driver ( driver_in );
	    }

	    /* Check the structure */
	    if ( i > 0 || append->answer ) 
	    {
		int ncols, col;

		if ( (table_out && !table_in) 
		     || (!table_out && table_in) )
		{
		    G_fatal_error ( "Missing table" );
		}

                if ( G_strcasecmp( fi_in->key, key ) != 0 )
                {
		    G_fatal_error ( "Key columns differ." );
                }

		ncols = db_get_table_number_of_columns(table_out);

		if ( ncols != db_get_table_number_of_columns(table_in ) )
		{
		    G_fatal_error ( "Number of columns differ." );
		}

		for (col = 0; col < ncols; col++)
		{
		    dbColumn *column_out, *column_in;
		    int ctype_in, ctype_out;
		   
		    column_in = db_get_table_column (table_in, col);
		    column_out = db_get_table_column (table_out, col);
		     
		    if ( G_strcasecmp( db_get_column_name(column_in),
			 db_get_column_name(column_out) ) != 0 )
		    {
			G_fatal_error ( "Column names differ." );
		    }
		    ctype_in = db_sqltype_to_Ctype( db_get_column_sqltype (column_in));
		    ctype_out = db_sqltype_to_Ctype( db_get_column_sqltype (column_out));
		    if ( ctype_in != ctype_out )
		    {
			G_fatal_error ( "Column types differ." );
		    }
		    if ( ctype_in == DB_C_TYPE_STRING &&
			 db_get_column_length(column_in) !=
			 db_get_column_length(column_out) )
		    {
			G_fatal_error ( "Length of string columns differ." );
		    }
		    if ( G_strcasecmp( key,
			 db_get_column_name(column_out) ) == 0 )
		    {
                        keycol = col;
                    }
	       }
	    }

	    Vect_close ( &InMap );
	    i++;
	}

        if ( keycol == -1 )
        {
            G_fatal_error ( "Key column not found" );
        }
    }

    if ( append->answer ) {
	Vect_open_update ( &OutMap, out_name,  G_mapset() );
        maxcat = max_cat ( &OutMap, 1 );
    } else {
	Vect_open_new (&OutMap, out_name, 0);
    }

    Vect_hist_command ( &OutMap );

    driver_out = NULL;
    if ( do_table ) 
    {
	if ( append->answer ) {
	    fi_out = Vect_get_field ( &OutMap, 1 );
	} else {
            fi_out = Vect_default_field_info ( &OutMap, 1, NULL, GV_1TABLE ); 
            fi_out->key = key;
	}
	if ( fi_out ) 
	{
	    driver_out = db_start_driver_open_database ( 
			  fi_out->driver, fi_out->database );
	    if ( ! driver_out )
	    {
		G_fatal_error ( "Cannot open database %s "
		 "by driver %s", fi_out->database, fi_out->driver );
	    }
            db_begin_transaction ( driver_out );
	}

        db_set_string ( &table_name_out, fi_out->table );
        db_set_table_name ( table_out, fi_out->table );

        if ( !append->answer ) {
            if ( db_create_table ( driver_out, table_out ) != DB_OK ) 
            {
                G_fatal_error ( "Cannot create new table" );
            }
            Vect_map_add_dblink ( &OutMap, 1, NULL, fi_out->table, 
                                  fi_in->key, fi_out->database, 
                                  fi_out->driver);
        }
    }

    i = 0;
    while (old->answers[i]) 
    {
        int add_cat;
	in_name = old->answers[i++];
	fprintf (stdout, "    Patching file %s\n", in_name);
	Vect_set_open_level (1);
	Vect_open_old ( &InMap, in_name, "" );

	/*first time around, copy first in head to out head*/
	if (i == 1) 
	   Vect_copy_head_data ( &InMap, &OutMap);

        if ( do_table )
        {
            add_cat = maxcat+1;
        }
        else
        {
            add_cat = 0;
        }
        G_debug ( 2, "maxcat = %d add_cat = %d", maxcat, add_cat);
            
	ret = patch (&InMap, &OutMap, add_cat, &maxcat);
	if (ret < 0)
	    G_warning ( "Error reading file '%s'." 
		      "Some data may not be correct\n", in_name);

        if ( do_table ) 
        {
	    fi_in = Vect_get_field ( &InMap, 1 );
	    if ( fi_in ) 
	    {
		driver_in = db_start_driver_open_database ( 
			      fi_in->driver, fi_in->database );
		if ( ! driver_in )
		{
		    G_fatal_error ( "Cannot open database %s "
		     "by driver %s", fi_in->database, fi_in->driver );
		}

		db_set_string(&table_name_in,fi_in->table);
                copy_records ( driver_in, &table_name_in,
                               driver_out, &table_name_out,
                               keycol, add_cat ); 

		db_close_database_shutdown_driver ( driver_in );
	    }
     
        }

	Vect_close (&InMap);
    }
    n_files = i;

    if ( driver_out ) 
    {
        db_commit_transaction ( driver_out );
        db_close_database_shutdown_driver ( driver_out );
    } 
    
    Vect_set_map_name ( &OutMap, "Output from v.patch");
    Vect_set_person ( &OutMap, G_whoami ());

    Vect_build (&OutMap, stdout);
    Vect_close (&OutMap);

    fprintf (stderr, "Patch complete. %d files patched.\n", n_files);
    fprintf (stderr, "Intersections at borders will have to be snapped.\n");
    fprintf (stderr, "Lines common between files will have to be edited.\n");
    fprintf (stderr, "The header information also may have to be edited.\n");

    exit (EXIT_SUCCESS);
}


int copy_records ( dbDriver *driver_in, dbString *table_name_in,
                   dbDriver *driver_out, dbString *table_name_out,
                   int keycol, int add_cat )
{
    int ncols, col;
    dbCursor cursor;
    dbString value_str, sql;
    dbTable *table_in;

    db_init_string(&value_str);
    db_init_string(&sql);

    db_set_string ( &sql, "select * from ");
    db_append_string ( &sql, db_get_string(table_name_in) );

    if (db_open_select_cursor(driver_in, &sql, &cursor, DB_SEQUENTIAL) != DB_OK) {
        G_warning ( "Cannot open select cursor: '%s'", db_get_string(&sql) );
        return 0;
    }
    table_in = db_get_cursor_table (&cursor);
    ncols = db_get_table_number_of_columns(table_in);

    while ( 1 ) 
    {
        int more;
        char buf[2000];

        if ( db_fetch (&cursor, DB_NEXT, &more ) != DB_OK ) {
            G_fatal_error ( "Cannot fetch row" );
            db_close_cursor(&cursor);
        }
        if (!more) break;

        sprintf ( buf, "insert into %s values ( ", 
                         db_get_string(table_name_out) );
        db_set_string ( &sql, buf);

        for ( col = 0; col < ncols; col++ ) 
        {
            int ctype, sqltype;
            dbColumn *column;
            dbValue *value;
            column = db_get_table_column (table_in, col); 

            sqltype = db_get_column_sqltype (column);
            ctype = db_sqltype_to_Ctype(sqltype);
            value  = db_get_column_value(column);

            if ( col > 0 ) db_append_string ( &sql, ", " );

            if ( col == keycol )
            {
                db_set_value_int(value,db_get_value_int(value) + add_cat );
            }
            db_convert_value_to_string( value, sqltype, &value_str);

            switch ( ctype ) 
            {
                case DB_C_TYPE_STRING:
                case DB_C_TYPE_DATETIME:
                    if ( db_test_value_isnull(value) ) {
                        db_append_string ( &sql, "null" );
                    } else {
                        db_double_quote_string ( &value_str );
                        sprintf (buf, "'%s'", db_get_string(&value_str) );
                        db_append_string ( &sql, buf);
                    }
                    break;
                case DB_C_TYPE_INT:
                case DB_C_TYPE_DOUBLE:
                    if ( db_test_value_isnull(value) ) {
                        db_append_string ( &sql, "null" );
                    } else {
                        db_append_string ( &sql, db_get_string(&value_str) );
                    }
                    break;
                default:
                    G_fatal_error ( "Unknown column type" );
            }
        }
        db_append_string ( &sql, ")" );

        G_debug ( 2, "SQL: %s", db_get_string(&sql) );

        if (db_execute_immediate (driver_out, &sql) != DB_OK ) 
        {
            G_fatal_error ( "Cannot insert new record: '%s'", 
                             db_get_string(&sql) );
        }
    }
            
    db_close_cursor(&cursor);

    return 1;
} 

int patch ( struct Map_info *InMap, struct Map_info *OutMap, int add_cat, int *max_cat)
{
    int type;
    struct line_pnts *Points;
    struct line_cats *Cats;

    *max_cat = add_cat;

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    /* TODO:	
    OutMap->head.orig_scale = GREATER (OutMap->head.orig_scale, InMap->head.orig_scale);
    OutMap->head.digit_thresh = 0;
    OutMap->head.map_thresh = GREATER (OutMap->head.map_thresh, InMap->head.map_thresh);
    */

    while ( (type = Vect_read_next_line (InMap, Points, Cats)) > 0) 
    {
        int i;
        for ( i = 0; i< Cats->n_cats; i++ ) 
        {
            if ( Cats->field[i] == 1 )
            {
                Cats->cat[i] += add_cat;
                if ( Cats->cat[i] > *max_cat )
                    *max_cat = Cats->cat[i];
            }
        }

	Vect_write_line (OutMap, type, Points, Cats);
    }

    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);

    if (type != -2) return (-1);

    return (0);
}

int max_cat ( struct Map_info *Map, int layer )
{
    struct line_cats *Cats;
    int max = 0;
    
    Cats = Vect_new_cats_struct();

    while ( Vect_read_next_line (Map, NULL, Cats) > 0) 
    {
        int i;
        for ( i = 0; i< Cats->n_cats; i++ ) 
        {
            if ( Cats->field[i] == layer && Cats->cat[i] > max )
            {
                max = Cats->cat[i];
            }
        }
    }
    return max;
}



