/****************************************************************
 *								*
 * MODULE:	v.lidar.growing				*
 * 								*
 * AUTHOR(S):  	Roberto Antolin & Gonzalo Moreno		*
 *               						*
 * PURPOSE:	Building contour determination and Region	* 
 *		Growing algorithm for determining the building	*
 *		inside						*
 *               						*
 * COPYRIGHT:	(C) 2006 by Politecnico di Milano - 		*
 *			     Polo Regionale di Como		*
 *								*
 *               This program is free software under the 	*
 *               GNU General Public License (>=v2). 		*
 *               Read the file COPYING that comes with GRASS	*
 *               for details.					*
 *								*
 ****************************************************************/

/*INCLUDES*/
#include <stdlib.h> 
#include <string.h> 
#include <grass/config.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>
#include <grass/glocale.h>

#include "growing.h"

/* GLOBAL DEFINITIONS*/
int nsply, nsplx, count_obj;
double passoN, passoE;

/*--------------------------------------------------------------------------------*/
int
main (int argc,char *argv[])
{

 /* Variables' declarations */
    int row, nrows, col, ncols, MaxPoints;
    int last_row, last_column;
    int nlines, nlines_first, line_num;
    int more;
    int clas, region=TRUE;
    double Z_interp;
    double Thres_j, Thres_d, ew_resol, ns_resol;
    double minNS, minEW, maxNS, maxEW;
    char *dvr, *db, *mapset, buf[1024];

    int colorBordo, ripieno, conta, lungPunti, lungHull, xi, c1, c2;
    double altPiano;
    extern double **P, **cvxHull, **punti_bordo;

/* Struct declarations */
    struct Cell_head elaboration_reg, original_reg;
    struct element_grow **raster_matrix;

    struct Map_info In, Out, First;
    struct Option *in_opt, *out_opt, *first_opt, *dbdriver, *dbdatabase, *Thres_j_opt, *Thres_d_opt; 
    struct GModule *module;

    struct line_pnts *points, *points_first;
    struct line_cats *Cats, *Cats_first;

    dbDriver *driver;
    dbString sql;
    dbTable *table;
    dbCursor cursor;
	

/*------------------------------------------------------------------------------------------*/
/* Options' declaration */;
    module = G_define_module();
    module->keywords = _("vector, LIDAR");
    module->description = _("Building contour determination and Region Growing " 
    				"algorithm for determining the building inside");	

    in_opt = G_define_standard_option(G_OPT_V_INPUT);

    out_opt = G_define_standard_option(G_OPT_V_OUTPUT);

    first_opt = G_define_option();
    	first_opt->key = "input_first";
    	first_opt->type = TYPE_STRING;
    	first_opt->key_desc     = "name";
	first_opt->required     = YES;
	first_opt->gisprompt    = "old,vector,vector";
	first_opt->description = _("Name of the first pulse vector map");

    dbdatabase = G_define_option() ;
    	dbdatabase->key        	= "database" ;
    	dbdatabase->type       	= TYPE_STRING ;
    	dbdatabase->required   	= NO ;
    	dbdatabase->multiple   	= NO ;
    	dbdatabase->description	= _("Database name");
    if ( (db=G__getenv2("DB_DATABASE",G_VAR_MAPSET)) )
	    dbdatabase->answer = G_store ( db );

    dbdriver = G_define_option() ;
   	dbdriver->key         = "driver" ;
    	dbdriver->type        = TYPE_STRING ;
    	dbdriver->options     = db_list_drivers();
    	dbdriver->required    = NO ;
    	dbdriver->multiple    = NO ;
    	dbdriver->description = _("Driver name");
    if ( (dvr = G__getenv2("DB_DRIVER",G_VAR_MAPSET)) )
	    dbdriver->answer = G_store ( dvr );

    Thres_j_opt = G_define_option();
    	Thres_j_opt->key = "tj";
    	Thres_j_opt->type = TYPE_DOUBLE;
    	Thres_j_opt->required = NO;
	Thres_j_opt->description = _("Threshold for cell object frequency in region growing");
    	Thres_j_opt->answer = "0.2";

    Thres_d_opt = G_define_option();
    	Thres_d_opt->key = "td";
    	Thres_d_opt->type = TYPE_DOUBLE;
    	Thres_d_opt->required = NO;
	Thres_d_opt->description = _("Threshold for double pulse in region growing");
    	Thres_d_opt->answer = "0.6";

/* Parsing */	
    G_gisinit(argv[0]);
    if (G_parser (argc, argv))
	exit(EXIT_FAILURE); 

    Thres_j = atof (Thres_j_opt->answer);
    Thres_d = atof (Thres_d_opt->answer);

    Thres_j += 1;

/* Open input vector */
    Vect_check_input_output_name ( in_opt->answer, out_opt->answer, GV_FATAL_EXIT );
    if ((mapset = G_find_vector2 (in_opt->answer, "")) == NULL) {
	    G_fatal_error ( _("Cannot find input map <%s>"), in_opt->answer);
    }

    /*Vect_set_open_level (2);		WITH TOPOLOGY*/
    Vect_set_open_level (1); 		/* WITHOUT TOPOLOGY*/
    if (Vect_open_old (&In, in_opt->answer, mapset) < 1)
        G_fatal_error ( _("Cannot open vector points map <%s>"), in_opt->answer);
	
    if (Vect_open_old (&First, first_opt->answer, mapset) < 1)
	G_fatal_error ( _("Cannot open vector points map <%s>"), first_opt->answer);

/* Open output vector */
    	if (0 > Vect_open_new (&Out, out_opt->answer, WITH_Z)) {
	    Vect_close (&In);
	    Vect_close (&First);
	    exit (EXIT_FAILURE);
    	}

/* Copy vector Head File */
	Vect_copy_head_data (&In, &Out);
	Vect_hist_copy (&In, &Out);
	Vect_hist_command (&Out);

/* Starting driver and open db */
    driver = db_start_driver_open_database (dbdriver->answer, dbdatabase->answer);
    if (driver == NULL)
	    G_fatal_error( _("No db connection for driver <%s> defined. Run db.connect"), dbdriver->answer);

/* Setting regions and boxes */
    G_debug (1, _("Setting regions and boxes"));
    G_get_set_window (&original_reg);
    G_get_set_window (&elaboration_reg);

/*  Fixxing parameters of the elaboration region */
/*! The original_region will be divided into several subregions */
    ew_resol = original_reg.ew_res;
    ns_resol = original_reg.ns_res;

/* Subdividing and working with tiles */
    elaboration_reg.south = original_reg.north;
    last_row = FALSE;

    db_init_string (&sql);
    db_zero_string (&sql);

    sprintf (buf, "SELECT Interp,ID FROM %s_edge_Interpolation", in_opt->answer);	
    db_append_string (&sql, buf);

    if (db_open_select_cursor (driver, &sql, &cursor, DB_SEQUENTIAL) != DB_OK)
	    G_fatal_error (_("It was impossible open table"));

    count_obj = 1;

    nlines = Vect_get_num_lines (&In);
    points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();

    nlines_first = Vect_get_num_lines (&First);
    points_first = Vect_new_line_struct ();
    Cats_first = Vect_new_cats_struct ();

    while (last_row == FALSE){		/* For each strip of LATO rows*/
	
	elaboration_reg.north=elaboration_reg.south;
	
	if (elaboration_reg.north > original_reg.north) 		/* First row*/
	    elaboration_reg.north=original_reg.north;
	
	elaboration_reg.south = elaboration_reg.north - LATO * ns_resol;
	if (elaboration_reg.south <= original_reg.south) {		/* Last row*/
	    elaboration_reg.south = original_reg.south;
	    last_row = TRUE;
	}

	elaboration_reg.east = original_reg.west;
	last_column = FALSE;
	
	while (last_column == FALSE){	/* For each strip of LATO columns*/
	    BOUND_BOX elaboration_box;

	    elaboration_reg.west=elaboration_reg.east;
	    if (elaboration_reg.west < original_reg.west)  		/* First column*/
		elaboration_reg.west = original_reg.west;

	    elaboration_reg.east = elaboration_reg.west + LATO * ew_resol;
	
	    if (elaboration_reg.east >= original_reg.east) {		/* Last column*/
		elaboration_reg.east = original_reg.east;
		last_column = TRUE;
	    }

	/*Setting the active region*/
	    G_set_window (&elaboration_reg);
	    nrows = G_window_rows ();
	    ncols = G_window_cols ();

	    G_debug (1,_("Rows = %d"), nrows);
	    G_debug (1,_("Columns = %d"), ncols);

	    raster_matrix = structMatrix (0, nrows, 0, ncols);
	    MaxPoints = nrows * ncols;

	/* Initializing matrix */
	    for (row = 0; row <= nrows; row++) {
		for (col = 0; col <= ncols; col++) {
		    raster_matrix[row][col].interp = 0;
		    raster_matrix[row][col].fi = 0;
		    raster_matrix[row][col].bordo = 0;
		    raster_matrix[row][col].dueImp = SINGLE_PULSE;
		    raster_matrix[row][col].orig = 0;
		    raster_matrix[row][col].fo = 0;
		    raster_matrix[row][col].clas = PRE_TERRAIN;
		    raster_matrix[row][col].fc = 0;
		    raster_matrix[row][col].obj = 0;
		}
	    }
	    
	    line_num = 0;
	    Vect_rewind (&In);
	    while (Vect_read_next_line (&In, points, Cats) > 0) {
		line_num++;

		Vect_region_box (&elaboration_reg, &elaboration_box);
		
		if ((Vect_point_in_box (points->x[0], points->y[0], points->z[0], &elaboration_box)) && \
				((points->x[0] != elaboration_reg.west) || (points->x[0] == original_reg.west)) && \
				((points->y[0] != elaboration_reg.north) || (points->y[0] == original_reg.north))){	

		    row = (int)(G_northing_to_row(points->y[0], &elaboration_reg));
		    col = (int)(G_easting_to_col(points->x[0], &elaboration_reg));

		    while (1) {
			if(db_fetch (&cursor, DB_NEXT, &more) != DB_OK || !more) break;
			dbColumn *Z_Interp_col, *ID_col;
			dbValue *Z_Interp_value, *ID_value;

			table = db_get_cursor_table (&cursor);

			ID_col = db_get_table_column (table,1);
			if ( db_sqltype_to_Ctype (db_get_column_sqltype (ID_col)) == DB_C_TYPE_INT) 
			    ID_value = db_get_column_value (ID_col);
			else continue;
			
			if (db_get_value_int (ID_value) == line_num) {
			    Z_Interp_col = db_get_table_column (table,0);
			    if ( db_sqltype_to_Ctype (db_get_column_sqltype (Z_Interp_col)) == DB_C_TYPE_DOUBLE ) 
				Z_Interp_value = db_get_column_value (Z_Interp_col);
			    else continue;
			    Z_interp = db_get_value_double (Z_Interp_value);
			    break;
			}
	    	    }

		    raster_matrix[row][col].interp += Z_interp;
		    raster_matrix[row][col].fi ++;

		    /*if (( clas = Vect_get_line_cat (&In, line_num, F_EDGE_DETECTION_CLASS) ) != UNKNOWN_EDGE) {*/
		    if (Vect_cat_get (Cats, F_EDGE_DETECTION_CLASS, &clas)) {
			raster_matrix[row][col].clas += clas;
			raster_matrix[row][col].fc ++;
		    }

		    raster_matrix[row][col].orig += points->z[0];
		    raster_matrix[row][col].fo ++;
		}
		
		Vect_reset_cats (Cats);
		Vect_reset_line (points);
	    }

	    for (row = 0; row <= nrows; row++) {
		for (col = 0; col <= ncols; col++) {

		    if (raster_matrix[row][col].fc != 0) {
			raster_matrix[row][col].clas --;
			raster_matrix[row][col].clas /= raster_matrix[row][col].fc;
		    }

		    if (raster_matrix[row][col].fi != 0)
			raster_matrix[row][col].interp /= raster_matrix[row][col].fi;

		    if (raster_matrix[row][col].fo != 0)
			raster_matrix[row][col].orig /= raster_matrix[row][col].fo;
		}
	    }

	    /* DOUBLE IMPULSE */
	    Vect_rewind (&First);
	    while (Vect_read_next_line (&First, points_first, Cats_first) > 0) {

		if ((Vect_point_in_box (points_first->x[0], points_first->y[0], points_first->z[0], &elaboration_box)) && \
				((points->x[0] != elaboration_reg.west) || (points->x[0] == original_reg.west)) && ((points->y[0] \
				!= elaboration_reg.north) || (points->y[0] == original_reg.north))) {

			row = (int)(G_northing_to_row(points_first->y[0], &elaboration_reg));
			col = (int)(G_easting_to_col(points_first->x[0], &elaboration_reg));

			if (fabs(points_first->z[0] - raster_matrix[row][col].orig) >= Thres_d) 
			    raster_matrix[row][col].dueImp = DOUBLE_PULSE;
		}
		Vect_reset_cats (Cats_first);
		Vect_reset_line (points_first);
	    }

	/* REGION GROWING */
	    if (region == TRUE) {
		G_debug (1, _("Region Growing"));

		punti_bordo = G_alloc_matrix (MaxPoints, 3);
		P = Pvector(0, MaxPoints);

		colorBordo = 5;
		ripieno = 6;

		for (row = 0; row <= nrows; row++) {
		    for (col = 0; col <= ncols; col++) {

			if ((raster_matrix[row][col].clas >= Thres_j) && (raster_matrix[row][col].clas < colorBordo) \
					&& (raster_matrix[row][col].fi != 0) &&	(raster_matrix[row][col].dueImp == SINGLE_PULSE)) {
			
			/* Selecting a connected Object zone*/
			    ripieno++;
			    if (ripieno > 10) ripieno = 6;

			/* Selecting points on a connected edge*/
			    for (conta = 0; conta < MaxPoints; conta++) {
				 punti_bordo[conta][0] = 0;
				 punti_bordo[conta][1] = 0;
				 punti_bordo[conta][2] = 0;
				 P[conta] = punti_bordo[conta];
			    }

			    lungPunti = 0;
			    lungHull = 0;

			    regGrow8 (elaboration_reg, raster_matrix, punti_bordo, &lungPunti, row, col, colorBordo, Thres_j, MaxPoints);

			    /*CONVEX-HULL COMPUTATION */
			    lungHull = ch2d(P, lungPunti);
			    cvxHull = G_alloc_matrix (lungHull, 3);

			    for (xi = 0; xi < lungHull; xi++) {
				cvxHull[xi][0] = P[xi][0];
				cvxHull[xi][1] = P[xi][1];
				cvxHull[xi][2] = P[xi][2];
			    }

			    /* Computes the interpoling plane based only on Object points*/
			    altPiano = pianOriz(punti_bordo, lungPunti, &minNS, &minEW, &maxNS, &maxEW, raster_matrix, colorBordo);

			    for (c1 = minNS; c1 <= maxNS; c1++) {
				for (c2 = minEW; c2 <= maxEW; c2++) {
				    if (checkHull(c1, c2, cvxHull, lungHull) == 1) {
					raster_matrix[c1][c2].obj = count_obj;

					if ((raster_matrix[c1][c2].clas == PRE_TERRAIN) && (raster_matrix[c1][c2].orig >=altPiano))
					    raster_matrix[c1][c2].clas = ripieno;
				    }
				}
			    }
			    G_free_matrix (cvxHull);
			    count_obj++;
			}
		    }
		}
		G_free_matrix (punti_bordo);
		free_Pvector (P, 0, MaxPoints);
	    }

	/* WRITING THE OUTPUT VECTOR CATEGORIES */
	    Vect_rewind (&In);
	    while (Vect_read_next_line (&In, points, Cats) > 0) {	 /* Read every line for buffering points */

		if ((Vect_point_in_box (points->x[0], points->y[0], points->z[0], &elaboration_box)) && \
				((points->x[0] != elaboration_reg.west) || (points->x[0] == original_reg.west)) && \
				((points->y[0] != elaboration_reg.north) || (points->y[0] == original_reg.north))) {

		    row = (int)(G_northing_to_row(points->y[0], &elaboration_reg));
		    col = (int)(G_easting_to_col(points->x[0], &elaboration_reg));

		    if (raster_matrix[row][col].clas == PRE_TERRAIN) {
			if (raster_matrix[row][col].dueImp == SINGLE_PULSE)
			    Vect_cat_set (Cats, F_CLASSIFICATION, TERRAIN_SINGLE);
			else 
			    Vect_cat_set (Cats, F_CLASSIFICATION, TERRAIN_DOUBLE);
		    }
		    else {
			if (raster_matrix[row][col].dueImp == SINGLE_PULSE) 
			    Vect_cat_set (Cats, F_CLASSIFICATION, OBJECT_SINGLE);
			else 
			    Vect_cat_set (Cats, F_CLASSIFICATION, OBJECT_DOUBLE);
		    }

		    Vect_cat_set (Cats, F_COUNTER_OBJ, raster_matrix[row][col].obj);
		    Vect_write_line (&Out, GV_POINT, points, Cats);
		}
		Vect_reset_cats (Cats);
		Vect_reset_line (points);
	    }
	    free_structmatrix (raster_matrix, 0, nrows-1, 0, ncols-1);
	} /*! END WHILE; last_column = TRUE*/
    } /*! END WHILE; last_row = TRUE*/

    Vect_close (&In);
    Vect_close (&First);
    Vect_close (&Out);

    db_close_database_shutdown_driver (driver);

    G_done_msg("");
    exit(EXIT_SUCCESS);
}
