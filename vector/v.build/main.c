/* ***************************************************************
 * *
 * * MODULE:       v.build
 * * 
 * * AUTHOR(S):    Radim Blazek
 * *               
 * * PURPOSE:      Build topology
 * *               
 * * COPYRIGHT:    (C) 2001 by the GRASS Development Team
 * *
 * *               This program is free software under the 
 * *               GNU General Public License (>=v2). 
 * *               Read the file COPYING that comes with GRASS
 * *               for details.
 * *
 * **************************************************************/
#include <stdlib.h>
#include "gis.h"
#include "Vect.h"

int 
main (int argc, char *argv[])
{
    struct GModule *module;
    struct Option *map_opt, *opt, *err_opt;
    struct Map_info Map;
    int    i, build = 0, dump = 0, sdump = 0, cdump = 0;
    
    map_opt = G_define_standard_option(G_OPT_V_MAP);
    
    err_opt = G_define_standard_option(G_OPT_V_OUTPUT);
    err_opt->key = "error";
    err_opt->description  = "Name of vector where errors are written";
    err_opt->required = NO;
    
    opt = G_define_option();
    opt->key = "option";
    opt->type =  TYPE_STRING;
    opt->options = "build,dump,sdump,cdump";
    opt->required = NO;
    opt->multiple = YES;
    opt->answer = "build";
    opt->description  = "Build topology or dump topology or spatial index to stdout\n"
	"\t\tbuild - build topology\n"
	"\t\tdupm  - write topology to stdout\n"
	"\t\tsdump - write spatial index to stdout\n"
	"\t\tcdump - write category index to stdout\n";
    
    G_gisinit(argv[0]);
    if (G_parser (argc, argv))
	exit(-1); 
   
    module = G_define_module(); 
    module->description = "Creates topology for GRASS vector data.";
  
    i = 0; 
    while (opt->answers[i]) {
	if ( *opt->answers[i] == 'b')  build = 1;
        else if ( *opt->answers[i] == 'd')  dump = 1;
        else if ( *opt->answers[i] == 's')  sdump = 1;
        else if ( *opt->answers[i] == 'c')  cdump = 1;

	i++;
    }
    if ( err_opt->answer ) {    
        Vect_check_input_output_name ( map_opt->answer, err_opt->answer, GV_FATAL_EXIT );
    }
    
    /* build topology */
    if ( build ) { 
	/* open input vector */
	if ( G_find_vector2 (map_opt->answer, G_mapset()) == NULL)
	     G_fatal_error ("Could not find input map <%s>\n", map_opt->answer);
	
	Vect_set_open_level (1); 
	Vect_open_old (&Map, map_opt->answer, G_mapset()); 

	Vect_build ( &Map, stdout );
    }
    /* dump topology */
    if (dump || sdump || cdump) {
        if ( !build ) { 
	    Vect_set_open_level (2);
	    Vect_open_old (&Map, map_opt->answer, G_mapset()); 
        }
        if (dump)
	    Vect_topo_dump ( &(Map.plus), stdout );

        if (sdump)
	    Vect_spatial_index_dump ( &(Map.plus), stdout );

        if (cdump)
	    Vect_cidx_dump ( &Map, stdout );
    }

    if ( err_opt->answer ) {
	int    nlines, line, type, area, left, right, err;
        struct Map_info  Err;
	struct line_pnts *Points;
	struct line_cats *Cats;
	
	Points = Vect_new_line_struct ();
        Cats = Vect_new_cats_struct ();

        Vect_open_new (&Err, err_opt->answer, Vect_is_3d(&Map) );

	nlines = Vect_get_num_lines (&Map);

	for ( line = 1; line <= nlines; line++ ){
	    err = 0;
	    type = Vect_read_line (&Map, Points, Cats, line);

	    
	    if ( type == GV_BOUNDARY ) {
		Vect_get_line_areas ( &Map, line, &left, &right );
		if ( left == 0 || right == 0 )
		    err = 1;
	    } else if ( type == GV_CENTROID ) {
		area = Vect_get_centroid_area ( &Map, line );
		if ( area <= 0 )
		    err = 1;
	    }
	    
	    if (err)
		Vect_write_line ( &Err, type, Points, Cats );
	}

	Vect_build ( &Err, stdout );
        Vect_close ( &Err );
    }

    Vect_close (&Map);
    
    exit(0) ;
}


