#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gis.h"
#include "dbmi.h"
#include "Vect.h"
#include "local_proto.h"

#define	A_DIR	"dig_ascii"

int 
main (int argc, char *argv[])
{
	FILE *ascii;
	struct GModule *module;
	struct Option *old, *new, *delim_opt, *columns_opt, *xcol_opt, 
		*ycol_opt, *zcol_opt, *catcol_opt;
	int    xcol, ycol, zcol, catcol;
	struct Flag *zcoorf, *t_flag, *e_flag;
	char   *mapset, *table;
	char   errmsg[200];
	char   *fs;
	int    zcoor=WITHOUT_Z, points_format, make_table; 

	struct Map_info Map;

	G_gisinit(argv[0]);

	module = G_define_module();
	module->description = "Convert GRASS ascii file or points file to binary vector. "
	    "If option 'input' is not used, the module read points from standard input ('points mode').";

        /************************** Command Parser ************************************/
	old = G_define_option();
	old->key	 = "input";
	old->type	 =  TYPE_STRING;
	old->required	 =  NO;
	old->multiple	 =  NO;
	old->gisprompt   = "old,dig_ascii,ascii vector";
	old->description = "ascii file to be converted to binary vector file";

	new = G_define_standard_option(G_OPT_V_OUTPUT);

	delim_opt = G_define_option();
	delim_opt->key = "fs";
	delim_opt->key_desc = "character|space|tab";
	delim_opt->type = TYPE_STRING;
	delim_opt->required = NO;
	delim_opt->description = "field separator";
	delim_opt->answer = "|";

	columns_opt = G_define_option();
	columns_opt->key = "columns";
	columns_opt->type = TYPE_STRING;
	columns_opt->required = NO;
	columns_opt->multiple = NO;
	columns_opt->description = "Columns definition for points mode in SQL style, for example:\n"
	    "'x double precision, y double precision, cat int, name varchar(10)'";

	xcol_opt = G_define_option();
	xcol_opt->key = "xcol";
	xcol_opt->type = TYPE_INTEGER;
	xcol_opt->required = NO;
	xcol_opt->multiple = NO;
	xcol_opt->answer = "1";
	xcol_opt->description = "Number of column used as x coordinate (first column is 1) for points mode.";

	ycol_opt = G_define_option();
	ycol_opt->key = "ycol";
	ycol_opt->type = TYPE_INTEGER;
	ycol_opt->required = NO;
	ycol_opt->multiple = NO;
	ycol_opt->answer = "2";
	ycol_opt->description = "Number of column used as y coordinate (first column is 1) for points mode.";

	zcol_opt = G_define_option();
	zcol_opt->key = "zcol";
	zcol_opt->type = TYPE_INTEGER;
	zcol_opt->required = NO;
	zcol_opt->multiple = NO;
	zcol_opt->answer = "0";
	zcol_opt->description = "Number of column used as z coordinate (first column is 1) for points mode. "
	            "If 0, z coordinate is not used.";

	catcol_opt = G_define_option();
	catcol_opt->key = "catcol";
	catcol_opt->type = TYPE_INTEGER;
	catcol_opt->required = NO;
	catcol_opt->multiple = NO;
	catcol_opt->answer = "0";
	catcol_opt->description = "Number of column used as category (first column is 1) for points mode. "
	            "If 0, unique category is assigned to each row and written to new column 'cat'.";

	zcoorf = G_define_flag ();
        zcoorf->key           	= 'z';
	zcoorf->description   	= "create 3D file";  

	t_flag = G_define_flag();
	t_flag->key              = 't';
	t_flag->description      = "Do not create table in points mode.";

	e_flag = G_define_flag();
	e_flag->key              = 'e';
	e_flag->description      = "Create a new empty map and exit. Nothing is read from input.";
	
	if (G_parser (argc, argv))
		exit(-1);
	if (zcoorf->answer && zcol_opt->answer == "0")
		G_fatal_error("Please specify zcol.");

	xcol = atoi(xcol_opt->answer) - 1;
	ycol = atoi(ycol_opt->answer) - 1;
	zcol = atoi(zcol_opt->answer) - 1;
	if (zcoorf->answer && zcol < 0)
		G_fatal_error("Please specify reasonable zcol.");
	catcol = atoi(catcol_opt->answer) - 1;

	if ( !*(new->answer) )
	{
	    fprintf (stderr, "%s: Command line error: missing output name.\n\n", argv[0]);
		    G_usage();
	    exit (-1);
	}
        /*****************************************************************************/
        
	if ( old->answer != NULL ) {
	    if ((mapset = G_find_file2 (A_DIR, old->answer, "")) == NULL)
	    {
		    sprintf (errmsg, "Could not find ascii file <%s>\n", old->answer);
		    G_fatal_error (errmsg);
	    }
	    if ( (ascii = G_fopen_old (A_DIR, old->answer, mapset) ) == NULL )
	    {
		    sprintf(errmsg, "Could not open ascii file <%s>\n", old->answer);
		    G_fatal_error (errmsg);
	    }
	    points_format = 0;
        } else { /* write points to stdout */
	    points_format = 1;
        }

	if (  (fs = delim_opt->answer) ) {
	    if(strcmp (fs, "space") == 0)
		fs = " ";
	    else if(strcmp (fs, "tab") == 0)
		fs = "\t";
	}
	else
	    fs = "|";

	/* check dimension */
	if (zcoorf->answer) {
	    zcoor = 1;	    
	}

	Vect_open_new (&Map, new->answer, zcoor);
	Vect_hist_command ( &Map );

	if ( e_flag->answer ) { 
	    Vect_build ( &Map, stdout );
	    Vect_close ( &Map );
	    exit(0) ;
	}

	if ( points_format ) {
	    int i, rowlen, ncols, minncols, *coltype, *collen;
	    int n_int = 0, n_double = 0, n_string = 0;
	    char buf[1000];
	    struct field_info *Fi;
	    char *tmp, *key;
	    dbDriver *driver;
	    dbString sql;

	    /* Open temporary file */
	    tmp = G_tempfile();
	    if (NULL == (ascii = fopen(tmp, "w+"))) {
		G_fatal_error ( "Can't open temp file %s", tmp);
	    }
	    unlink(tmp);

	    points_analyse ( stdin, ascii, fs, PNT_HEAD_NO, &rowlen, &ncols, &minncols, &coltype, &collen);
	    fprintf ( stderr, "Maximum input row length: %d\n", rowlen);
	    fprintf ( stderr, "Maximum number of columns: %d\n", ncols);
	    fprintf ( stderr, "Minimum number of columns: %d\n", minncols);

	    /* check column numbers */
	    if ( xcol >= minncols ) G_fatal_error ( "xcol > minimum last column number"); 
	    if ( ycol >= minncols ) G_fatal_error ( "ycol > minimum last column number"); 
	    if ( zcol >= minncols ) G_fatal_error ( "zcol > minimum last column number"); 
	    if ( catcol >= minncols ) G_fatal_error ( "catcol > minimum last column number"); 

	    if ( coltype[xcol] == DB_C_TYPE_STRING ) G_fatal_error ( "xcol is not a number type");
	    if ( coltype[ycol] == DB_C_TYPE_STRING ) G_fatal_error ( "ycol is not a number type");
	    if ( zcol >= 0 && coltype[zcol] == DB_C_TYPE_STRING ) G_fatal_error ( "zcol is not a number type");
	    if ( catcol >= 0 && coltype[catcol] == DB_C_TYPE_STRING ) G_fatal_error ( "catcol is not a number type");

	    /* Create table */
	    make_table = 0;
	    for ( i = 0 ; i < ncols; i++ ) {
		if ( xcol != i && ycol != i && zcol != i && catcol != i ) {
		    make_table = 1;
		    break;
		}
	    }
	    if ( t_flag->answer ) {
		make_table = 0;
	    }

	    if ( make_table ) {	
		Fi = Vect_default_field_info ( &Map, 1, NULL, GV_1TABLE );
		driver = db_start_driver_open_database ( Fi->driver, Vect_subst_var(Fi->database,&Map) );
		if ( driver == NULL ) {
		    G_fatal_error ( "Cannot open database %s by driver %s",
			    Vect_subst_var(Fi->database,&Map), Fi->driver );
		}
		db_begin_transaction ( driver );

		db_init_string (&sql);
		sprintf ( buf, "create table %s ( ", Fi->table );
		db_append_string ( &sql, buf );

		if ( catcol < 0 ) { 
		    db_append_string ( &sql, "cat integer, " );
		}

		for ( i = 0 ; i < ncols; i++ ) {
		    fprintf ( stderr, "column: %d  type: ", i+1);
		    if ( i > 0 && !columns_opt->answer ) {
			db_append_string ( &sql, ", " );
		    }
		    if ( catcol == i && coltype[i] != DB_C_TYPE_INT ) 
			G_fatal_error ("Category column is not integer");

		    switch ( coltype[i] ) {
			case DB_C_TYPE_INT:
			    fprintf ( stderr, "integer\n");
			    if ( !columns_opt->answer ) {
				sprintf ( buf, "int_%d integer", n_int+1);
				db_append_string ( &sql, buf);
				if ( catcol == i ) {
				    sprintf ( buf, "int_%d", n_int+1);
				    key = G_store( buf );
				}
			    }
			    n_int++;
			    break;
			case DB_C_TYPE_DOUBLE:
			    fprintf ( stderr, "double\n");
			    if ( !columns_opt->answer ) {
				sprintf ( buf, "dbl_%d double precision", n_double+1);
				db_append_string ( &sql, buf);
			    }
			    n_double++;
			    break;
			case DB_C_TYPE_STRING:
			    fprintf ( stderr, "string  length: %d\n", collen[i]);
			    if ( !columns_opt->answer ) {
				sprintf ( buf, "str_%d varchar(%d)", n_string+1, collen[i]);
				db_append_string ( &sql, buf);
			    }
			    n_string++;
			    break;
		    }
		}
		if ( columns_opt->answer ) {
		    db_append_string ( &sql, columns_opt->answer );
		}
		db_append_string ( &sql, " )" );
		
		/* this link is added with default 'cat' key, later deleted and replaced by true key name,
		 * otherwise if module crashes when the table exists but link is not written it makes troubles */ 
		Vect_map_add_dblink ( &Map, 1, NULL, Fi->table, "cat", Fi->database, Fi->driver);

		/* Create table */
		G_debug ( 3, db_get_string ( &sql ) );
		if (db_execute_immediate (driver, &sql) != DB_OK ) {
		    G_fatal_error ( "Cannot create table: %s", db_get_string ( &sql )  );
		}

		/* Check column types */
		if ( columns_opt->answer ) {
		    int nc;
		    dbTable *table;
		    dbColumn *column;
		    
		    db_set_string ( &sql, Fi->table );
		    if(db_describe_table (driver, &sql, &table) != DB_OK)
			G_fatal_error ("Cannot describe table %s", Fi->table);
				
		    nc = db_get_table_number_of_columns(table);

		    if ( (catcol >= 0 && nc != ncols) || (catcol < 0 && (nc-1) != ncols) ) {
			G_fatal_error ("Number of columns defined (%d) does not match number of columns (%d) "
				"in input", nc, ncols);
		    }

		    for ( i = 0; i < ncols; i++ ) {
			int dbcol, ctype, length;

			if ( catcol < 0 ) dbcol = i+1; /* first is category */
			else dbcol = i;
			
			column = db_get_table_column ( table, dbcol );
			ctype =  db_sqltype_to_Ctype ( db_get_column_sqltype(column) );
			length = db_get_column_length ( column );
			
			if ( catcol == i ) { /* if catcol == -1 it cannot be tru */
			    key = G_store(db_get_column_name(column));
			}

			switch ( coltype[i] ) {
			    case DB_C_TYPE_INT:
				if ( ctype == DB_C_TYPE_DOUBLE ) {
				    G_warning ( "Column %d defined as double has only integer values", i+1);
				} else if ( ctype == DB_C_TYPE_STRING ) {
				    G_warning ( "Column %d defined as string has only integer values", i+1);
				}
				break;
			    case DB_C_TYPE_DOUBLE:
				if ( ctype == DB_C_TYPE_INT ) {
				    G_fatal_error ( "Column %d defined as integer has double values", i+1);
				} else if ( ctype == DB_C_TYPE_STRING ) {
				    G_fatal_error ( "Column %d defined as string has double values", i+1);
				}
				break;
			    case DB_C_TYPE_STRING:
				if ( ctype == DB_C_TYPE_INT ) {
				    G_fatal_error ( "Column %d defined as integer has string values", i+1);
				} else if ( ctype == DB_C_TYPE_DOUBLE ) {
				    G_fatal_error ( "Column %d defined as double has string values", i+1);
				}
				if ( length < collen[i] ) {
				    G_fatal_error ( "Length of column %d (%d) is less than maximum value "
						    "length (%d)", i+1, length, collen[i]);
				}
				break;
			}
		    }
		}

		if ( catcol < 0 ) {
		   key = "cat";	
		} else if ( !columns_opt->answer ) {
		    

		}
		Vect_map_del_dblink ( &Map, 1 );
		Vect_map_add_dblink ( &Map, 1, NULL, Fi->table, key, Fi->database, Fi->driver);
		table = Fi->table;
	    } else { 
		driver = NULL;
		table = NULL;
	    }

	    points_to_bin ( ascii, rowlen, &Map, driver, table, fs, PNT_HEAD_NO, 
		            ncols, coltype,  
		            xcol, ycol, zcol, catcol );

	    if ( driver ) {
	        db_commit_transaction ( driver );
	        db_close_database_shutdown_driver ( driver );
	    }
	    fclose (ascii);
	} else {
    	    read_head(ascii, &Map);
	    asc_to_bin(ascii, &Map) ;
	    fclose(ascii) ;
	}
        
	Vect_build ( &Map, stdout );
	Vect_close ( &Map );

	exit(0) ;
}

