/*
*  v.lrs.create - Generate segments or points from imput map for existing 
*                 linear reference system
*/

 /******************************************************************************
 * Copyright (c) 2004, Radim Blazek (blazek@itc.it)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>
#include "../lib/lrs.h"

int find_line ( struct Map_info *Map, int lfield, int cat );
    
int main(int argc, char **argv)
{
    int    i, ret, points_written, lines_written, points_read, lines_read;
    int    cat, *cats, ncat, lfield;
    int    line, ltype;
    int    id, lid, lcat1, lcat2;
    double mpost, offset, mpost2, offset2, map_offset1, map_offset2, multip, side_offset;
    double x, y, z, angle, len;
    char   stype;
    struct Option *in_opt, *out_opt, *driver_opt, *database_opt;
    struct Option *lfield_opt;
    struct Option *table_opt;
    struct GModule *module;
    char   *mapset, buf[2000];
    struct Map_info In, Out;
    struct line_cats *LCats, *SCats; 
    struct line_pnts *LPoints, *SPoints;
    struct field_info *Lfi;
    dbDriver *rsdriver;
    dbHandle rshandle;
    dbString rsstmt;	   
    dbCursor rscursor;

    G_gisinit (argv[0]) ;

    module = G_define_module();
    module->description = "Create points/segments from imput lines, "
	   "linear reference system and positions read from stdin in format:\n"
           "P <pid> <lid> <milepost>+<offset> [<side offset>]\n"
           "L <sid> <lid> <milepost>+<offset> <milepost>+<offset> [<side offset>]";

    in_opt = G_define_standard_option(G_OPT_V_INPUT);
    in_opt->description = "Input map containing lines";
    
    out_opt = G_define_standard_option(G_OPT_V_OUTPUT); 
    out_opt->description = "Output map where segments will be written";

    lfield_opt = G_define_standard_option(G_OPT_V_FIELD);
    lfield_opt->key = "llayer";
    lfield_opt->answer = "1";
    lfield_opt->description = "Line layer";
    
    driver_opt = G_define_option() ;
    driver_opt->key         = "rsdriver" ;
    driver_opt->type        = TYPE_STRING ;
    driver_opt->required    = YES; 
    driver_opt->description = "Driver name for reference system table";
    
    database_opt = G_define_option() ;
    database_opt->key         = "rsdatabase" ;
    database_opt->type        = TYPE_STRING ;
    database_opt->required    = YES; 
    database_opt->description = "Database name for reference system table";
    
    table_opt = G_define_option() ;
    table_opt->key         = "rstable" ;
    table_opt->type        = TYPE_STRING ;
    table_opt->required    = YES; 
    table_opt->description = "Name of the reference system table";
    
    if(G_parser(argc,argv)) exit(1);

    LCats = Vect_new_cats_struct ();
    SCats = Vect_new_cats_struct ();
    LPoints = Vect_new_line_struct ();
    SPoints = Vect_new_line_struct ();
    
    lfield = atoi (lfield_opt->answer);
    multip = 1000; /* Number of map units per MP unit */

    /* Open input lines */
    mapset = G_find_vector2 (in_opt->answer, NULL); 
    if(mapset == NULL) G_fatal_error ("Could not find input %s\n", in_opt->answer);
    Vect_set_open_level ( 2 );
    Vect_open_old (&In, in_opt->answer, mapset); 
    
    /* Open output segments */
    Vect_open_new ( &Out, out_opt->answer, Vect_is_3d (&In) );
    
    db_init_handle (&rshandle);
    db_init_string (&rsstmt);
    rsdriver = db_start_driver(driver_opt->answer);
    db_set_handle (&rshandle, database_opt->answer, NULL);
    if (db_open_database(rsdriver, &rshandle) != DB_OK)
        G_fatal_error("Cannot open database for reference table");

    points_read = 0; lines_read = 0;
    points_written = 0; lines_written = 0;

    while ( fgets (buf, sizeof(buf), stdin) != NULL ) {
	G_debug ( 2, "SEGMENT: %s", G_chop(buf));
	side_offset = 0;
	Vect_reset_line ( SPoints );
	Vect_reset_cats ( SCats );
	switch ( buf[0] ) {
	    case 'P':
		side_offset = 0;
		ret = sscanf ( buf, "%c %d %d %lf+%lf %lf", &stype, &id, &lid, &mpost, &offset, 
			                           &side_offset);
		if ( ret < 5 ) { 
		    G_warning ( "Cannot read input: %s", buf);
		    break;
		}
		points_read++;
		G_debug (2, "point: %d %d %f+%f %f", id, lid, mpost, offset, side_offset);
		
		
	        ret = LR_get_offset( rsdriver, table_opt->answer, "lcat", "lid", 
			"start_map", "end_map", "start_mp", "start_off", "end_mp", "end_off",
                        lid, mpost, offset, multip, &lcat1, &map_offset1 );
		if ( ret == 0 ) {
		    G_warning ( "No record in LR table for: %s", buf);
		    break;
		}
		if ( ret == 3 ) {
		    G_warning ( "More than one record in LR table for: %s", buf);
		    break;
		}
		/* OK, write point */
                line = find_line ( &In, lfield, lcat1 );
		if ( line == 0 ) {
		    G_warning ( "Cannot find line of cat %d", lcat1);
		    break;
		}

	        Vect_read_line ( &In, LPoints, LCats, line );
		ret = Vect_point_on_line ( LPoints, map_offset1, &x, &y, &z, &angle, NULL);
                if ( ret == 0 ) {
		    len = Vect_line_length ( LPoints );
		    G_warning ( "Cannot get point on line: cat = %d distance = %f (line length = %f)\n%s"
			        , lcat1, map_offset1, len, buf);
		    break;
		}

                Vect_append_point ( SPoints, x, y, z );
		Vect_cat_set ( SCats, 1, id );

		Vect_write_line ( &Out, GV_POINT, SPoints, SCats);
		points_written++;
		break;
	    case 'L':
		side_offset = 0;
		ret = sscanf ( buf, "%c %d %d %lf+%lf %lf+%lf %lf", &stype, &id, &lid, &mpost, &offset, 
			              &mpost2, &offset2, &side_offset);
		if ( ret < 7 ) { 
		    G_warning ( "Cannot read input: %s", buf);
		    break;
		}
		lines_read++;
		G_debug (2, "line: %d %d %f+%f %f+%f %f", id, lid, mpost, offset,
		                      mpost2, offset2, side_offset);
		/* Find both points */
                /* Nearest up */
	        ret = LR_get_nearest_offset( rsdriver, table_opt->answer, "lcat", "lid", 
			"start_map", "end_map", "start_mp", "start_off", "end_mp", "end_off",
                        lid, mpost, offset, multip, 0, &lcat1, &map_offset1 );
		if ( ret == 0 ) {
		    G_warning ( "No record in LRS table for 1. point of:\n  %s", buf);
		    break;
		}
		if ( ret == 3 ) {
		    G_warning ( "Using last from more offsets found for 1. point of:\n  %s", buf);
		}
		if ( ret == 2 ) {
		    G_warning ( "Requested offset for the 1. point not found, "
                                "using nearest found:\n  %s", buf);
		}
                /* Nearest down */
	        ret = LR_get_nearest_offset( rsdriver, table_opt->answer, "lcat", "lid", 
			"start_map", "end_map", "start_mp", "start_off", "end_mp", "end_off",
                        lid, mpost2, offset2, multip, 1, &lcat2, &map_offset2 );
		if ( ret == 0 ) {
		    G_warning ( "No record in LRS table for 2. point of:\n  %s", buf);
		    break;
		}
		if ( ret == 2 ) {
		    G_warning ( "Requested offset for the 2. point not found, "
                                "using nearest found:\n  %s", buf);
		}
		if ( ret == 3 ) {
		    G_warning ( "Using first from more offsets found for 2. point of:\n  %s", buf);
		}
		/* Check if both points are at the same line */
		if ( lcat1 != lcat2 ) {
		    G_warning ("Segment over 2 (or more) segments, not yet supported");
		    break;
		}
		G_debug (2, "segment: lcat = %d : %f -  %f", lcat1, map_offset1, map_offset2);
                line = find_line ( &In, lfield, lcat1 );
		if ( line == 0 ) {
		    G_warning ( "Cannot find line of cat %d", lcat1);
		    break;
		}

	        Vect_read_line ( &In, LPoints, LCats, line );
		
		len = Vect_line_length ( LPoints );
		if ( map_offset2 > len ) {
		    /* This is mostly caused by calculation only -> use a threshold for warning */
                    if ( fabs(map_offset2-len) > 1e-6 ) { /* usually around 1e-7 */
		        G_warning ( "End of segment > line length (%e) -> cut", 
                                    fabs(map_offset2-len)); 
                    }
		    map_offset2 = len;
		}
		    
		ret = Vect_line_segment ( LPoints, map_offset1, map_offset2, SPoints );
                if ( ret == 0 ) {
		    G_warning ( "Cannot make line segment: cat = %d : %f - %f (line length = %f)\n%s", 
			                      lcat1, map_offset1, map_offset2, len, buf);
		    break;
		}
		
		Vect_cat_set ( SCats, 1, id );

		Vect_write_line ( &Out, GV_LINE, SPoints, SCats);
	        G_debug ( 3, "  segment n_points = %d", SPoints->n_points);

		lines_written++;
	        G_debug ( 3, "  -> written.");
		break;
	    default:
		G_warning ("Incorrect segment type: %s", buf );
	}

    }

    db_close_database(rsdriver);
    Vect_build (&Out, stdout);

    /* Free, close ... */
    Vect_close(&In);
    Vect_close(&Out);

    fprintf ( stdout, "%d points read from input\n", points_read);
    fprintf ( stdout, "%d points written to output map (%d lost)\n", 
	                    points_written, points_read-points_written);
    fprintf ( stdout, "%d lines read from input\n", lines_read);
    fprintf ( stdout, "%d lines written to output map (%d lost)\n", 
	                   lines_written, lines_read-lines_written);

    exit(0);
}

/* Find line by cat, returns 0 if not found */
/* TODO: use category index */
int 
find_line ( struct Map_info *Map, int lfield, int lcat )
{
    int i, nlines, type, cat;
    struct line_cats *Cats;
    
    G_debug (2, "find_line(): lfield = %d lcat = %d", lfield, lcat);
    Cats = Vect_new_cats_struct ();
    
    nlines = Vect_get_num_lines ( Map );
    for ( i = 1; i <= nlines; i++ ) {
	type = Vect_read_line ( Map, NULL, Cats, i );
	if ( !(type & GV_LINE) ) continue;
	Vect_cat_get ( Cats, lfield, &cat );
	if ( cat == lcat ) return i;
    }

    return 0;
}
