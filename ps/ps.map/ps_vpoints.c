/* Functions: PS_vector_plot
**
** Modified by: Janne Soimasuo August 1994 line_cat added
** Author: Paul W. Carlson	March 1992
** modified to use G_plot_line() by Olga Waupotitsch on dec,93
*/

#include <grass/Vect.h>
#include <grass/dbmi.h>
#include <grass/symbol.h>
#include <grass/glocale.h>

#include "ps_info.h"
#include "clr.h"
#include "local_proto.h"
#include "vector.h"

int PS_vpoints_plot (struct Map_info *P_map, int vec, int type)
{
    struct line_pnts *Points, *nPoints, *pPoints;
    int k, line, cat, nlines, ret;
    struct line_cats *Cats; 

    char eps[50], epsfile[1024], sname[100];
    double nn, ee;
    double size, x, y, xt, yt;
    double llx, lly, urx, ury;
    int x_int, y_int, eps_exist;
    SYMBOL *Symb;
    VARRAY *Varray = NULL;

    /* Attributes if sizecol is used */
    int i, nrec, ctype;
    struct field_info *Fi;
    dbDriver *Driver;
    dbCatValArray cvarr;
    int size_val_int;
    double size_val;


    /* Create vector array if required */
    if ( vector.layer[vec].cats != NULL || vector.layer[vec].where != NULL ) {
	Varray = Vect_new_varray ( Vect_get_num_lines(P_map) );
        if ( vector.layer[vec].cats != NULL ) {
	    ret = Vect_set_varray_from_cat_string (P_map, vector.layer[vec].field,
	                 vector.layer[vec].cats, vector.layer[vec].ltype, 1, Varray );
	} else {
	    ret = Vect_set_varray_from_db (P_map, vector.layer[vec].field,
	                 vector.layer[vec].where, vector.layer[vec].ltype, 1, Varray );
	}
	G_debug ( 3, "%d items selected for vector %d", ret, vec );
	if(ret == -1)
	     G_fatal_error(_("Cannot load data from table"));
    }

    /* allocate memory for coordinates */
    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct ();

    /* process only vectors in current window */
    Vect_set_constraint_region(P_map, PS.w.north, PS.w.south, PS.w.east, PS.w.west,
	                       PORT_DOUBLE_MAX, -PORT_DOUBLE_MAX);

    /* Read symbol */
    if ( vector.layer[vec].symbol != NULL ) { 
        sprintf( sname, "SITESYMBOL%d", vec);
        Symb = S_read ( vector.layer[vec].symbol );
	if ( Symb == NULL ) { 
	    G_warning (_("Cannot read symbol, using default icon"));
	}
	symbol_save ( Symb, &(vector.layer[vec].color),
			 &(vector.layer[vec].fcolor), sname );
	vector.layer[vec].symbol_ps = G_store ( sname );
    }


    /* if eps file is specified as common for all points then
       read bbox and save eps to PS file */
    if ( vector.layer[vec].epstype == 1)
    {
	if ( !eps_bbox( vector.layer[vec].epspre, &llx, &lly, &urx, &ury) ) {
	    vector.layer[vec].epstype = 0; /* eps file can't be read */
	} else {  /* save to PS */
	    sprintf (eps, "SITEEPSF%d", vec);
	    eps_save ( PS.fp, vector.layer[vec].epspre, eps);
	}
    }


    /* Load attributes if sizecol used */
    if(vector.layer[vec].sizecol != NULL) {
	db_CatValArray_init ( &cvarr );

	Fi = Vect_get_field( P_map, vector.layer[vec].field );
	if ( Fi == NULL ) {
	    G_fatal_error(_("Cannot get layer info for vector map"));
	}

	Driver = db_start_driver_open_database(Fi->driver, Fi->database);
	if (Driver == NULL)
	    G_fatal_error(_("Cannot open database %s by driver %s"), Fi->database, Fi->driver);

	/* Note do not check if the column exists in the table because it may be expression */

   /* TODO: only select values we need instead of all in column */
	nrec = db_select_CatValArray(Driver, Fi->table, Fi->key, 
			vector.layer[vec].sizecol, NULL, &cvarr );
	G_debug (3, "nrec = %d", nrec );

	ctype = cvarr.ctype;
	if ( ctype != DB_C_TYPE_INT && ctype != DB_C_TYPE_DOUBLE )
	    G_fatal_error (_("Column type not supported"));

	if ( nrec < 0 ) G_fatal_error (_("Cannot select data from table"));
	G_debug(2, "\n%d records selected from table", nrec);

	db_close_database_shutdown_driver(Driver);

	for ( i = 0; i < cvarr.n_values; i++ ) {
	    if ( ctype == DB_C_TYPE_INT ) {
		G_debug (4, "cat = %d val = %d", cvarr.value[i].cat, cvarr.value[i].val.i );
	    } else if ( ctype == DB_C_TYPE_DOUBLE ) {
		G_debug (4, "cat = %d val = %f", cvarr.value[i].cat, cvarr.value[i].val.d );
	    }
	}
    }

    /* read and plot vectors */
    k = 0;
    nlines = Vect_get_num_lines ( P_map );
    for ( line = 1; line <= nlines; line++ ) {
	if (0 > (ret = Vect_read_line(P_map, Points, Cats, line))) {
	    if (ret == -1) G_warning(_("Read error in vector file"));
	    break;
	}
	if ( !(ret & GV_POINTS) ) continue;
	if ( !(ret & vector.layer[vec].ltype) ) continue;

	if ( Varray != NULL && Varray->c[line] == 0 ) continue; /* is not in array */

	pPoints = Points; nPoints=0;
	Vect_cat_get( Cats, 1, &cat);

	nn = Points->y[0];
	ee = Points->x[0];

	if (nn > PS.w.north || nn < PS.w.south) continue;
	if (ee > PS.w.east  || ee < PS.w.west ) continue;

	G_plot_where_xy(ee, nn, &x_int, &y_int);
	x = (double) x_int / 10.;
	y = (double) y_int / 10.;

	if( vector.layer[vec].sizecol == NULL)
	    size = vector.layer[vec].size;
	else {  /* get value from sizecol column */

	    if( ctype == DB_C_TYPE_INT ) {
		ret = db_CatValArray_get_value_int(&cvarr, cat, &size_val_int);
		if ( ret != DB_OK ) {
		    G_warning(_("No record for cat = %d"), cat );
		    continue;
		}
		size_val = (double)size_val_int;
	    }

	    if( ctype == DB_C_TYPE_DOUBLE ) {
		ret = db_CatValArray_get_value_double(&cvarr, cat, &size_val);
		if ( ret != DB_OK ) {
		    G_warning(_("No record for cat = %d"), cat );
		    continue;
		}
	    }

	    if (size_val < 0.0) {
		G_warning(_("Attribute is of invalid size (%.3f) for category %d."), size_val, cat);
		continue;
	    }

	    if (size_val == 0.0) continue;

	    size = size_val * vector.layer[vec].scale;
	    G_debug(3, "    dynamic symbol size = %.2f", size);
	}

	if (vector.layer[vec].epstype == 1)  /* draw common eps */ 
	{
	    /* calculate translation */
	    eps_trans (llx, lly, urx, ury, x, y, size, vector.layer[vec].rotate, &xt, &yt);
	    eps_draw_saved ( PS.fp, eps, xt, yt, size, vector.layer[vec].rotate);
	}
	else if ( vector.layer[vec].epstype == 2)  /* draw epses */ 
	{
	    sprintf (epsfile, "%s%d%s", vector.layer[vec].epspre, cat, vector.layer[vec].epssuf);
	    if ( (eps_exist = eps_bbox( epsfile, &llx, &lly, &urx, &ury)) )
	    {
		eps_trans (llx, lly, urx, ury, x, y, size, vector.layer[vec].rotate, &xt, &yt);

		eps_draw ( PS.fp, epsfile, xt, yt, size, vector.layer[vec].rotate); 
	    }
	}

	 /* draw the icon */	    
	if ( ( vector.layer[vec].epstype == 0) || (vector.layer[vec].epstype == 2 
		    && !eps_exist ) )   
	{
	    if ( Symb != NULL ) { 
		symbol_draw ( sname, x, y, size, vector.layer[vec].rotate, 
		    vector.layer[vec].width);
	    }
	}
    }
    fprintf(PS.fp, "\n");
    return 0;
}
