/***************************************************************
 *
 * MODULE:       v.to.points
 * 
 * AUTHOR(S):    Radim Blazek
 *               
 * PURPOSE:      Create points along lines 
 *               
 * COPYRIGHT:    (C) 2002 by the GRASS Development Team
 *
 *               This program is free software under the 
 *               GNU General Public License (>=v2). 
 *               Read the file COPYING that comes with GRASS
 *               for details.
 *
 **************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gis.h"
#include "Vect.h"
#include "dbmi.h"

static int point_cat;
static struct line_cats *PCats; 
static struct line_pnts *PPoints;
static dbString stmt;
static dbDriver *driver;
static struct field_info *Fi;

void write_point ( struct Map_info *Out, double x, double y, double z, int line_cat, double along, 
	           int table )
{ 
    char buf[2000];
    G_debug ( 3, "write_point()" );
    
    Vect_reset_line ( PPoints );
    Vect_reset_cats ( PCats );
    
    /* Write point */
    Vect_append_point ( PPoints, x, y, z );
    Vect_cat_set (PCats, 1, point_cat);
    Vect_write_line ( Out, GV_POINT, PPoints, PCats );

    /* Attributes */
    if ( !table ) {
	db_zero_string ( &stmt );
	sprintf (buf, "insert into %s values ( %d, %d, %lf )", Fi->table, point_cat, line_cat, along );
	db_append_string ( &stmt, buf);

	if (db_execute_immediate (driver, &stmt) != DB_OK ) {
	    G_warning ( "Cannot inser new row: %s", db_get_string ( &stmt ) );
	}
    }
    point_cat++;
}

void write_line ( struct Map_info *Out, struct line_pnts *LPoints, int cat, 
	          int vertex, int interpolate, double dmax, int table )
{ 
    if ( vertex ) { /* use line vertices */
	double along;
	int vert;

	along = 0;
	for ( vert = 0; vert < LPoints->n_points; vert++ ) {
	    G_debug ( 3, "vert = %d", vert );

	    write_point ( Out, LPoints->x[vert], LPoints->y[vert], LPoints->z[vert], cat, along, 
			  table );

	    if ( vert < LPoints->n_points - 1) {  
		double dx, dy, dz, len;

		dx = LPoints->x[vert+1] - LPoints->x[vert];
		dy = LPoints->y[vert+1] - LPoints->y[vert];
		dz = LPoints->z[vert+1] - LPoints->z[vert];
		len = hypot ( hypot (dx, dy), dz );
	    
		/* interpolate segment */
		if ( interpolate && vert < ( LPoints->n_points - 1) ) {
		    int i, n;
		    double x, y, z, dlen;
		    
		    if ( len < dmax ) continue;

		    n = len / dmax + 1; /* number of segments */
		    dx /= n; dy /= n; dz /= n; dlen = len / n;
		    
		    for ( i = 1; i < n; i++ ) {
			x = LPoints->x[vert] + i * dx;
			y = LPoints->y[vert] + i * dy;
			z = LPoints->z[vert] + i * dz;

			write_point ( Out, x, y, z, cat, along + i * dlen, table);
		    }
		}
		along += len;
	    }
	}
    } else { /* do not use vertices */
	int i, n;
	double len, dlen, along, x, y, z;

	len = Vect_line_length ( LPoints );
	n = len / dmax + 1; /* number of segments */
	dlen = len / n; /* length of segment */
	
	G_debug ( 3, "n = %d len = %f dlen = %f", n, len, dlen );
	    
	for ( i = 0; i <= n; i++ ) { 
	    if ( i > 0 && i < n ) {
		along = i * dlen;
		Vect_point_on_line ( LPoints, along, &x, &y, &z, NULL, NULL );
	    } else { /* first and last vertex */
		if ( i == 0 ) {
		    along = 0;
		    x = LPoints->x[0];
		    y = LPoints->y[0];
		    z = LPoints->z[0];
	        } else { /* last */
		    along = len;
		    x = LPoints->x[LPoints->n_points-1];
		    y = LPoints->y[LPoints->n_points-1];
		    z = LPoints->z[LPoints->n_points-1];
		}
	    }
	    G_debug ( 3, "  i = %d along = %f", i, along );
	    write_point ( Out, x, y, z, cat, along, table );
	}
    }
}

int main(int argc, char **argv)
{
    int    field, type;
    double dmax;
    struct Option *in_opt, *out_opt, *type_opt, *dmax_opt, *lfield_opt;
    struct Flag *inter_flag, *vertex_flag, *table_flag;
    struct GModule *module;
    char   *mapset;
    struct Map_info In, Out;
    struct line_cats *LCats; 
    struct line_pnts *LPoints;
    char buf[2000];

    G_gisinit (argv[0]) ;

    module = G_define_module();
    module->description = "Create points along input lines.";

    in_opt = G_define_standard_option(G_OPT_V_INPUT);
    in_opt->description = "Input map containing lines";

    type_opt = G_define_standard_option(G_OPT_V_TYPE) ;
    type_opt->answer = "point,line,boundary,centroid";
    
    out_opt = G_define_standard_option(G_OPT_V_OUTPUT); 
    out_opt->description = "Output map where points will be written";

    lfield_opt = G_define_standard_option(G_OPT_V_FIELD);
    lfield_opt->key = "lfield";
    lfield_opt->answer = "1";
    lfield_opt->description = "Line field";

    vertex_flag = G_define_flag ();
    vertex_flag->key = 'v';
    vertex_flag->description = "Write line vertices.";

    inter_flag = G_define_flag ();
    inter_flag->key = 'i';
    inter_flag->description = "Interpolate points between line vertices.";

    dmax_opt = G_define_option ();
    dmax_opt->key = "dmax";
    dmax_opt->type = TYPE_DOUBLE;
    dmax_opt->required = NO;
    dmax_opt->answer = "100";
    dmax_opt->description = "Maximum distance between points.";
    
    table_flag = G_define_flag ();
    table_flag->key = 't';
    table_flag->description = "Do not create attribute table.";
    
    if(G_parser(argc,argv)) exit(1);

    LCats = Vect_new_cats_struct ();
    PCats = Vect_new_cats_struct ();
    LPoints = Vect_new_line_struct ();
    PPoints = Vect_new_line_struct ();
    db_init_string (&stmt);
    
    field = atoi (lfield_opt->answer);
    type = Vect_option_to_types ( type_opt );
    dmax = atof ( dmax_opt->answer );

    /* Open input lines */
    mapset = G_find_vector2 (in_opt->answer, NULL); 
    if(mapset == NULL) G_fatal_error ("Could not find input %s\n", in_opt->answer);
    Vect_set_open_level ( 2 );
    Vect_open_old (&In, in_opt->answer, mapset); 
    
    /* Open output segments */
    Vect_open_new ( &Out, out_opt->answer, Vect_is_3d (&In) );

    /* Table */
    if ( !table_flag->answer ) {
	Fi = Vect_default_field_info ( &Out, 1, NULL, GV_1TABLE );
	Vect_map_add_dblink ( &Out, 1, NULL, Fi->table, "cat", Fi->database, Fi->driver);

	/* Open driver */
	driver = db_start_driver_open_database ( Fi->driver, Fi->database );
	if ( driver == NULL )
	      G_fatal_error ( "Cannot open database %s by driver %s", Fi->database, Fi->driver );

	sprintf ( buf, "create table %s ( cat int, lcat int, along double precision )", Fi->table );
	db_append_string ( &stmt, buf);

	if (db_execute_immediate (driver, &stmt) != DB_OK ) {
	    db_close_database_shutdown_driver ( driver );
	    G_fatal_error ( "Cannot create table: %s", db_get_string ( &stmt ) );
	}
    }
    
    point_cat = 1;

    if ( type & (GV_POINTS | GV_LINES) ) {
    	int    line, nlines;

	nlines = Vect_get_num_lines (&In);
	for ( line = 1; line <= nlines; line++ ) {
	    int ltype, cat;
	    
	    G_debug ( 3, "line = %d", line );
	    ltype = Vect_read_line (&In, LPoints, LCats, line);
	    if ( !(ltype & type ) ) continue;

	    Vect_cat_get ( LCats, field, &cat );
	    
	    if (  LPoints->n_points <= 1 ) {
		write_point ( &Out, LPoints->x[0], LPoints->y[0], LPoints->z[0], cat, 0.0, table_flag->answer);
	    } else { /* lines */
                write_line ( &Out, LPoints, cat, vertex_flag->answer, inter_flag->answer, 
			     dmax, table_flag->answer );
	    }
	}
    }

    if ( type == GV_AREA ) {
	int area, nareas, centroid, cat;
	
	nareas = Vect_get_num_areas (&In);
	for ( area = 1; area <= nareas; area++ ) {
	    int i, isle, nisles;
	    
	    centroid = Vect_get_area_centroid ( &In, area );
	    cat = 0;
	    if ( centroid > 0 ) {
		Vect_read_line (&In, NULL, LCats, centroid );
		Vect_cat_get (LCats, field, &cat);
	    }
	    
	    Vect_get_area_points ( &In, area, LPoints );

	    write_line ( &Out, LPoints, cat, vertex_flag->answer, inter_flag->answer, 
			 dmax, table_flag->answer );

	    nisles = Vect_get_area_num_isles (&In, area);

	    for ( i = 0; i < nisles; i++ ) {
		isle = Vect_get_area_isle (&In, area, i);
		Vect_get_isle_points ( &In, isle, LPoints );

		write_line ( &Out, LPoints, cat, vertex_flag->answer, inter_flag->answer, 
			     dmax, table_flag->answer );
	    }
	}
    }	

    if ( !table_flag->answer )
        db_close_database_shutdown_driver ( driver );

    Vect_build (&Out, stderr);

    /* Free, close ... */
    Vect_close(&In);
    Vect_close(&Out);

    fprintf ( stdout, "%d points written to output map\n", point_cat - 1);

    exit(0);
}

