#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "gis.h"
#include "Vect.h"
#include "dbmi.h"

/* Result code */
#define SP_OK          0  /* Path found */
#define SP_UNREACHABLE 1  /* Node is not reachable */
#define SP_NOPOINT     2  /* Missing point of given category */

#define INPUT_MODE_NODE  1
#define INPUT_MODE_COOR  2

typedef struct { /* category index */
    int cat;     /* line category */
    int line;     /* line number */
} CIDX;

int cmp ( const void *, const void *);

int path ( struct Map_info *In, struct Map_info *Out, int nfield, double maxdist, int segments)
{
    int    i, nlines, line, npoints, type, cat, id, fcat, tcat, fline, tline, fnode, tnode, count;
    int    ret, sp, input_mode, unreachable, nopoint, formaterr;
    struct ilist *AList;
    double cost;
    struct line_pnts *Points, *OPoints, *FPoints, *TPoints;
    struct line_cats *Cats;
    CIDX   *Cidx, *Citem;
    char   buf[2000], dummy[2000];
    double fx, fy, tx, ty;

    /* Attribute table */
    dbString sql;
    dbDriver *driver;
    struct field_info *Fi;

    AList = Vect_new_list ();
    Points = Vect_new_line_struct ();
    OPoints = Vect_new_line_struct ();
    FPoints = Vect_new_line_struct ();
    TPoints = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    db_init_string (&sql);

    /* Create category index for input points */
    npoints = Vect_get_num_primitives ( In, GV_POINT );
    Cidx = (CIDX *) G_malloc ( npoints * sizeof (CIDX) );
 
    nlines = Vect_get_num_lines ( In );
    count = 0;
    for ( line = 1; line <= nlines; line++ ) {
	type = Vect_read_line (In, NULL, Cats, line);
	if ( type != GV_POINT ) continue;

        Vect_cat_get ( Cats, nfield, &cat );

        if ( cat < 0 ) continue;
	
	Cidx[count].cat = cat;
	Cidx[count].line = line;
        count++;
    }

    if ( count < npoints ) 
	G_warning ( "%d points without category (nfield: %d).", npoints - count, nfield );

    npoints = count;

    qsort( (void *)Cidx, npoints, sizeof(CIDX), cmp);

    /* Create table */
    Fi = Vect_default_field_info ( Out, 1, NULL, GV_1TABLE );
    Vect_map_add_dblink ( Out, 1, NULL, Fi->table, "cat", Fi->database, Fi->driver);
    
    driver = db_start_driver_open_database ( Fi->driver, Fi->database );
    if ( driver == NULL )
	G_fatal_error ( "Cannot open database %s by driver %s", Fi->database, Fi->driver );
    
    sprintf ( buf, "create table %s ( cat integer, id integer, fcat integer, tcat integer, " 
	      "sp integer, cost double precision, fdist double precision, tdist double precision )", 
	      Fi->table );

    db_set_string ( &sql, buf );
    G_debug ( 2, db_get_string ( &sql ) );
    
    if (db_execute_immediate (driver, &sql) != DB_OK ) {
        db_close_database_shutdown_driver ( driver );
	G_fatal_error ( "Cannot create table: %s", db_get_string ( &sql )  );
    }

    db_begin_transaction ( driver );

    /* Read stdin, find shortest path, and write connectin line and new database record */
    cat = 0;
    formaterr = nopoint = unreachable = 0;
    while ( fgets (buf, sizeof(buf), stdin) != NULL ) {
	double fdist, tdist;

	G_chop(buf);
	
	ret = sscanf ( buf, "%d %lf %lf %lf %lf %s", &id, &fx, &fy, &tx, &ty, dummy);
	if ( ret == 5 ) {
	    input_mode = INPUT_MODE_COOR;
	} else {
	    ret = sscanf ( buf, "%d %d %d %s", &id, &fcat, &tcat, dummy);
	    if ( ret != 3 ) {
		G_warning ( "Wrong input: %s", buf);
		formaterr++;
		continue;
	    }
	    input_mode = INPUT_MODE_NODE;
	}

	G_debug ( 3, "mode = %d", input_mode );
	
	cat++; /* Output category */
	sp = SP_OK;
	cost = fdist = tdist = 0;

	if ( input_mode == INPUT_MODE_NODE ) {
	    /* From */
	    Citem = (CIDX *) bsearch( (void *) &fcat, Cidx, npoints, sizeof(CIDX), cmp);
	    if ( Citem == NULL ) {
		G_warning ( "No point with category %d", fcat);
		sp = SP_NOPOINT; 
		fline = 0;
		fnode = 0;
		nopoint++;
	    } else {
		fline = Citem->line;
		Vect_get_line_nodes ( In, fline, &fnode, NULL);
	    }
	    G_debug ( 3, "from: cat = %5d point(line) = %5d node = %5d", fcat, fline, fnode );
	    
	    /* To */
	    Citem = (CIDX *) bsearch( (void *) &tcat, Cidx, npoints, sizeof(CIDX), cmp);
	    if ( Citem == NULL ) {
		G_warning ( "No point with category %d", tcat);
		sp = SP_NOPOINT;
		tline = 0;
		tnode = 0;
		nopoint++;
	    } else { 
		tline = Citem->line;
		Vect_get_line_nodes ( In, tline, &tnode, NULL);
	    }
	    G_debug ( 3, "to  : cat = %5d point(line) = %5d node = %5d", tcat, tline, tnode );
		    
	    if ( sp != SP_NOPOINT ) {
		ret = Vect_net_shortest_path ( In, fnode, tnode, AList, &cost);

		if ( ret == -1 ) {
		    sp = SP_UNREACHABLE;
		    unreachable++;
		    G_warning ( "Point with category %d is not reachable from point with category %d\n", 
				tcat, fcat );
		} else {
		    /* Write new line connecting 'from' and 'to' */
		    G_debug ( 3, "Number of arcs = %d, total costs = %f",  AList->n_values, cost);

		    Vect_reset_cats ( Cats );
		    Vect_cat_set ( Cats, 1, cat );

		    if ( segments ) {
			for ( i = 0; i < AList->n_values; i++ ) {
			    line = AList->value[i];
			    Vect_read_line ( In, Points, NULL, abs(line) );

			    if ( line > 0 ) {
			        Vect_write_line ( Out, GV_LINE, Points, Cats);
			    } else {
			        Vect_reset_line ( OPoints );
				Vect_append_points ( OPoints,  Points, GV_BACKWARD );
			        Vect_write_line ( Out, GV_LINE, OPoints, Cats);
			    }
			}	
		    } else { 
			Vect_reset_line ( OPoints );

			for ( i = 0; i < AList->n_values; i++ ) {
			    line = AList->value[i];
			    Vect_read_line ( In, Points, NULL, abs(line) );
			    if ( line > 0 ) 
				Vect_append_points ( OPoints,  Points, GV_FORWARD );
			    else 
				Vect_append_points ( OPoints,  Points, GV_BACKWARD );
			}	

			Vect_write_line ( Out, GV_LINE, OPoints, Cats);
		    }
		}
	    }
	} else { /* INPUT_MODE_COOR */
	    fcat = tcat = 0;
	    ret = Vect_net_shortest_path_coor ( In, fx, fy, 0.0, tx, ty, 0.0, maxdist, maxdist,
                                                &cost, OPoints, AList, FPoints, TPoints, &fdist, &tdist );

	    if ( ret == 0 ) {
		sp = SP_UNREACHABLE;
		unreachable++;
		G_warning ( "Point %f,%f is not reachable from point %f,%f\n", tx, ty, fx, fy );
	    } else {
		Vect_reset_cats ( Cats );
		Vect_cat_set ( Cats, 1, cat );

		if ( segments ) {
		    /* From point to the first node */
		    if ( FPoints->n_points > 0 ) 
			Vect_write_line ( Out, GV_LINE, FPoints, Cats);

		    /* On the network */
		    for ( i = 0; i < AList->n_values; i++ ) {
			line = AList->value[i];

			Vect_read_line ( In, Points, NULL, abs(line) );

			if ( line > 0 ) {
			    Vect_write_line ( Out, GV_LINE, Points, Cats);
			} else {
			    Vect_reset_line ( OPoints );
			    Vect_append_points ( OPoints,  Points, GV_BACKWARD );
			    Vect_write_line ( Out, GV_LINE, OPoints, Cats);
			}
		    }	

		    /* From last node to point */ 
		    if ( TPoints->n_points > 0 ) 
			Vect_write_line ( Out, GV_LINE, TPoints, Cats);
		    
		} else {
		    Vect_write_line ( Out, GV_LINE, OPoints, Cats);
		}
	    }
	}

	sprintf (buf, "insert into %s values ( %d, %d, %d, %d, %d, %f, %f, %f)", 
		       Fi->table, cat, id, fcat, tcat, sp, cost, fdist, tdist );
	db_set_string ( &sql, buf );
	G_debug ( 3, db_get_string ( &sql ) );
	
	if (db_execute_immediate (driver, &sql) != DB_OK ) {
	    db_close_database_shutdown_driver ( driver );
	    G_fatal_error ( "Insert new row: %s", db_get_string ( &sql )  );
	}
    }

    db_commit_transaction ( driver );
    
    db_close_database_shutdown_driver ( driver );
    
    Vect_destroy_list(AList);
    Vect_destroy_line_struct(Points);
    Vect_destroy_line_struct(OPoints);
    Vect_destroy_cats_struct(Cats);

    /* Print table info */
    fprintf ( stderr, "Attributes:\n");
    fprintf ( stderr, "  cat  - path unique category assigned by module\n");
    fprintf ( stderr, "  id   - path id (read from input)\n");
    fprintf ( stderr, "  fcat - from point category\n");
    fprintf ( stderr, "  tcat - to point category\n");
    fprintf ( stderr, "  sp - result status:\n");
    fprintf ( stderr, "       0 - OK, path found\n");
    fprintf ( stderr, "       1 - node is not reachable\n");
    fprintf ( stderr, "       2 - point of given category does not exist\n");
    fprintf ( stderr, "  cost - travelling costs (on the network, not to/from network)\n");
    fprintf ( stderr, "  fdist - the distance from first point to the network\n");
    fprintf ( stderr, "  tdist - the distance from the network to second point\n");

    if ( formaterr )
       G_warning ( "%d input format errors", formaterr );
    if ( nopoint )
       G_warning ( "%d points of given category missing", nopoint );
    if ( unreachable )
       G_warning ( "%d destinations unreachable (including points out of threshold)", unreachable );

    return 1;
}

int cmp ( const void *pa, const void *pb)
{
    CIDX *p1 = (CIDX *) pa;
    CIDX *p2 = (CIDX *) pb;

    if( p1->cat < p2->cat) return -1;
    if( p1->cat > p2->cat) return 1;
    return 0;
}

