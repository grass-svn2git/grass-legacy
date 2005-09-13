/**************************************************************
 * v.to.rast3 - read GRASS vector and writes 3D raster
 *
 * 9/2005 Upgrade to GRASS 6 by Radim Blazek
 * 
 **************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gis.h"
#include "glocale.h"
#include "G3d.h"
#include "Vect.h"
#include "dbmi.h"

int main(int argc, char *argv[])
{
    struct GModule *module;
    struct Option *in_opt, *out_opt, *field_opt, *col_opt;
    int field;
    G3D_Region region;

    struct Map_info Map;
    struct line_pnts *Points;
    struct line_cats *Cats;
    void *map = NULL;

    int nlines, line, nrec, ctype;

    struct field_info *Fi;
    dbDriver *Driver;
    dbCatValArray cvarr;

    module = G_define_module();
    module->description = _("Converts a binary GRASS vector map "
	"(only points) layer into a 3D GRASS raster map layer.");

    in_opt = G_define_standard_option(G_OPT_V_INPUT);
    field_opt = G_define_standard_option(G_OPT_V_FIELD);

    col_opt = G_define_option();
    col_opt->key            = "column";
    col_opt->type           = TYPE_STRING;
    col_opt->required       = YES;
    col_opt->multiple       = NO;
    col_opt->description    = _("Column name");

    out_opt = G_define_option() ;
    out_opt->key        = "output" ;
    out_opt->type       = TYPE_STRING ;
    out_opt->required   = YES;
    out_opt->description= _("Name of output G3D grid file") ;
    out_opt->gisprompt  = "new,grid3,3d raster" ;

    G_gisinit(argv[0]);

    if (G_parser(argc, argv))
        exit(1);

    if (G_legal_filename(out_opt->answer) < 0)
    {
	G_fatal_error ( "Illegal output name: '%s'", out_opt->answer);
    }

    G3d_getWindow (&region);
    G3d_readWindow(&region,NULL);

    field = atoi (field_opt->answer);

    Vect_set_open_level (2);
    Vect_open_old ( &Map, in_opt->answer, "");

    db_CatValArray_init ( &cvarr );
    Fi = Vect_get_field( &Map, field);
    if ( Fi == NULL ) {
	G_fatal_error ("Cannot get layer info for vector map");
    }

    Driver = db_start_driver_open_database ( Fi->driver, Fi->database );
    if (Driver == NULL)
	G_fatal_error("Cannot open database %s by driver %s", Fi->database, Fi->driver);

    /* Note: do not check if the column exists in the table because it may be expression */

    nrec = db_select_CatValArray ( Driver, Fi->table, Fi->key, 
				col_opt->answer, NULL, &cvarr );

    G_debug (2, "nrec = %d", nrec );
    if ( nrec < 0 ) G_fatal_error ("Cannot select data from table");

    ctype = cvarr.ctype;
    if ( ctype != DB_C_TYPE_INT && ctype != DB_C_TYPE_DOUBLE )
        G_fatal_error ( "Column type not supported" );

    db_close_database_shutdown_driver(Driver);

    map = G3d_openCellNew ( out_opt->answer, G3D_FLOAT,  
		G3D_USE_CACHE_DEFAULT, &region);

    if (map == NULL)
    {
	G_fatal_error ("Can't create output map");
    }

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct ();

    nlines = Vect_get_num_lines (&Map);
    for ( line = 1; line <= nlines; line++) {
        int type, cat, depth, row, col, ret;
        double value;

        type = Vect_read_line ( &Map, Points, Cats, line ); 
        if ( !(type & GV_POINT) ) continue;

        Vect_cat_get (Cats, field, &cat);
        if ( cat < 0 ) { continue; }

        if ( Points->x[0] < region.west || Points->x[0] > region.east
            || Points->y[0] < region.south || Points->y[0] > region.north
            || Points->z[0] < region.bottom || Points->z[0] > region.top 
        ) 
	{
	    continue;
        }

        row = (int) floor ( (Points->y[0] - region.south 
                             - region.ns_res/2.) / region.ns_res);
        col = (int) floor ( (Points->x[0] - region.west
                             - region.ew_res/2.) / region.ew_res);
	depth = (int) floor ( (Points->z[0] - region.bottom
                              - region.tb_res/2.) / region.tb_res ) ;

	if ( ctype == DB_C_TYPE_INT ) {
            int ivalue;
	    ret = db_CatValArray_get_value_int ( &cvarr, cat, &ivalue );
	    value = ivalue;
	} else if ( ctype == DB_C_TYPE_DOUBLE ) {
	    ret = db_CatValArray_get_value_double ( &cvarr, cat, &value );
	}

	if ( ret != DB_OK ) {
	    G_warning ("No record for line (cat = %d)", cat );
	    continue ;
	}

        G_debug ( 3, "col,row,depth,val: %d %d %d %f", col, row, depth, value);

	G3d_putFloat (map, col, row, depth, (float)value);
    }

    Vect_close ( &Map );

    if (! G3d_closeCell (map) )
    {
        G_fatal_error ( "Could not close new g3d map" );
    }

    exit(0);
}

