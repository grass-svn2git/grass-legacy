/* TODO: LL_TEST section commented as it doesn't always work */

#include <stdio.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"
#include "Vect.h"
#include "local_proto.h"

/* Determine if the string is integer, e.g. 123, +123, -123
 * return 1 if integer, 0 otherwise */
int is_int ( char *str ) 
{
    int i = -1;
    while ( str[++i] != '\0' ) {
	if ( i == 0 && (str[i] == '+' || str[i] == '-') ) continue;
	if ( !isdigit(str[i]) ) return 0;
    }
    return 1;
}

/* Determine if the string is double, e.g. 123.456, +123.456, -123.456
 * return 1 if double, 0 otherwise */
int is_double ( char *str ) 
{
    int i = -1, ndots = 0;
    while ( str[++i] != '\0' ) {
	if ( i == 0 && (str[i] == '+' || str[i] == '-') ) continue;
	if ( str[i] == '.' ) {
	    if ( ndots > 0 ) return 0; /* > 1 dot */
	    ndots++;
	    continue;
	}
	if ( !isdigit(str[i]) ) return 0;
    }
    return 1;
}

/* Analyse points ascii file. Determine number of columns and column types.
 * ascii_tmp: write copy of tempfile to ascii_tmp:
 * rowlength: maximum row length
 * ncolumns: number of columns
 * minncolumns: minimum number of columns
 * column_type: column types
 * column_length: column lengths (string only)
 */

int points_analyse ( FILE *ascii_in, FILE *ascii, char *fs,
	             int *rowlength, int *ncolumns, int *minncolumns, 
		     int **column_type, int **column_length, int skip_lines )
{
    int i;
    int buflen; /* buffer length */
    char *buf;   /* buffer */
    int row = 1;    /* line number, first is 1 */
    int ncols = 0; /* number of columns */
    int minncols = -1;
    int *coltype = NULL; /* column types */
    int *collen = NULL; /* column lengths */
    char **tokens;
    int ntokens;   /* number of tokens */
    int len, rowlen = 0; /* maximum row length */
    struct Cell_head window;
    double northing=.0;
    double easting=.0;
    char *coorbuf, *tmp_token;

    buflen = 1000;
    buf = (char *) G_malloc ( buflen );
    coorbuf=(char *) G_malloc(256);
    tmp_token=(char *) G_malloc(256);

    /* fetch projection for LatLong test */
    if (G__get_window (&window, "", "WIND", G_mapset()) != NULL){
        G_get_default_window (&window);
    }

    while (1) {
	len = 0; /* not really needed, but what the heck */

	if( G_getl2(buf, buflen-1, ascii_in) == 0 )
	    break; /* EOF */

	if(row <= skip_lines) {
	    G_debug (3, "skipping header row %d : %d chars", row, strlen(buf));
	    /* this fn is read-only, write to hist with points_to_bin() */
	    fprintf(ascii, "%s\n", buf);
	    len = strlen ( buf ) + 1;
	    if (len > rowlen ) rowlen = len;

	    row++;
	    continue;
	}

	if(buf[0] == '#') {
	    G_debug (3, "skipping comment row %d : %d chars", row, strlen(buf));
	    continue;
	}

	/* no G_chop() as first/last column may be empty fs=tab value */
	G_debug (3, "row %d : %d chars", row, strlen(buf));
	fprintf(ascii, "%s\n", buf);
	len = strlen ( buf ) + 1;
	if (len > rowlen ) rowlen = len;

	tokens = G_tokenize (buf, fs);
	ntokens = G_number_of_tokens ( tokens );
	if ( ntokens > ncols ) { 
	    int c;
	    coltype = (int *) G_realloc ( coltype, ntokens * sizeof(int) );
	    collen = (int *) G_realloc ( collen, ntokens * sizeof(int) ); 
	    for ( c = ncols; c < ntokens; c++ ) {
		coltype[c] = DB_C_TYPE_INT; /* default type */
		collen[c] = 0;
	    }
	    ncols = ntokens;
	}

	if ( minncols == -1 || minncols > ntokens ) minncols = ntokens; 

	/* Determine column types */
	for ( i = 0; i < ntokens; i++ ) {
#ifdef LL_TEST
/* doesn't work yet under certain conditions */
	    if ((G_projection() == PROJECTION_LL)){
	       /* check if coordinates are DMS or decimal or not latlong at all */
	       sprintf(coorbuf,"%s", tokens[i]);
	       G_debug (4, "token: %s", coorbuf);
	       if (G_scan_northing ( coorbuf, &northing, window.proj) ){
                   G_debug (4, "is_latlong north: %f", northing);
		   sprintf(tmp_token, "%f", northing);
		   /* replace current DMS token by decimal degree */
		   tokens[i]=tmp_token;
               }else{
	        if (G_scan_easting ( coorbuf, &easting, window.proj) ){
	           G_debug (4, "is_latlong east: %f", easting);
		   sprintf(tmp_token, "%f", easting);
		   /* replace current DMS token by decimal degree */
		   tokens[i]=tmp_token;
	        } else{
	           /* maybe do nothing here */
		   G_warning("Unparsable LatLong value found: %s", tokens[i]);
		}
               }
	    }
#endif
	    G_debug (4, "row %d col %d: '%s' is_int = %d is_double = %d", 
		         row, i, tokens[i], is_int(tokens[i]), is_double(tokens[i]) );
	    if ( is_int(tokens[i]) ) continue; /* integer */
	    if ( is_double(tokens[i]) ) { /* double */
		if ( coltype[i] == DB_C_TYPE_INT ) {
		     coltype[i] = DB_C_TYPE_DOUBLE;
		}
	       	continue;
	    }
            /* string */
	    coltype[i] = DB_C_TYPE_STRING;
	    len = strlen (tokens[i]);
	    if ( len > collen[i] ) collen[i] = len;
	}
	G_free_tokens(tokens);

	row++;
    }

    *rowlength = rowlen;
    *ncolumns = ncols;
    *minncolumns = minncols;
    *column_type = coltype;
    *column_length = collen;

    G_free(coorbuf);

    return 0;
}


/* Import points from ascii file.
 *
 * fs: field separator
 * xcol, ycol, zcol, catcol: x,y,z,cat column in input file, first column is 1, 
 *                            zcol and catcol may be 0 (do not use)
 * rowlen: maximum row length
 * Note: column types (both in header or coldef) must be supported by driver
 */
int points_to_bin( FILE *ascii, int rowlen, struct Map_info *Map, dbDriver *driver,
		   char *table, char *fs, int ncols, int *coltype,
	           int xcol, int ycol, int zcol, int catcol, int skip_lines )
{
    char   *buf, buf2[1000];  
    int cat = 0;
    int row = 1;
    struct line_pnts *Points;
    struct line_cats *Cats;	
    dbString sql, val;
    struct Cell_head window;

    /* fetch projection for LatLong test */
    if (G__get_window (&window, "", "WIND", G_mapset()) != NULL){
        G_get_default_window (&window);
    }

    rewind(ascii);
    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();	

    buf = (char *) G_malloc ( rowlen + 1 );
    db_init_string (&sql);
    db_init_string (&val);

    if(skip_lines > 0) {
	sprintf(buf, "HEADER: (%d lines)\n", skip_lines);
	Vect_hist_write(Map, buf);
    }

    while ( G_getl2(buf, rowlen, ascii) != 0 ) {
	int i, len;
	double x, y, z;
	char **tokens;
	int ntokens;   /* number of tokens */

	if(row <= skip_lines) {
	    G_debug (4, "writing skip line %d to hist : %d chars", row, strlen(buf));
	    Vect_hist_write(Map, buf);
	    Vect_hist_write(Map, "\n");
	    row++;
	    continue;
	}

	len = strlen(buf);
	if ( len == 0 ) continue; /* should not happen */

	G_debug (4, "row: %s", buf );

	tokens = G_tokenize (buf, fs);
	ntokens = G_number_of_tokens ( tokens );

        if ((G_projection() == PROJECTION_LL)){
            G_scan_easting ( tokens[xcol], &x, window.proj);
            G_scan_northing ( tokens[ycol], &y, window.proj);
	}else{
           x = atof ( tokens[xcol] );
           y = atof ( tokens[ycol] );
	}
	G_debug (4, "x: %f, y: %f", x, y);

	if ( zcol >= 0 ) z = atof ( tokens[zcol] );
	else z = 0.0;

	if ( catcol >= 0 ) cat = atof ( tokens[catcol] );
	else cat++; 

	Vect_reset_line ( Points );
	Vect_reset_cats ( Cats ); 

	Vect_append_point ( Points, x, y, z);
	Vect_cat_set ( Cats, 1, cat );
	
	Vect_write_line ( Map, GV_POINT, Points, Cats );

	/* Attributes */
	if ( driver ) {
	    sprintf ( buf2, "insert into %s values ( ", table);
	    db_set_string (&sql, buf2 );

	    if ( catcol < 0 ) {
		sprintf ( buf2, "%d, ", cat );
		db_append_string (&sql, buf2 );
	    }

	    for ( i = 0; i < ntokens; i++ ) {
		if ( i > 0 ) db_append_string (&sql, ", " );

		if ( strlen(tokens[i]) > 0 ) {
		    if ( coltype[i] == DB_C_TYPE_INT || coltype[i] == DB_C_TYPE_DOUBLE ) {
			sprintf ( buf2, "%s", tokens[i] );
		    } else {
			db_set_string (&val, tokens[i]);
		    /* TODO: strip leading and trailing "quotes" from input string */
			db_double_quote_string ( &val );
			sprintf ( buf2, "'%s'", db_get_string ( &val ) );
		    }
		} else {
		    sprintf ( buf2, "null" );
		}
		db_append_string (&sql, buf2 );
	    }
	    db_append_string (&sql, ")" );
	    G_debug ( 3, db_get_string ( &sql ) );

	    if (db_execute_immediate (driver, &sql) != DB_OK ) {
		G_fatal_error ( "Cannot insert values: %s", db_get_string ( &sql )  );
	    }
	}

	G_free_tokens(tokens);
    }
    return 0;	
}
